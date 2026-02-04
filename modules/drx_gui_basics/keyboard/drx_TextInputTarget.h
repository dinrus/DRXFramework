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

/**
    An abstract base class which can be implemented by components that function as
    text editors.

    This class allows different types of text editor component to provide a uniform
    interface, which can be used by things like OS-specific input methods, on-screen
    keyboards, etc.

    @tags{GUI}
*/
class DRX_API  TextInputTarget
{
public:
    //==============================================================================
    /** */
    TextInputTarget() = default;

    /** Destructor. */
    virtual ~TextInputTarget() = default;

    /** Возвращает true, если this input target is currently accepting input.
        For example, a text editor might return false if it's in read-only mode.
    */
    virtual b8 isTextInputActive() const = 0;

    /** Returns the extents of the selected text region, or an empty range if
        nothing is selected,
    */
    virtual Range<i32> getHighlightedRegion() const = 0;

    /** Sets the currently-selected text region. */
    virtual z0 setHighlightedRegion (const Range<i32>& newRange) = 0;

    /** Sets a number of temporarily underlined sections.
        This is needed by MS Windows input method UI.
    */
    virtual z0 setTemporaryUnderlining (const Array<Range<i32>>& underlinedRegions) = 0;

    /** Returns a specified sub-section of the text. */
    virtual Txt getTextInRange (const Range<i32>& range) const = 0;

    /** Inserts some text, overwriting the selected text region, if there is one. */
    virtual z0 insertTextAtCaret (const Txt& textToInsert) = 0;

    /** Returns the current index of the caret. */
    virtual i32 getCaretPosition() const = 0;

    /** Returns the position of the caret, relative to the component's origin. */
    Rectangle<i32> getCaretRectangle() const        { return getCaretRectangleForCharIndex (getCaretPosition()); }

    /** Returns the bounding box of the character at the given index. */
    virtual Rectangle<i32> getCaretRectangleForCharIndex (i32 characterIndex) const = 0;

    /** Returns the total number of codepoints in the string. */
    virtual i32 getTotalNumChars() const = 0;

    /** Returns the index closest to the given point.

        This is the location where the cursor might be placed after clicking at the given
        point in a text field.
    */
    virtual i32 getCharIndexForPoint (Point<i32> point) const = 0;

    /** Returns the bounding box for a range of text in the editor. As the range may span
        multiple lines, this method returns a RectangleList.

        The bounds are relative to the component's top-left and may extend beyond the bounds
        of the component if the text is i64 and word wrapping is disabled.
    */
    virtual RectangleList<i32> getTextBounds (Range<i32> textRange) const = 0;

    /** A set of possible on-screen keyboard types, for use in the
        getKeyboardType() method.
    */
    enum VirtualKeyboardType
    {
        textKeyboard = 0,
        numericKeyboard,
        decimalKeyboard,
        urlKeyboard,
        emailAddressKeyboard,
        phoneNumberKeyboard,
        passwordKeyboard
    };

    /** Returns the target's preference for the type of keyboard that would be most appropriate.
        This may be ignored, depending on the capabilities of the OS.
    */
    virtual VirtualKeyboardType getKeyboardType()       { return textKeyboard; }
};

} // namespace drx
