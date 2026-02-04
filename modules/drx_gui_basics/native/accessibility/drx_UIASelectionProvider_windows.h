/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

//==============================================================================
class UIASelectionItemProvider  : public UIAProviderBase,
                                  public ComBaseClassHelper<ISelectionItemProvider>
{
public:
    explicit UIASelectionItemProvider (AccessibilityNativeHandle* handle)
        : UIAProviderBase (handle),
          isRadioButton (getHandler().getRole() == AccessibilityRole::radioButton)
    {
    }

    //==============================================================================
    DRX_COMRESULT AddToSelection() override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (isRadioButton)
        {
            handler.getActions().invoke (AccessibilityActionType::press);
            sendAccessibilityAutomationEvent (handler, UIA_SelectionItem_ElementSelectedEventId);

            return S_OK;
        }

        handler.getActions().invoke (AccessibilityActionType::toggle);
        handler.getActions().invoke (AccessibilityActionType::press);

        return S_OK;
    }

    DRX_COMRESULT get_IsSelected (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            const auto state = getHandler().getCurrentState();
            *pRetVal = isRadioButton ? state.isChecked() : state.isSelected();
            return S_OK;
        });
    }

    DRX_COMRESULT get_SelectionContainer (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            if (! isRadioButton)
                if (auto* parent = getHandler().getParent())
                    parent->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    DRX_COMRESULT RemoveFromSelection() override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        if (! isRadioButton)
        {
            const auto& handler = getHandler();

            if (handler.getCurrentState().isSelected())
                getHandler().getActions().invoke (AccessibilityActionType::toggle);
        }

        return S_OK;
    }

    DRX_COMRESULT Select() override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        AddToSelection();

        if (isElementValid() && ! isRadioButton)
        {
            const auto& handler = getHandler();

            if (auto* parent = handler.getParent())
                for (auto* child : parent->getChildren())
                    if (child != &handler && child->getCurrentState().isSelected())
                        child->getActions().invoke (AccessibilityActionType::toggle);
        }

        return S_OK;
    }

private:
    const b8 isRadioButton;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIASelectionItemProvider)
};

//==============================================================================
class UIASelectionProvider  : public UIAProviderBase,
                              public ComBaseClassHelper<ISelectionProvider2>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    DRX_COMRESULT QueryInterface (REFIID iid, uk* result) override
    {
        if (iid == __uuidof (IUnknown) || iid == __uuidof (ISelectionProvider))
            return castToType<ISelectionProvider> (result);

        if (iid == __uuidof (ISelectionProvider2))
            return castToType<ISelectionProvider2> (result);

        *result = nullptr;
        return E_NOINTERFACE;
    }

    //==============================================================================
    DRX_COMRESULT get_CanSelectMultiple (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = isMultiSelectable();
            return S_OK;
        });
    }

    DRX_COMRESULT get_IsSelectionRequired (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getSelectedChildren().size() > 0 && ! isMultiSelectable();
            return S_OK;
        });
    }

    DRX_COMRESULT GetSelection (SAFEARRAY** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            return addHandlersToArray (getSelectedChildren(), pRetVal);
        });
    }

    //==============================================================================
    DRX_COMRESULT get_FirstSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto selectedChildren = getSelectedChildren();

            if (! selectedChildren.empty())
                selectedChildren.front()->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    DRX_COMRESULT get_LastSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto selectedChildren = getSelectedChildren();

            if (! selectedChildren.empty())
                selectedChildren.back()->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    DRX_COMRESULT get_CurrentSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            get_FirstSelectedItem (pRetVal);
            return S_OK;
        });
    }

    DRX_COMRESULT get_ItemCount (i32* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = (i32) getSelectedChildren().size();
            return S_OK;
        });
    }

private:
    b8 isMultiSelectable() const noexcept
    {
         return getHandler().getCurrentState().isMultiSelectable();
    }

    std::vector<const AccessibilityHandler*> getSelectedChildren() const
    {
        std::vector<const AccessibilityHandler*> selectedHandlers;

        for (auto* child : getHandler().getComponent().getChildren())
            if (auto* handler = child->getAccessibilityHandler())
                if (handler->getCurrentState().isSelected())
                    selectedHandlers.push_back (handler);

        return selectedHandlers;
    }

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIASelectionProvider)
};

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace drx
