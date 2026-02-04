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

/** An abstract interface which represents a UI element that supports a text interface.

    A UI element can use this interface to provide extended textual information which
    cannot be conveyed using just the title, description, and help text properties of
    AccessibilityHandler. This is typically for text that an accessibility client might
    want to read line-by-line, or provide text selection and input for.

    @tags{Accessibility}
*/
class DRX_API  AccessibilityTextInterface
{
public:
    /** Destructor. */
    virtual ~AccessibilityTextInterface() = default;

    /** Возвращает true, если the text being displayed is protected and should not be
        exposed to the user, for example a password entry field.
    */
    virtual b8 isDisplayingProtectedText() const = 0;

    /** Возвращает true, если the text being displayed is read-only or false if editable. */
    virtual b8 isReadOnly() const = 0;

    /** Returns the total number of characters in the text element. */
    virtual i32 getTotalNumCharacters() const = 0;

    /** Returns the range of characters that are currently selected, or an empty
        range if nothing is selected.
    */
    virtual Range<i32> getSelection() const = 0;

    /** Selects a section of the text. */
    virtual z0 setSelection (Range<i32> newRange) = 0;

    /** Gets the current text insertion position, if supported. */
    virtual i32 getTextInsertionOffset() const = 0;

    /** Returns a section of text. */
    virtual Txt getText (Range<i32> range) const = 0;

    /** Returns the full text. */
    Txt getAllText() const { return getText ({ 0, getTotalNumCharacters() }); }

    /** Replaces the text with a new string. */
    virtual z0 setText (const Txt& newText) = 0;

    /** Returns the bounding box in screen coordinates for a range of text.
        As the range may span multiple lines, this method returns a RectangleList.
    */
    virtual RectangleList<i32> getTextBounds (Range<i32> textRange) const = 0;

    /** Returns the index of the character at a given position in screen coordinates. */
    virtual i32 getOffsetAtPoint (Point<i32> point) const = 0;
};

} // namespace drx
