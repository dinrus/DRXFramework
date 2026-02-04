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

//==============================================================================
class UIATextProvider  : public UIAProviderBase,
                         public ComBaseClassHelper<ITextProvider2>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    DRX_COMRESULT QueryInterface (REFIID iid, uk* result) override
    {
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        if (iid == __uuidof (IUnknown) || iid == __uuidof (ITextProvider))
            return castToType<ITextProvider> (result);

        if (iid == __uuidof (ITextProvider2))
            return castToType<ITextProvider2> (result);

        *result = nullptr;
        return E_NOINTERFACE;

        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }

    //==============================================================================
    DRX_COMRESULT get_DocumentRange (ITextRangeProvider** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *pRetVal = new UIATextRangeProvider (*this, { 0, textInterface.getTotalNumCharacters() });
            return S_OK;
        });
    }

    DRX_COMRESULT get_SupportedTextSelection (SupportedTextSelection* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = SupportedTextSelection_Single;
            return S_OK;
        });
    }

    DRX_COMRESULT GetSelection (SAFEARRAY** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 1);

            if (pRetVal != nullptr)
            {
                auto selection = textInterface.getSelection();
                auto hasSelection = ! selection.isEmpty();
                auto cursorPos = textInterface.getTextInsertionOffset();

                auto* rangeProvider = new UIATextRangeProvider (*this,
                                                                { hasSelection ? selection.getStart() : cursorPos,
                                                                  hasSelection ? selection.getEnd()   : cursorPos });

                LONG pos = 0;
                auto hr = SafeArrayPutElement (*pRetVal, &pos, static_cast<IUnknown*> (rangeProvider));

                if (FAILED (hr))
                    return E_FAIL;

                rangeProvider->Release();
            }

            return S_OK;
        });
    }

    DRX_COMRESULT GetVisibleRanges (SAFEARRAY** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 1);

            if (pRetVal != nullptr)
            {
                auto* rangeProvider = new UIATextRangeProvider (*this, { 0, textInterface.getTotalNumCharacters() });

                LONG pos = 0;
                auto hr = SafeArrayPutElement (*pRetVal, &pos, static_cast<IUnknown*> (rangeProvider));

                if (FAILED (hr))
                    return E_FAIL;

                rangeProvider->Release();
            }

            return S_OK;
        });
    }

    DRX_COMRESULT RangeFromChild (IRawElementProviderSimple*, ITextRangeProvider** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, []
        {
            return S_OK;
        });
    }

    DRX_COMRESULT RangeFromPoint (UiaPoint point, ITextRangeProvider** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            auto offset = textInterface.getOffsetAtPoint ({ roundToInt (point.x), roundToInt (point.y) });

            if (offset > 0)
                *pRetVal = new UIATextRangeProvider (*this, { offset, offset });

            return S_OK;
        });
    }

    //==============================================================================
    DRX_COMRESULT GetCaretRange (BOOL* isActive, ITextRangeProvider** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *isActive = getHandler().hasFocus (false);

            auto cursorPos = textInterface.getTextInsertionOffset();
            *pRetVal = new UIATextRangeProvider (*this, { cursorPos, cursorPos });

            return S_OK;
        });
    }

    DRX_COMRESULT RangeFromAnnotation (IRawElementProviderSimple*, ITextRangeProvider** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, []
        {
            return S_OK;
        });
    }

private:
    //==============================================================================
    template <typename Value, typename Callback>
    DRX_COMRESULT withTextInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* textInterface = getHandler().getTextInterface())
                return callback (*textInterface);

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    class UIATextRangeProvider  : public UIAProviderBase,
                                  public ComBaseClassHelper<ITextRangeProvider>
    {
    public:
        UIATextRangeProvider (UIATextProvider& textProvider, Range<i32> range)
            : UIAProviderBase (textProvider.getHandler().getNativeImplementation()),
              owner (addComSmartPtrOwner (&textProvider)),
              selectionRange (range)
        {
        }

        //==============================================================================
        Range<i32> getSelectionRange() const noexcept  { return selectionRange; }

        //==============================================================================
        DRX_COMRESULT AddToSelection() override
        {
            return Select();
        }

        DRX_COMRESULT Clone (ITextRangeProvider** pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, [&]
            {
                *pRetVal = new UIATextRangeProvider (*owner, selectionRange);
                return S_OK;
            });
        }

        DRX_COMRESULT Compare (ITextRangeProvider* range, BOOL* pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, [&]
            {
                *pRetVal = (selectionRange == static_cast<UIATextRangeProvider*> (range)->getSelectionRange());
                return S_OK;
            });
        }

        DRX_COMRESULT CompareEndpoints (TextPatternRangeEndpoint endpoint,
                                         ITextRangeProvider* targetRange,
                                         TextPatternRangeEndpoint targetEndpoint,
                                         i32* pRetVal) override
        {
            if (targetRange == nullptr)
                return E_INVALIDARG;

            return withCheckedComArgs (pRetVal, *this, [&]
            {
                auto offset = (endpoint == TextPatternRangeEndpoint_Start ? selectionRange.getStart()
                                                                          : selectionRange.getEnd());

                auto otherRange = static_cast<UIATextRangeProvider*> (targetRange)->getSelectionRange();
                auto otherOffset = (targetEndpoint == TextPatternRangeEndpoint_Start ? otherRange.getStart()
                                                                                     : otherRange.getEnd());

                *pRetVal = offset - otherOffset;
                return S_OK;
            });
        }

        DRX_COMRESULT ExpandToEnclosingUnit (TextUnit unit) override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (auto* textInterface = owner->getHandler().getTextInterface())
            {
                using ATH = AccessibilityTextHelpers;

                const auto boundaryType = getBoundaryType (unit);
                const auto start = ATH::findTextBoundary (*textInterface,
                                                          selectionRange.getStart(),
                                                          boundaryType,
                                                          ATH::Direction::backwards,
                                                          ATH::IncludeThisBoundary::yes,
                                                          ATH::IncludeWhitespaceAfterWords::no);

                const auto end = ATH::findTextBoundary (*textInterface,
                                                        start,
                                                        boundaryType,
                                                        ATH::Direction::forwards,
                                                        ATH::IncludeThisBoundary::no,
                                                        ATH::IncludeWhitespaceAfterWords::yes);

                selectionRange = Range<i32> (start, end);

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        DRX_COMRESULT FindAttribute (TEXTATTRIBUTEID, VARIANT, BOOL, ITextRangeProvider** pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, []
            {
                return S_OK;
            });
        }

        DRX_COMRESULT FindText (BSTR text, BOOL backward, BOOL ignoreCase,
                                 ITextRangeProvider** pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                auto selectionText = textInterface.getText (selectionRange);
                Txt textToSearchFor (text);

                auto offset = (backward ? (ignoreCase ? selectionText.lastIndexOfIgnoreCase (textToSearchFor) : selectionText.lastIndexOf (textToSearchFor))
                                        : (ignoreCase ? selectionText.indexOfIgnoreCase (textToSearchFor)     : selectionText.indexOf (textToSearchFor)));

                if (offset != -1)
                    *pRetVal = new UIATextRangeProvider (*owner, { offset, offset + textToSearchFor.length() });

                return S_OK;
            });
        }

        DRX_COMRESULT GetAttributeValue (TEXTATTRIBUTEID attributeId, VARIANT* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                VariantHelpers::clear (pRetVal);

                switch (attributeId)
                {
                    case UIA_IsReadOnlyAttributeId:
                    {
                        VariantHelpers::setBool (textInterface.isReadOnly(), pRetVal);
                        break;
                    }

                    case UIA_CaretPositionAttributeId:
                    {
                        auto cursorPos = textInterface.getTextInsertionOffset();

                        auto caretPos = [&]
                        {
                            if (cursorPos == 0)
                                return CaretPosition_BeginningOfLine;

                            if (cursorPos == textInterface.getTotalNumCharacters())
                                return CaretPosition_EndOfLine;

                            return CaretPosition_Unknown;
                        }();

                        VariantHelpers::setInt (caretPos, pRetVal);
                        break;
                    }
                }

                return S_OK;
            });
        }

        DRX_COMRESULT GetBoundingRectangles (SAFEARRAY** pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                auto rectangleList = textInterface.getTextBounds (selectionRange);
                auto numRectangles = rectangleList.getNumRectangles();

                *pRetVal = SafeArrayCreateVector (VT_R8, 0, 4 * (ULONG) numRectangles);

                if (*pRetVal == nullptr)
                    return E_FAIL;

                if (numRectangles > 0)
                {
                    f64* doubleArr = nullptr;

                    if (FAILED (SafeArrayAccessData (*pRetVal, reinterpret_cast<uk*> (&doubleArr))))
                    {
                        SafeArrayDestroy (*pRetVal);
                        return E_FAIL;
                    }

                    for (i32 i = 0; i < numRectangles; ++i)
                    {
                        auto r = Desktop::getInstance().getDisplays().logicalToPhysical (rectangleList.getRectangle (i));

                        doubleArr[i * 4]     = r.getX();
                        doubleArr[i * 4 + 1] = r.getY();
                        doubleArr[i * 4 + 2] = r.getWidth();
                        doubleArr[i * 4 + 3] = r.getHeight();
                    }

                    if (FAILED (SafeArrayUnaccessData (*pRetVal)))
                    {
                        SafeArrayDestroy (*pRetVal);
                        return E_FAIL;
                    }
                }

                return S_OK;
            });
        }

        DRX_COMRESULT GetChildren (SAFEARRAY** pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, [&]
            {
                *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 0);
                return S_OK;
            });
        }

        DRX_COMRESULT GetEnclosingElement (IRawElementProviderSimple** pRetVal) override
        {
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

            return withCheckedComArgs (pRetVal, *this, [&]
            {
                getHandler().getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
                return S_OK;
            });

            DRX_END_IGNORE_WARNINGS_GCC_LIKE
        }

        DRX_COMRESULT GetText (i32 maxLength, BSTR* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                auto text = textInterface.getText (selectionRange);

                if (maxLength >= 0 && text.length() > maxLength)
                    text = text.substring (0, maxLength);

                *pRetVal = SysAllocString ((const OLECHAR*) text.toWideCharPointer());
                return S_OK;
            });
        }

        DRX_COMRESULT Move (TextUnit unit, i32 count, i32* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                using ATH = AccessibilityTextHelpers;

                const auto boundaryType = getBoundaryType (unit);
                const auto previousUnitBoundary = ATH::findTextBoundary (textInterface,
                                                                         selectionRange.getStart(),
                                                                         boundaryType,
                                                                         ATH::Direction::backwards,
                                                                         ATH::IncludeThisBoundary::yes,
                                                                         ATH::IncludeWhitespaceAfterWords::no);

                auto numMoved = 0;
                auto movedEndpoint = previousUnitBoundary;

                for (; numMoved < std::abs (count); ++numMoved)
                {
                    const auto nextEndpoint = ATH::findTextBoundary (textInterface,
                                                                     movedEndpoint,
                                                                     boundaryType,
                                                                     count > 0 ? ATH::Direction::forwards : ATH::Direction::backwards,
                                                                     ATH::IncludeThisBoundary::no,
                                                                     count > 0 ? ATH::IncludeWhitespaceAfterWords::yes : ATH::IncludeWhitespaceAfterWords::no);

                    if (nextEndpoint == movedEndpoint)
                        break;

                    movedEndpoint = nextEndpoint;
                }

                *pRetVal = numMoved;

                ExpandToEnclosingUnit (unit);
                return S_OK;
            });
        }

        DRX_COMRESULT MoveEndpointByRange (TextPatternRangeEndpoint endpoint,
                                            ITextRangeProvider* targetRange,
                                            TextPatternRangeEndpoint targetEndpoint) override
        {
            if (targetRange == nullptr)
                return E_INVALIDARG;

            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (owner->getHandler().getTextInterface() != nullptr)
            {
                auto otherRange = static_cast<UIATextRangeProvider*> (targetRange)->getSelectionRange();
                auto targetPoint = (targetEndpoint == TextPatternRangeEndpoint_Start ? otherRange.getStart()
                                                                                     : otherRange.getEnd());

                setEndpointChecked (endpoint, targetPoint);
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        DRX_COMRESULT MoveEndpointByUnit (TextPatternRangeEndpoint endpoint,
                                           TextUnit unit,
                                           i32 count,
                                           i32* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                if (count == 0 || textInterface.getTotalNumCharacters() == 0)
                    return S_OK;

                const auto endpointToMove = (endpoint == TextPatternRangeEndpoint_Start ? selectionRange.getStart()
                                                                                        : selectionRange.getEnd());

                using ATH = AccessibilityTextHelpers;

                const auto direction = (count > 0 ? ATH::Direction::forwards
                                                  : ATH::Direction::backwards);

                const auto boundaryType = getBoundaryType (unit);
                auto movedEndpoint = endpointToMove;

                i32 numMoved = 0;
                for (; numMoved < std::abs (count); ++numMoved)
                {
                    auto nextEndpoint = ATH::findTextBoundary (textInterface,
                                                               movedEndpoint,
                                                               boundaryType,
                                                               direction,
                                                               ATH::IncludeThisBoundary::no,
                                                               direction == ATH::Direction::forwards ? ATH::IncludeWhitespaceAfterWords::yes
                                                                                                     : ATH::IncludeWhitespaceAfterWords::no);

                    if (nextEndpoint == movedEndpoint)
                        break;

                    movedEndpoint = nextEndpoint;
                }

                *pRetVal = numMoved;

                setEndpointChecked (endpoint, movedEndpoint);

                return S_OK;
            });
        }

        DRX_COMRESULT RemoveFromSelection() override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (auto* textInterface = owner->getHandler().getTextInterface())
            {
                textInterface->setSelection ({});
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        DRX_COMRESULT ScrollIntoView (BOOL) override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        DRX_COMRESULT Select() override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (auto* textInterface = owner->getHandler().getTextInterface())
            {
                textInterface->setSelection ({});
                textInterface->setSelection (selectionRange);

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

    private:
        static AccessibilityTextHelpers::BoundaryType getBoundaryType (TextUnit unit)
        {
            switch (unit)
            {
                case TextUnit_Character:
                    return AccessibilityTextHelpers::BoundaryType::character;

                case TextUnit_Format:
                case TextUnit_Word:
                    return AccessibilityTextHelpers::BoundaryType::word;

                case TextUnit_Line:
                    return AccessibilityTextHelpers::BoundaryType::line;

                case TextUnit_Paragraph:
                case TextUnit_Page:
                case TextUnit_Document:
                    return AccessibilityTextHelpers::BoundaryType::document;
            };

            jassertfalse;
            return AccessibilityTextHelpers::BoundaryType::character;
        }

        z0 setEndpointChecked (TextPatternRangeEndpoint endpoint, i32 newEndpoint)
        {
            if (endpoint == TextPatternRangeEndpoint_Start)
            {
                if (selectionRange.getEnd() < newEndpoint)
                    selectionRange.setEnd (newEndpoint);

                selectionRange.setStart (newEndpoint);
            }
            else
            {
                if (selectionRange.getStart() > newEndpoint)
                    selectionRange.setStart (newEndpoint);

                selectionRange.setEnd (newEndpoint);
            }
        }

        ComSmartPtr<UIATextProvider> owner;
        Range<i32> selectionRange;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIATextRangeProvider)
    };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIATextProvider)
};

} // namespace drx
