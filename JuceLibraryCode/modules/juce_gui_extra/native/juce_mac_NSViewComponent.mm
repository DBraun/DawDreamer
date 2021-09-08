/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
static const auto nsViewFrameChangedSelector = @selector (frameChanged:);
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

struct NSViewCallbackInterface
{
    virtual ~NSViewCallbackInterface() = default;
    virtual void frameChanged() = 0;
};

//==============================================================================
struct NSViewFrameChangeCallbackClass   : public ObjCClass<NSObject>
{
    NSViewFrameChangeCallbackClass()
        : ObjCClass ("JUCE_NSViewCallback_")
    {
        addIvar<NSViewCallbackInterface*> ("target");

        addMethod (nsViewFrameChangedSelector, frameChanged, "v@:@");

        registerClass();
    }

    static void setTarget (id self, NSViewCallbackInterface* c)
    {
        object_setInstanceVariable (self, "target", c);
    }

private:
    static void frameChanged (id self, SEL, NSNotification*)
    {
        if (auto* target = getIvar<NSViewCallbackInterface*> (self, "target"))
            target->frameChanged();
    }

    JUCE_DECLARE_NON_COPYABLE (NSViewFrameChangeCallbackClass)
};

//==============================================================================
class NSViewFrameWatcher : private NSViewCallbackInterface
{
public:
    NSViewFrameWatcher (NSView* viewToWatch, std::function<void()> viewResizedIn)
        : viewResized (std::move (viewResizedIn)), callback (makeCallbackForView (viewToWatch))
    {
    }

    ~NSViewFrameWatcher() override
    {
        [[NSNotificationCenter defaultCenter] removeObserver: callback];
        [callback release];
        callback = nil;
    }

    JUCE_DECLARE_NON_COPYABLE (NSViewFrameWatcher)
    JUCE_DECLARE_NON_MOVEABLE (NSViewFrameWatcher)

private:
    id makeCallbackForView (NSView* view)
    {
        static NSViewFrameChangeCallbackClass cls;
        auto* result = [cls.createInstance() init];
        NSViewFrameChangeCallbackClass::setTarget (result, this);

        [[NSNotificationCenter defaultCenter]  addObserver: result
                                                  selector: nsViewFrameChangedSelector
                                                      name: NSViewFrameDidChangeNotification
                                                    object: view];

        return result;
    }

    void frameChanged() override { viewResized(); }

    std::function<void()> viewResized;
    id callback;
};

//==============================================================================
class NSViewAttachment  : public ReferenceCountedObject,
                          public ComponentMovementWatcher
{
public:
    NSViewAttachment (NSView* v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v), owner (comp),
          currentPeer (nullptr)
    {
        [view retain];
        [view setPostsFrameChangedNotifications: YES];
        updateAlpha();

        if (owner.isShowing())
            componentPeerChanged();
    }

    ~NSViewAttachment() override
    {
        removeFromParent();
        [view release];
    }

    void componentMovedOrResized (Component& comp, bool wasMoved, bool wasResized) override
    {
        ComponentMovementWatcher::componentMovedOrResized (comp, wasMoved, wasResized);

        // The ComponentMovementWatcher version of this method avoids calling
        // us when the top-level comp is resized, but if we're listening to the
        // top-level comp we still want the NSView to track its size.
        if (comp.isOnDesktop() && wasResized)
            componentMovedOrResized (wasMoved, wasResized);
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
        {
            const auto newArea = peer->getAreaCoveredBy (owner);

            if (convertToRectInt ([view frame]) != newArea)
                [view setFrame: makeNSRect (newArea)];
        }
    }

    void componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (currentPeer != peer)
        {
            currentPeer = peer;

            if (peer != nullptr)
            {
                auto peerView = (NSView*) peer->getNativeHandle();
                [peerView addSubview: view];
                componentMovedOrResized (false, false);
            }
            else
            {
                removeFromParent();
            }
        }

        [view setHidden: ! owner.isShowing()];
    }

    void componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    void updateAlpha()
    {
        [view setAlphaValue: (CGFloat) owner.getAlpha()];
    }

    NSView* const view;

    using Ptr = ReferenceCountedObjectPtr<NSViewAttachment>;

private:
    Component& owner;
    ComponentPeer* currentPeer;
    NSViewFrameWatcher frameWatcher { view, [this] { owner.childBoundsChanged (nullptr); } };

    void removeFromParent()
    {
        if ([view superview] != nil)
            [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                        // override the call and use it as a sign that they're being deleted, which breaks everything..
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewAttachment)
};

//==============================================================================
NSViewComponent::NSViewComponent() = default;
NSViewComponent::~NSViewComponent() = default;

void NSViewComponent::setView (void* view)
{
    if (view != getView())
    {
        auto old = attachment;

        attachment = nullptr;

        if (view != nullptr)
            attachment = attachViewToComponent (*this, view);

        old = nullptr;
    }
}

void* NSViewComponent::getView() const
{
    return attachment != nullptr ? static_cast<NSViewAttachment*> (attachment.get())->view
                                 : nullptr;
}

void NSViewComponent::resizeToFitView()
{
    if (attachment != nullptr)
    {
        auto* view = static_cast<NSViewAttachment*> (attachment.get())->view;
        auto r = [view frame];
        setBounds (Rectangle<int> ((int) r.size.width, (int) r.size.height));

        if (auto* peer = getTopLevelComponent()->getPeer())
        {
            const auto position = peer->getAreaCoveredBy (*this).getPosition();
            [view setFrameOrigin: convertToCGPoint (position)];
        }
    }
}

void NSViewComponent::paint (Graphics&) {}

void NSViewComponent::alphaChanged()
{
    if (attachment != nullptr)
        (static_cast<NSViewAttachment*> (attachment.get()))->updateAlpha();
}

ReferenceCountedObject* NSViewComponent::attachViewToComponent (Component& comp, void* view)
{
    return new NSViewAttachment ((NSView*) view, comp);
}

} // namespace juce
