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

//==============================================================================
class UIAValueProvider  : public UIAProviderBase,
                          public ComBaseClassHelper<IValueProvider>
{
public:
    UIAValueProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT SetValue (LPCWSTR val) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();
        auto& valueInterface = *handler.getValueInterface();

        if (valueInterface.isReadOnly())
            return (HRESULT) UIA_E_NOTSUPPORTED;

        valueInterface.setValueAsString (String (val));

        VARIANT newValue;
        VariantHelpers::setString (valueInterface.getCurrentValueAsString(), &newValue);

        sendAccessibilityPropertyChangedEvent (handler, UIA_ValueValuePropertyId, newValue);

        return S_OK;
    }

    JUCE_COMRESULT get_Value (BSTR* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto currentValueString = getHandler().getValueInterface()->getCurrentValueAsString();

            *pRetVal = SysAllocString ((const OLECHAR*) currentValueString.toWideCharPointer());
            return S_OK;
        });
    }

    JUCE_COMRESULT get_IsReadOnly (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getHandler().getValueInterface()->isReadOnly();
            return S_OK;
        });
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAValueProvider)
};

} // namespace juce
