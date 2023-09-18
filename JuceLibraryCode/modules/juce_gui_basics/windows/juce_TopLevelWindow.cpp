/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
TopLevelWindow::TopLevelWindow (const String& name, const bool shouldAddToDesktop)
    : Component (name)
{
    setTitle (name);

    setOpaque (true);

    if (shouldAddToDesktop)
        Component::addToDesktop (TopLevelWindow::getDesktopWindowStyleFlags());
    else
        setDropShadowEnabled (true);

    setWantsKeyboardFocus (true);
    setBroughtToFrontOnMouseClick (true);
    isCurrentlyActive = detail::TopLevelWindowManager::getInstance()->addWindow (this);
}

TopLevelWindow::~TopLevelWindow()
{
    shadower = nullptr;
    detail::TopLevelWindowManager::getInstance()->removeWindow (this);
}

//==============================================================================
void TopLevelWindow::focusOfChildComponentChanged (FocusChangeType)
{
    auto* wm = detail::TopLevelWindowManager::getInstance();

    if (hasKeyboardFocus (true))
        wm->checkFocus();
    else
        wm->checkFocusAsync();
}

void TopLevelWindow::setWindowActive (const bool isNowActive)
{
    if (isCurrentlyActive != isNowActive)
    {
        isCurrentlyActive = isNowActive;
        activeWindowStatusChanged();
    }
}

void TopLevelWindow::activeWindowStatusChanged()
{
}

bool TopLevelWindow::isUsingNativeTitleBar() const noexcept
{
    return useNativeTitleBar && (isOnDesktop() || ! isShowing());
}

void TopLevelWindow::visibilityChanged()
{
    if (isShowing())
        if (auto* p = getPeer())
            if ((p->getStyleFlags() & (ComponentPeer::windowIsTemporary
                                        | ComponentPeer::windowIgnoresKeyPresses)) == 0)
                toFront (true);
}

void TopLevelWindow::parentHierarchyChanged()
{
    setDropShadowEnabled (useDropShadow);
}

int TopLevelWindow::getDesktopWindowStyleFlags() const
{
    int styleFlags = ComponentPeer::windowAppearsOnTaskbar;

    if (useDropShadow)       styleFlags |= ComponentPeer::windowHasDropShadow;
    if (useNativeTitleBar)   styleFlags |= ComponentPeer::windowHasTitleBar;

    return styleFlags;
}

void TopLevelWindow::setDropShadowEnabled (const bool useShadow)
{
    useDropShadow = useShadow;

    if (isOnDesktop())
    {
        shadower = nullptr;
        Component::addToDesktop (getDesktopWindowStyleFlags());
    }
    else
    {
        if (useShadow && isOpaque())
        {
            if (shadower == nullptr)
            {
                shadower = getLookAndFeel().createDropShadowerForComponent (*this);

                if (shadower != nullptr)
                    shadower->setOwner (this);
            }
        }
        else
        {
            shadower = nullptr;
        }
    }
}

void TopLevelWindow::setUsingNativeTitleBar (const bool shouldUseNativeTitleBar)
{
    if (useNativeTitleBar != shouldUseNativeTitleBar)
    {
        detail::FocusRestorer focusRestorer;
        useNativeTitleBar = shouldUseNativeTitleBar;
        recreateDesktopWindow();
        sendLookAndFeelChange();
    }
}

void TopLevelWindow::recreateDesktopWindow()
{
    if (isOnDesktop())
    {
        Component::addToDesktop (getDesktopWindowStyleFlags());
        toFront (true);
    }
}

void TopLevelWindow::addToDesktop()
{
    shadower = nullptr;
    Component::addToDesktop (getDesktopWindowStyleFlags());
    setDropShadowEnabled (isDropShadowEnabled()); // force an update to clear away any fake shadows if necessary.
}

void TopLevelWindow::addToDesktop (int windowStyleFlags, void* nativeWindowToAttachTo)
{
    /* It's not recommended to change the desktop window flags directly for a TopLevelWindow,
       because this class needs to make sure its layout corresponds with settings like whether
       it's got a native title bar or not.

       If you need custom flags for your window, you can override the getDesktopWindowStyleFlags()
       method. If you do this, it's best to call the base class's getDesktopWindowStyleFlags()
       method, then add or remove whatever flags are necessary from this value before returning it.
    */
    jassert ((windowStyleFlags & ~ComponentPeer::windowIsSemiTransparent)
               == (getDesktopWindowStyleFlags() & ~ComponentPeer::windowIsSemiTransparent));

    Component::addToDesktop (windowStyleFlags, nativeWindowToAttachTo);

    if (windowStyleFlags != getDesktopWindowStyleFlags())
        sendLookAndFeelChange();
}

std::unique_ptr<AccessibilityHandler> TopLevelWindow::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::window);
}

//==============================================================================
void TopLevelWindow::centreAroundComponent (Component* c, const int width, const int height)
{
    if (c == nullptr)
        c = TopLevelWindow::getActiveTopLevelWindow();

    if (c == nullptr || c->getBounds().isEmpty())
    {
        centreWithSize (width, height);
    }
    else
    {
        const auto scale = getDesktopScaleFactor() / Desktop::getInstance().getGlobalScaleFactor();

        const auto [targetCentre, parentArea] = [&]
        {
            const auto globalTargetCentre = c->localPointToGlobal (c->getLocalBounds().getCentre()) / scale;

            if (auto* parent = getParentComponent())
                return std::make_pair (parent->getLocalPoint (nullptr, globalTargetCentre), parent->getLocalBounds());

            return std::make_pair (globalTargetCentre, c->getParentMonitorArea() / scale);
        }();

        setBounds (Rectangle<int> (targetCentre.x - width / 2,
                                   targetCentre.y - height / 2,
                                   width, height)
                     .constrainedWithin (parentArea.reduced (12, 12)));
    }
}

//==============================================================================
int TopLevelWindow::getNumTopLevelWindows() noexcept
{
    return detail::TopLevelWindowManager::getInstance()->windows.size();
}

TopLevelWindow* TopLevelWindow::getTopLevelWindow (const int index) noexcept
{
    return detail::TopLevelWindowManager::getInstance()->windows [index];
}

TopLevelWindow* TopLevelWindow::getActiveTopLevelWindow() noexcept
{
    TopLevelWindow* best = nullptr;
    int bestNumTWLParents = -1;

    for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
    {
        auto* tlw = TopLevelWindow::getTopLevelWindow (i);

        if (tlw->isActiveWindow())
        {
            int numTWLParents = 0;

            for (auto* c = tlw->getParentComponent(); c != nullptr; c = c->getParentComponent())
                if (dynamic_cast<const TopLevelWindow*> (c) != nullptr)
                    ++numTWLParents;

            if (bestNumTWLParents < numTWLParents)
            {
                best = tlw;
                bestNumTWLParents = numTWLParents;
            }
        }
    }

    return best;
}

} // namespace juce
