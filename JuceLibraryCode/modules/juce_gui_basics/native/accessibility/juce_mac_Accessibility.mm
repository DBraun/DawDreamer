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

#if (! defined MAC_OS_X_VERSION_10_13) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_13
 using NSAccessibilityRole = NSString*;
 using NSAccessibilityNotificationName = NSString*;
#endif

#if (! defined MAC_OS_X_VERSION_10_9) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_9
 const NSAccessibilityNotificationName NSAccessibilityLayoutChangedNotificationJuce = @"AXLayoutChanged";
#else
 const NSAccessibilityNotificationName NSAccessibilityLayoutChangedNotificationJuce = NSAccessibilityLayoutChangedNotification;
#endif

#if JUCE_OBJC_HAS_AVAILABLE_FEATURE || (defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10)

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& handler)
        : accessibilityElement (AccessibilityElement::create (handler))
    {}

    NSAccessibilityElement<NSAccessibility>* getAccessibilityElement() const noexcept
    {
        return accessibilityElement.get();
    }

private:
    //==============================================================================
    class AccessibilityElement  : public AccessibleObjCClass<NSAccessibilityElement<NSAccessibility>>
    {
    public:
        static Holder create (AccessibilityHandler& handler)
        {
           #if JUCE_OBJC_HAS_AVAILABLE_FEATURE
            if (@available (macOS 10.10, *))
           #endif
            {
                static AccessibilityElement cls;
                Holder element ([cls.createInstance() init]);
                object_setInstanceVariable (element.get(), "handler", &handler);
                return element;
            }

            return {};
        }

    private:
        AccessibilityElement()
        {
            addMethod (@selector (accessibilityNotifiesWhenDestroyed),     getAccessibilityNotifiesWhenDestroyed, "c@:");
            addMethod (@selector (isAccessibilityElement),                 getIsAccessibilityElement,             "c@:");
            addMethod (@selector (isAccessibilityEnabled),                 getIsAccessibilityEnabled,             "c@:");
            addMethod (@selector (accessibilityWindow),                    getAccessibilityWindow,                "@@:");
            addMethod (@selector (accessibilityTopLevelUIElement),         getAccessibilityWindow,                "@@:");
            addMethod (@selector (accessibilityFocusedUIElement),          getAccessibilityFocusedUIElement,      "@@:");
            addMethod (@selector (accessibilityHitTest:),                  accessibilityHitTest,                  "@@:", @encode (NSPoint));
            addMethod (@selector (accessibilityParent),                    getAccessibilityParent,                "@@:");
            addMethod (@selector (accessibilityChildren),                  getAccessibilityChildren,              "@@:");
            addMethod (@selector (isAccessibilityFocused),                 getIsAccessibilityFocused,             "c@:");
            addMethod (@selector (setAccessibilityFocused:),               setAccessibilityFocused,               "v@:c");
            addMethod (@selector (isAccessibilityModal),                   getIsAccessibilityModal,               "c@:");
            addMethod (@selector (accessibilityFrame),                     getAccessibilityFrame,                 @encode (NSRect), "@:");
            addMethod (@selector (accessibilityRole),                      getAccessibilityRole,                  "@@:");
            addMethod (@selector (accessibilitySubrole),                   getAccessibilitySubrole,               "@@:");
            addMethod (@selector (accessibilityTitle),                     getAccessibilityTitle,                 "@@:");
            addMethod (@selector (accessibilityLabel),                     getAccessibilityLabel,                 "@@:");
            addMethod (@selector (accessibilityHelp),                      getAccessibilityHelp,                  "@@:");
            addMethod (@selector (accessibilityValue),                     getAccessibilityValue,                 "@@:");
            addMethod (@selector (setAccessibilityValue:),                 setAccessibilityValue,                 "v@:@");
            addMethod (@selector (accessibilitySelectedChildren),          getAccessibilitySelectedChildren,      "@@:");
            addMethod (@selector (setAccessibilitySelectedChildren:),      setAccessibilitySelectedChildren,      "v@:@");
            addMethod (@selector (accessibilityOrientation),               getAccessibilityOrientation,           "i@:@");

            addMethod (@selector (accessibilityInsertionPointLineNumber),  getAccessibilityInsertionPointLineNumber, "i@:");
            addMethod (@selector (accessibilityVisibleCharacterRange),     getAccessibilityVisibleCharacterRange,    @encode (NSRange), "@:");
            addMethod (@selector (accessibilityNumberOfCharacters),        getAccessibilityNumberOfCharacters,       "i@:");
            addMethod (@selector (accessibilitySelectedText),              getAccessibilitySelectedText,             "@@:");
            addMethod (@selector (accessibilitySelectedTextRange),         getAccessibilitySelectedTextRange,        @encode (NSRange), "@:");
            addMethod (@selector (accessibilityAttributedStringForRange:), getAccessibilityAttributedStringForRange, "@@:", @encode (NSRange));
            addMethod (@selector (accessibilityRangeForLine:),             getAccessibilityRangeForLine,             @encode (NSRange), "@:i");
            addMethod (@selector (accessibilityStringForRange:),           getAccessibilityStringForRange,           "@@:", @encode (NSRange));
            addMethod (@selector (accessibilityRangeForPosition:),         getAccessibilityRangeForPosition,         @encode (NSRange), "@:", @encode (NSPoint));
            addMethod (@selector (accessibilityRangeForIndex:),            getAccessibilityRangeForIndex,            @encode (NSRange), "@:i");
            addMethod (@selector (accessibilityFrameForRange:),            getAccessibilityFrameForRange,            @encode (NSRect), "@:", @encode (NSRange));
            addMethod (@selector (accessibilityLineForIndex:),             getAccessibilityLineForIndex,             "i@:i");
            addMethod (@selector (setAccessibilitySelectedTextRange:),     setAccessibilitySelectedTextRange,        "v@:", @encode (NSRange));

            addMethod (@selector (accessibilityRowCount),            getAccessibilityRowCount,         "i@:");
            addMethod (@selector (accessibilityRows),                getAccessibilityRows,             "@@:");
            addMethod (@selector (accessibilitySelectedRows),        getAccessibilitySelectedRows,     "@@:");
            addMethod (@selector (setAccessibilitySelectedRows:),    setAccessibilitySelectedRows,     "v@:@");
            addMethod (@selector (accessibilityColumnCount),         getAccessibilityColumnCount,      "i@:");
            addMethod (@selector (accessibilityColumns),             getAccessibilityColumns,          "@@:");
            addMethod (@selector (accessibilitySelectedColumns),     getAccessibilitySelectedColumns,  "@@:");
            addMethod (@selector (setAccessibilitySelectedColumns:), setAccessibilitySelectedColumns,  "v@:@");

            addMethod (@selector (accessibilityRowIndexRange),    getAccessibilityRowIndexRange,    @encode (NSRange), "@:");
            addMethod (@selector (accessibilityColumnIndexRange), getAccessibilityColumnIndexRange, @encode (NSRange), "@:");
            addMethod (@selector (accessibilityIndex),            getAccessibilityIndex,            "i@:");
            addMethod (@selector (accessibilityDisclosureLevel),  getAccessibilityDisclosureLevel,  "i@:");
            addMethod (@selector (isAccessibilityExpanded),       getIsAccessibilityExpanded,       "c@:");

            addMethod (@selector (accessibilityPerformIncrement), accessibilityPerformIncrement, "c@:");
            addMethod (@selector (accessibilityPerformDecrement), accessibilityPerformDecrement, "c@:");
            addMethod (@selector (accessibilityPerformDelete),    accessibilityPerformDelete,    "c@:");
            addMethod (@selector (accessibilityPerformPress),     accessibilityPerformPress,     "c@:");
            addMethod (@selector (accessibilityPerformShowMenu),  accessibilityPerformShowMenu,  "c@:");
            addMethod (@selector (accessibilityPerformRaise),     accessibilityPerformRaise,     "c@:");

            addMethod (@selector (isAccessibilitySelectorAllowed:), getIsAccessibilitySelectorAllowed, "c@:@");

           #if defined (MAC_OS_X_VERSION_10_13) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_13
            addMethod (@selector (accessibilityChildrenInNavigationOrder), getAccessibilityChildren, "@@:");
           #endif

            registerClass();
        }

        //==============================================================================
        static bool isSelectable (AccessibleState state) noexcept
        {
            return state.isSelectable() || state.isMultiSelectable();
        }

        static NSArray* getSelectedChildren (NSArray* children)
        {
            NSMutableArray* selected = [[NSMutableArray new] autorelease];

            for (id child in children)
            {
                if (auto* handler = getHandler (child))
                {
                    const auto currentState = handler->getCurrentState();

                    if (isSelectable (currentState) && currentState.isSelected())
                        [selected addObject: child];
                }
            }

            return selected;
        }

        static void setSelectedChildren (NSArray* children, NSArray* selected)
        {
            for (id child in children)
            {
                if (auto* handler = getHandler (child))
                {
                    const auto currentState = handler->getCurrentState();
                    const BOOL isSelected = [selected containsObject: child];

                    if (isSelectable (currentState))
                    {
                        if (currentState.isSelected() != isSelected)
                            handler->getActions().invoke (AccessibilityActionType::toggle);
                    }
                    else if (currentState.isFocusable())
                    {
                        [child setAccessibilityFocused: isSelected];
                    }
                }
            }
        }

        //==============================================================================
        static BOOL getAccessibilityNotifiesWhenDestroyed (id, SEL)  { return YES; }

        static BOOL getIsAccessibilityEnabled (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return handler->getComponent().isEnabled();

            return NO;
        }

        static id getAccessibilityWindow (id self, SEL)
        {
            return [[self accessibilityParent] accessibilityWindow];
        }

        static id getAccessibilityFocusedUIElement (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* modal = Component::getCurrentlyModalComponent())
                {
                    const auto& component = handler->getComponent();

                    if (! component.isParentOf (modal)
                         && component.isCurrentlyBlockedByAnotherModalComponent())
                    {
                        if (auto* modalHandler = modal->getAccessibilityHandler())
                        {
                            if (auto* focusChild = modalHandler->getChildFocus())
                                return (id) focusChild->getNativeImplementation();

                            return (id) modalHandler->getNativeImplementation();
                        }
                    }
                }

                if (auto* focusChild = handler->getChildFocus())
                    return (id) focusChild->getNativeImplementation();
            }

            return nil;
        }

        static id accessibilityHitTest (id self, SEL, NSPoint point)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* child = handler->getChildAt (roundToIntPoint (flippedScreenPoint (point))))
                    return (id) child->getNativeImplementation();

                return self;
            }

            return nil;
        }

        static id getAccessibilityParent (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* parentHandler = handler->getParent())
                    return NSAccessibilityUnignoredAncestor ((id) parentHandler->getNativeImplementation());

                return NSAccessibilityUnignoredAncestor ((id) handler->getComponent().getWindowHandle());
            }

            return nil;
        }

        static NSArray* getAccessibilityChildren (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                auto children = handler->getChildren();

                auto* accessibleChildren = [NSMutableArray arrayWithCapacity: (NSUInteger) children.size()];

                for (auto* childHandler : children)
                    [accessibleChildren addObject: (id) childHandler->getNativeImplementation()];

                return accessibleChildren;
            }

            return nil;
        }

        static NSArray* getAccessibilitySelectedChildren (id self, SEL)
        {
            return getSelectedChildren ([self accessibilityChildren]);
        }

        static void setAccessibilitySelectedChildren (id self, SEL, NSArray* selected)
        {
            setSelectedChildren ([self accessibilityChildren], selected);
        }

        static NSAccessibilityOrientation getAccessibilityOrientation (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return handler->getComponent().getBounds().toFloat().getAspectRatio() > 1.0f
                           ? NSAccessibilityOrientationHorizontal
                           : NSAccessibilityOrientationVertical;

            return NSAccessibilityOrientationUnknown;
        }

        static BOOL getIsAccessibilityFocused (id self, SEL)
        {
            return [[self accessibilityWindow] accessibilityFocusedUIElement] == self;
        }

        static void setAccessibilityFocused (id self, SEL, BOOL focused)
        {
            if (auto* handler = getHandler (self))
            {
                if (focused)
                {
                    const WeakReference<Component> safeComponent (&handler->getComponent());

                    performActionIfSupported (self, AccessibilityActionType::focus);

                    if (safeComponent != nullptr)
                        handler->grabFocus();
                }
                else
                {
                    handler->giveAwayFocus();
                }
            }
        }

        static NSRect getAccessibilityFrame (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return flippedScreenRect (makeNSRect (handler->getComponent().getScreenBounds()));

            return NSZeroRect;
        }

        static NSAccessibilityRole getAccessibilityRole (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                switch (handler->getRole())
                {
                    case AccessibilityRole::popupMenu:
                    case AccessibilityRole::tooltip:
                    case AccessibilityRole::splashScreen:
                    case AccessibilityRole::dialogWindow:
                    case AccessibilityRole::window:        return NSAccessibilityWindowRole;

                    case AccessibilityRole::tableHeader:
                    case AccessibilityRole::unspecified:
                    case AccessibilityRole::group:         return NSAccessibilityGroupRole;

                    case AccessibilityRole::label:
                    case AccessibilityRole::staticText:    return NSAccessibilityStaticTextRole;

                    case AccessibilityRole::tree:
                    case AccessibilityRole::list:          return NSAccessibilityOutlineRole;

                    case AccessibilityRole::listItem:
                    case AccessibilityRole::treeItem:      return NSAccessibilityRowRole;

                    case AccessibilityRole::button:        return NSAccessibilityButtonRole;
                    case AccessibilityRole::toggleButton:  return NSAccessibilityCheckBoxRole;
                    case AccessibilityRole::radioButton:   return NSAccessibilityRadioButtonRole;
                    case AccessibilityRole::comboBox:      return NSAccessibilityPopUpButtonRole;
                    case AccessibilityRole::image:         return NSAccessibilityImageRole;
                    case AccessibilityRole::slider:        return NSAccessibilitySliderRole;
                    case AccessibilityRole::editableText:  return NSAccessibilityTextAreaRole;
                    case AccessibilityRole::menuItem:      return NSAccessibilityMenuItemRole;
                    case AccessibilityRole::menuBar:       return NSAccessibilityMenuRole;
                    case AccessibilityRole::table:         return NSAccessibilityListRole;
                    case AccessibilityRole::column:        return NSAccessibilityColumnRole;
                    case AccessibilityRole::row:           return NSAccessibilityRowRole;
                    case AccessibilityRole::cell:          return NSAccessibilityCellRole;
                    case AccessibilityRole::hyperlink:     return NSAccessibilityLinkRole;
                    case AccessibilityRole::progressBar:   return NSAccessibilityProgressIndicatorRole;
                    case AccessibilityRole::scrollBar:     return NSAccessibilityScrollBarRole;

                    case AccessibilityRole::ignored:       break;
                }

                return NSAccessibilityUnknownRole;
            }

            return nil;
        }

        static NSAccessibilityRole getAccessibilitySubrole (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* textInterface = getTextInterface (self))
                    if (textInterface->isDisplayingProtectedText())
                        return NSAccessibilitySecureTextFieldSubrole;

                const auto role = handler->getRole();

                if (role == AccessibilityRole::window)                                     return NSAccessibilityStandardWindowSubrole;
                if (role == AccessibilityRole::dialogWindow)                               return NSAccessibilityDialogSubrole;
                if (role == AccessibilityRole::tooltip
                    || role == AccessibilityRole::splashScreen)                            return NSAccessibilityFloatingWindowSubrole;
                if (role == AccessibilityRole::toggleButton)                               return NSAccessibilityToggleSubrole;
                if (role == AccessibilityRole::treeItem
                    || role == AccessibilityRole::listItem)                                return NSAccessibilityOutlineRowSubrole;
                if (role == AccessibilityRole::row && getCellInterface (self) != nullptr)  return NSAccessibilityTableRowSubrole;

                const auto& component = handler->getComponent();

                if (auto* documentWindow = component.findParentComponentOfClass<DocumentWindow>())
                {
                    if (role == AccessibilityRole::button)
                    {
                        if (&component == documentWindow->getCloseButton())     return NSAccessibilityCloseButtonSubrole;
                        if (&component == documentWindow->getMinimiseButton())  return NSAccessibilityMinimizeButtonSubrole;
                        if (&component == documentWindow->getMaximiseButton())  return NSAccessibilityFullScreenButtonSubrole;
                    }
                }
            }

            return NSAccessibilityUnknownRole;
        }

        static NSString* getAccessibilityLabel (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return juceStringToNS (handler->getDescription());

            return nil;
        }

        //==============================================================================
        static NSInteger getAccessibilityInsertionPointLineNumber (id self, SEL)
        {
            if (auto* textInterface = getTextInterface (self))
                return [self accessibilityLineForIndex: textInterface->getTextInsertionOffset()];

            return 0;
        }

        static NSRange getAccessibilityVisibleCharacterRange (id self, SEL)
        {
            if (auto* textInterface = getTextInterface (self))
                return juceRangeToNS ({ 0, textInterface->getTotalNumCharacters() });

            return NSMakeRange (0, 0);
        }

        static NSInteger getAccessibilityNumberOfCharacters (id self, SEL)
        {
            if (auto* textInterface = getTextInterface (self))
                return textInterface->getTotalNumCharacters();

            return 0;
        }

        static NSString* getAccessibilitySelectedText (id self, SEL)
        {
            if (auto* textInterface = getTextInterface (self))
                return juceStringToNS (textInterface->getText (textInterface->getSelection()));

            return nil;
        }

        static NSRange getAccessibilitySelectedTextRange (id self, SEL)
        {
            if (auto* textInterface = getTextInterface (self))
            {
                const auto currentSelection = textInterface->getSelection();

                if (currentSelection.isEmpty())
                    return NSMakeRange ((NSUInteger) textInterface->getTextInsertionOffset(), 0);

                return juceRangeToNS (currentSelection);
            }

            return NSMakeRange (0, 0);
        }

        static NSAttributedString* getAccessibilityAttributedStringForRange (id self, SEL, NSRange range)
        {
            NSString* string = [self accessibilityStringForRange: range];

            if (string != nil)
                return [[[NSAttributedString alloc] initWithString: string] autorelease];

            return nil;
        }

        static NSRange getAccessibilityRangeForLine (id self, SEL, NSInteger line)
        {
            if (auto* textInterface = getTextInterface (self))
            {
                auto text = textInterface->getText ({ 0, textInterface->getTotalNumCharacters() });
                auto lines = StringArray::fromLines (text);

                if (line < lines.size())
                {
                    auto lineText = lines[(int) line];
                    auto start = text.indexOf (lineText);

                    if (start >= 0)
                        return NSMakeRange ((NSUInteger) start, (NSUInteger) lineText.length());
                }
            }

            return NSMakeRange (0, 0);
        }

        static NSString* getAccessibilityStringForRange (id self, SEL, NSRange range)
        {
            if (auto* textInterface = getTextInterface (self))
                return juceStringToNS (textInterface->getText (nsRangeToJuce (range)));

            return nil;
        }

        static NSRange getAccessibilityRangeForPosition (id self, SEL, NSPoint position)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* textInterface = handler->getTextInterface())
                {
                    auto screenPoint = roundToIntPoint (flippedScreenPoint (position));

                    if (handler->getComponent().getScreenBounds().contains (screenPoint))
                    {
                        auto offset = textInterface->getOffsetAtPoint (screenPoint);

                        if (offset >= 0)
                            return NSMakeRange ((NSUInteger) offset, 1);
                    }
                }
            }

            return NSMakeRange (0, 0);
        }

        static NSRange getAccessibilityRangeForIndex (id self, SEL, NSInteger index)
        {
            if (auto* textInterface = getTextInterface (self))
                if (isPositiveAndBelow (index, textInterface->getTotalNumCharacters()))
                    return NSMakeRange ((NSUInteger) index, 1);

            return NSMakeRange (0, 0);
        }

        static NSRect getAccessibilityFrameForRange (id self, SEL, NSRange range)
        {
            if (auto* textInterface = getTextInterface (self))
                return flippedScreenRect (makeNSRect (textInterface->getTextBounds (nsRangeToJuce (range)).getBounds()));

            return NSZeroRect;
        }

        static NSInteger getAccessibilityLineForIndex (id self, SEL, NSInteger index)
        {
            if (auto* textInterface = getTextInterface (self))
            {
                auto text = textInterface->getText ({ 0, (int) index });

                if (! text.isEmpty())
                    return StringArray::fromLines (text).size() - 1;
            }

            return 0;
        }

        static void setAccessibilitySelectedTextRange (id self, SEL, NSRange selectedRange)
        {
            if (auto* textInterface = getTextInterface (self))
                textInterface->setSelection (nsRangeToJuce (selectedRange));
        }

        //==============================================================================
        static NSArray* getAccessibilityRows (id self, SEL)
        {
            NSMutableArray* rows = [[NSMutableArray new] autorelease];

            if (auto* tableInterface = getTableInterface (self))
            {
                for (int row = 0; row < tableInterface->getNumRows(); ++row)
                {
                    if (auto* handler = tableInterface->getCellHandler (row, 0))
                    {
                        [rows addObject: (id) handler->getNativeImplementation()];
                    }
                    else
                    {
                        [rows addObject: [NSAccessibilityElement accessibilityElementWithRole: NSAccessibilityRowRole
                                                                                         frame: NSZeroRect
                                                                                         label: @"Offscreen Row"
                                                                                        parent: self]];
                    }
                }
            }

            return rows;
        }

        static NSArray* getAccessibilitySelectedRows (id self, SEL)
        {
            return getSelectedChildren ([self accessibilityRows]);
        }

        static void setAccessibilitySelectedRows (id self, SEL, NSArray* selected)
        {
            setSelectedChildren ([self accessibilityRows], selected);
        }

        static NSArray* getAccessibilityColumns (id self, SEL)
        {
            NSMutableArray* columns = [[NSMutableArray new] autorelease];

            if (auto* tableInterface = getTableInterface (self))
            {
                for (int column = 0; column < tableInterface->getNumColumns(); ++column)
                {
                    if (auto* handler = tableInterface->getCellHandler (0, column))
                    {
                        [columns addObject: (id) handler->getNativeImplementation()];
                    }
                    else
                    {
                        [columns addObject: [NSAccessibilityElement accessibilityElementWithRole: NSAccessibilityColumnRole
                                                                                            frame: NSZeroRect
                                                                                            label: @"Offscreen Column"
                                                                                           parent: self]];
                    }
                }
            }

            return columns;
        }

        static NSArray* getAccessibilitySelectedColumns (id self, SEL)
        {
            return getSelectedChildren ([self accessibilityColumns]);
        }

        static void setAccessibilitySelectedColumns (id self, SEL, NSArray* selected)
        {
            setSelectedChildren ([self accessibilityColumns], selected);
        }

        //==============================================================================
        static NSInteger getAccessibilityIndex (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* cellInterface = handler->getCellInterface())
                {
                    NSAccessibilityRole role = [self accessibilityRole];

                    if ([role isEqual: NSAccessibilityRowRole])
                        return cellInterface->getRowIndex();

                    if ([role isEqual: NSAccessibilityColumnRole])
                        return cellInterface->getColumnIndex();
                }
            }

            return 0;
        }

        static NSInteger getAccessibilityDisclosureLevel (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                if (auto* cellInterface = handler->getCellInterface())
                    return cellInterface->getDisclosureLevel();

            return 0;
        }

        static BOOL getIsAccessibilityExpanded (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return handler->getCurrentState().isExpanded();

            return NO;
        }

        //==============================================================================
        static BOOL accessibilityPerformShowMenu (id self, SEL)  { return performActionIfSupported (self, AccessibilityActionType::showMenu); }
        static BOOL accessibilityPerformRaise (id self, SEL)     { [self setAccessibilityFocused: YES]; return YES; }

        static BOOL accessibilityPerformDelete (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (hasEditableText (*handler))
                {
                    handler->getTextInterface()->setText ({});
                    return YES;
                }

                if (auto* valueInterface = handler->getValueInterface())
                {
                    if (! valueInterface->isReadOnly())
                    {
                        valueInterface->setValue ({});
                        return YES;
                    }
                }
            }

            return NO;
        }

        //==============================================================================
        static BOOL getIsAccessibilitySelectorAllowed (id self, SEL, SEL selector)
        {
            if (auto* handler = getHandler (self))
            {
                const auto role = handler->getRole();
                const auto currentState = handler->getCurrentState();

                for (auto textSelector : { @selector (accessibilityInsertionPointLineNumber),
                                           @selector (accessibilityVisibleCharacterRange),
                                           @selector (accessibilityNumberOfCharacters),
                                           @selector (accessibilitySelectedText),
                                           @selector (accessibilitySelectedTextRange),
                                           @selector (accessibilityAttributedStringForRange:),
                                           @selector (accessibilityRangeForLine:),
                                           @selector (accessibilityStringForRange:),
                                           @selector (accessibilityRangeForPosition:),
                                           @selector (accessibilityRangeForIndex:),
                                           @selector (accessibilityFrameForRange:),
                                           @selector (accessibilityLineForIndex:),
                                           @selector (setAccessibilitySelectedTextRange:) })
                {
                    if (selector == textSelector)
                        return handler->getTextInterface() != nullptr;
                }

                for (auto tableSelector : { @selector (accessibilityRowCount),
                                            @selector (accessibilityRows),
                                            @selector (accessibilitySelectedRows),
                                            @selector (accessibilityColumnCount),
                                            @selector (accessibilityColumns),
                                            @selector (accessibilitySelectedColumns) })
                {
                    if (selector == tableSelector)
                        return handler->getTableInterface() != nullptr;
                }

                for (auto cellSelector : { @selector (accessibilityRowIndexRange),
                                           @selector (accessibilityColumnIndexRange),
                                           @selector (accessibilityIndex),
                                           @selector (accessibilityDisclosureLevel) })
                {
                    if (selector == cellSelector)
                        return handler->getCellInterface() != nullptr;
                }

                for (auto valueSelector : { @selector (accessibilityValue),
                                            @selector (setAccessibilityValue:),
                                            @selector (accessibilityPerformDelete),
                                            @selector (accessibilityPerformIncrement),
                                            @selector (accessibilityPerformDecrement) })
                {
                    if (selector != valueSelector)
                        continue;

                    auto* valueInterface = handler->getValueInterface();

                    if (selector == @selector (accessibilityValue))
                        return valueInterface != nullptr
                            || hasEditableText (*handler)
                            || currentState.isCheckable();

                    auto hasEditableValue = [valueInterface] { return valueInterface != nullptr && ! valueInterface->isReadOnly(); };

                    if (selector == @selector (setAccessibilityValue:)
                     || selector == @selector (accessibilityPerformDelete))
                        return hasEditableValue() || hasEditableText (*handler);

                    auto isRanged = [valueInterface] { return valueInterface != nullptr && valueInterface->getRange().isValid(); };

                    if (selector == @selector (accessibilityPerformIncrement)
                     || selector == @selector (accessibilityPerformDecrement))
                        return hasEditableValue() && isRanged();

                    return NO;
                }

                for (auto actionSelector : { @selector (accessibilityPerformPress),
                                             @selector (accessibilityPerformShowMenu),
                                             @selector (accessibilityPerformRaise),
                                             @selector (setAccessibilityFocused:) })
                {
                    if (selector != actionSelector)
                        continue;

                    if (selector == @selector (accessibilityPerformPress))
                        return handler->getActions().contains (AccessibilityActionType::press);

                    if (selector == @selector (accessibilityPerformShowMenu))
                        return handler->getActions().contains (AccessibilityActionType::showMenu);

                    if (selector == @selector (accessibilityPerformRaise))
                        return [[self accessibilityRole] isEqual: NSAccessibilityWindowRole];

                    if (selector == @selector (setAccessibilityFocused:))
                        return currentState.isFocusable();
                }

                if (selector == @selector (accessibilitySelectedChildren))
                    return role == AccessibilityRole::popupMenu;

                if (selector == @selector (accessibilityOrientation))
                    return role == AccessibilityRole::scrollBar;

                if (selector == @selector (isAccessibilityExpanded))
                    return currentState.isExpandable();

                return sendSuperclassMessage<BOOL> (self, @selector (isAccessibilitySelectorAllowed:), selector);
            }

            return NO;
        }

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityElement)
    };

    //==============================================================================
    AccessibilityElement::Holder accessibilityElement;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeImpl)
};

//==============================================================================
AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const
{
    return (AccessibilityNativeHandle*) nativeImpl->getAccessibilityElement();
}

static bool areAnyAccessibilityClientsActive()
{
    const String voiceOverKeyString ("voiceOverOnOffKey");
    const String applicationIDString ("com.apple.universalaccess");

    CFUniquePtr<CFStringRef> cfKey (voiceOverKeyString.toCFString());
    CFUniquePtr<CFStringRef> cfID  (applicationIDString.toCFString());

    CFUniquePtr<CFPropertyListRef> value (CFPreferencesCopyAppValue (cfKey.get(), cfID.get()));

    if (value != nullptr)
        return CFBooleanGetValue ((CFBooleanRef) value.get());

    return false;
}

static void sendAccessibilityEvent (id accessibilityElement,
                                    NSAccessibilityNotificationName notification,
                                    NSDictionary* userInfo)
{
    jassert (notification != NSAccessibilityNotificationName{});

    NSAccessibilityPostNotificationWithUserInfo (accessibilityElement, notification, userInfo);
}

static void sendHandlerNotification (const AccessibilityHandler& handler,
                                     NSAccessibilityNotificationName notification)
{
    if (! areAnyAccessibilityClientsActive() || notification == NSAccessibilityNotificationName{})
        return;

    if (id accessibilityElement = (id) handler.getNativeImplementation())
    {
        sendAccessibilityEvent (accessibilityElement, notification,
                                (notification == NSAccessibilityLayoutChangedNotification
                                   ? @{ NSAccessibilityUIElementsKey: @[ accessibilityElement ] }
                                   : nil));
    }
}

void notifyAccessibilityEventInternal (const AccessibilityHandler& handler, InternalAccessibilityEvent eventType)
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case InternalAccessibilityEvent::elementCreated:         return NSAccessibilityCreatedNotification;
            case InternalAccessibilityEvent::elementDestroyed:       return NSAccessibilityUIElementDestroyedNotification;
            case InternalAccessibilityEvent::elementMovedOrResized:  return NSAccessibilityLayoutChangedNotificationJuce;
            case InternalAccessibilityEvent::focusChanged:           return NSAccessibilityFocusedUIElementChangedNotification;
            case InternalAccessibilityEvent::windowOpened:           return NSAccessibilityWindowCreatedNotification;
            case InternalAccessibilityEvent::windowClosed:           break;
        }

        return NSAccessibilityNotificationName{};
    }();

    sendHandlerNotification (handler, notification);
}

void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case AccessibilityEvent::textSelectionChanged:  return NSAccessibilitySelectedTextChangedNotification;
            case AccessibilityEvent::rowSelectionChanged:   return NSAccessibilitySelectedRowsChangedNotification;

            case AccessibilityEvent::textChanged:
            case AccessibilityEvent::valueChanged:          return NSAccessibilityValueChangedNotification;
            case AccessibilityEvent::titleChanged:          return NSAccessibilityTitleChangedNotification;
            case AccessibilityEvent::structureChanged:      return NSAccessibilityLayoutChangedNotificationJuce;
        }

        return NSAccessibilityNotificationName{};
    }();

    sendHandlerNotification (*this, notification);
}

void AccessibilityHandler::postAnnouncement (const String& announcementString, AnnouncementPriority priority)
{
    if (! areAnyAccessibilityClientsActive())
        return;

    #if JUCE_OBJC_HAS_AVAILABLE_FEATURE
     if (@available (macOS 10.10, *))
    #endif
     {
        auto nsPriority = [priority]
        {
            switch (priority)
            {
                case AnnouncementPriority::low:    return NSAccessibilityPriorityLow;
                case AnnouncementPriority::medium: return NSAccessibilityPriorityMedium;
                case AnnouncementPriority::high:   return NSAccessibilityPriorityHigh;
            }

            jassertfalse;
            return NSAccessibilityPriorityLow;
        }();

        sendAccessibilityEvent ((id) [NSApp mainWindow],
                                NSAccessibilityAnnouncementRequestedNotification,
                                @{ NSAccessibilityAnnouncementKey: juceStringToNS (announcementString),
                                   NSAccessibilityPriorityKey:     @(nsPriority) });
     }
}

#endif

} // namespace juce
