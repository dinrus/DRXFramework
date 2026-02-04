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
/**
    A base class for drawing a custom MIDI keyboard component.

    Implement the drawKeyboardBackground(), drawWhiteKey(), and drawBlackKey() methods
    to draw your content and this class will handle the underlying keyboard logic.

    The component is a ChangeBroadcaster, so if you want to be informed when the
    keyboard is scrolled, you can register a ChangeListener for callbacks.

    @tags{Audio}
*/
class DRX_API  KeyboardComponentBase  : public Component,
                                         public ChangeBroadcaster
{
public:
    //==============================================================================
    /** The direction of the keyboard.

        @see setOrientation
    */
    enum Orientation
    {
        horizontalKeyboard,
        verticalKeyboardFacingLeft,
        verticalKeyboardFacingRight,
    };

    //==============================================================================
    /** Constructor.

        @param orientation  whether the keyboard is horizontal or vertical
    */
    explicit KeyboardComponentBase (Orientation orientation);

    /** Destructor. */
    ~KeyboardComponentBase() override = default;

    //==============================================================================
    /** Changes the width used to draw the white keys. */
    z0 setKeyWidth (f32 widthInPixels);

    /** Returns the width that was set by setKeyWidth(). */
    f32 getKeyWidth() const noexcept                              { return keyWidth; }

    /** Changes the width used to draw the buttons that scroll the keyboard up/down in octaves. */
    z0 setScrollButtonWidth (i32 widthInPixels);

    /** Returns the width that was set by setScrollButtonWidth(). */
    i32 getScrollButtonWidth() const noexcept                       { return scrollButtonWidth; }

    /** Changes the keyboard's current direction. */
    z0 setOrientation (Orientation newOrientation);

    /** Returns the keyboard's current direction. */
    Orientation getOrientation() const noexcept                     { return orientation; }

    /** Возвращает true, если the keyboard's orientation is horizontal. */
    b8 isHorizontal() const noexcept                              { return orientation == horizontalKeyboard; }

    /** Sets the range of midi notes that the keyboard will be limited to.

        By default the range is 0 to 127 (inclusive), but you can limit this if you
        only want a restricted set of the keys to be shown.

        Note that the values here are inclusive and must be between 0 and 127.
    */
    z0 setAvailableRange (i32 lowestNote, i32 highestNote);

    /** Returns the first note in the available range.

        @see setAvailableRange
    */
    i32 getRangeStart() const noexcept                              { return rangeStart; }

    /** Returns the last note in the available range.

        @see setAvailableRange
    */
    i32 getRangeEnd() const noexcept                                { return rangeEnd; }

    /** If the keyboard extends beyond the size of the component, this will scroll
        it to show the given key at the start.

        Whenever the keyboard's position is changed, this will use the ChangeBroadcaster
        base class to send a callback to any ChangeListeners that have been registered.
    */
    z0 setLowestVisibleKey (i32 noteNumber);

    /** Returns the number of the first key shown in the component.

        @see setLowestVisibleKey
    */
    i32 getLowestVisibleKey() const noexcept                        { return (i32) firstKey; }

    /** Returns the absolute length of the white notes.

        This will be their vertical or horizontal length, depending on the keyboard's orientation.
    */
    f32 getWhiteNoteLength() const noexcept;

    /** Sets the length of the black notes as a proportion of the white note length. */
    z0 setBlackNoteLengthProportion (f32 ratio) noexcept;

    /** Returns the length of the black notes as a proportion of the white note length. */
    f32 getBlackNoteLengthProportion() const noexcept             { return blackNoteLengthRatio; }

    /** Returns the absolute length of the black notes.

        This will be their vertical or horizontal length, depending on the keyboard's orientation.
    */
    f32 getBlackNoteLength() const noexcept;

    /** Sets the width of the black notes as a proportion of the white note width. */
    z0 setBlackNoteWidthProportion (f32 ratio) noexcept;

    /** Returns the width of the black notes as a proportion of the white note width. */
    f32 getBlackNoteWidthProportion() const noexcept             { return blackNoteWidthRatio; }

    /** Returns the absolute width of the black notes.

        This will be their vertical or horizontal width, depending on the keyboard's orientation.
    */
    f32 getBlackNoteWidth() const noexcept                       { return keyWidth * blackNoteWidthRatio; }

    /** If set to true, then scroll buttons will appear at either end of the keyboard
        if there are too many notes to fit them all in the component at once.
    */
    z0 setScrollButtonsVisible (b8 canScroll);

    //==============================================================================
    /** Color IDs to use to change the colour of the octave scroll buttons.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        upDownButtonBackgroundColorId  = 0x1004000,
        upDownButtonArrowColorId       = 0x1004001
    };

    /** Returns the position within the component of the left-hand edge of a key.

        Depending on the keyboard's orientation, this may be a horizontal or vertical
        distance, in either direction.
    */
    f32 getKeyStartPosition (i32 midiNoteNumber) const;

    /** Returns the total width needed to fit all the keys in the available range. */
    f32 getTotalKeyboardWidth() const noexcept;

    /** This structure is returned by the getNoteAndVelocityAtPosition() method.
    */
    struct DRX_API  NoteAndVelocity
    {
        i32 note;
        f32 velocity;
    };

    /** Returns the note number and velocity for a given position within the component.

        If includeChildComponents is true then this will return a key obscured by any child
        components.
    */
    NoteAndVelocity getNoteAndVelocityAtPosition (Point<f32> position, b8 includeChildComponents = false);

   #ifndef DOXYGEN
    /** Returns the key at a given coordinate, or -1 if the position does not intersect a key. */
    [[deprecated ("This method has been deprecated in favour of getNoteAndVelocityAtPosition.")]]
    i32 getNoteAtPosition (Point<f32> p)  { return getNoteAndVelocityAtPosition (p).note; }
   #endif

    /** Returns the rectangle for a given key. */
    Rectangle<f32> getRectangleForKey (i32 midiNoteNumber) const;

    //==============================================================================
    /** This sets the octave number which is shown as the octave number for middle C.

        This affects only the default implementation of getWhiteNoteText(), which
        passes this octave number to MidiMessage::getMidiNoteName() in order to
        get the note text. See MidiMessage::getMidiNoteName() for more info about
        the parameter.

        By default this value is set to 3.

        @see getOctaveForMiddleC
    */
    z0 setOctaveForMiddleC (i32 octaveNumForMiddleC);

    /** This returns the value set by setOctaveForMiddleC().

        @see setOctaveForMiddleC
    */
    i32 getOctaveForMiddleC() const noexcept            { return octaveNumForMiddleC; }

    //==============================================================================
    /** Use this method to draw the background of the keyboard that will be drawn under
        the white and black notes. This can also be used to draw any shadow or outline effects.
    */
    virtual z0 drawKeyboardBackground (Graphics& g, Rectangle<f32> area) = 0;

    /** Use this method to draw a white key of the keyboard in a given rectangle.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual z0 drawWhiteKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area) = 0;

    /** Use this method to draw a black key of the keyboard in a given rectangle.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual z0 drawBlackKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area) = 0;

    /** This can be overridden to draw the up and down buttons that scroll the keyboard
        up/down in octaves.
    */
    virtual z0 drawUpDownButton (Graphics& g, i32 w, i32 h, b8 isMouseOver, b8 isButtonPressed, b8 movesOctavesUp);

    /** Calculates the position of a given midi-note.

        This can be overridden to create layouts with custom key-widths.

        @param midiNoteNumber   the note to find
        @param keyWidth         the desired width in pixels of one key - see setKeyWidth()
        @returns                the start and length of the key along the axis of the keyboard
    */
    virtual Range<f32> getKeyPosition (i32 midiNoteNumber, f32 keyWidth) const;

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;

private:
    //==============================================================================
    struct UpDownButton;

    Range<f32> getKeyPos (i32 midiNoteNumber) const;
    NoteAndVelocity remappedXYToNote (Point<f32>) const;
    z0 setLowestVisibleKeyFloat (f32 noteNumber);

    //==============================================================================
    Orientation orientation;

    f32 blackNoteLengthRatio = 0.7f, blackNoteWidthRatio = 0.7f;
    f32 xOffset = 0.0f;
    f32 keyWidth = 16.0f;
    f32 firstKey = 12 * 4.0f;

    i32 scrollButtonWidth = 12;
    i32 rangeStart = 0, rangeEnd = 127;
    i32 octaveNumForMiddleC = 3;

    b8 canScroll = true;
    std::unique_ptr<Button> scrollDown, scrollUp;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardComponentBase)
};

} // namespace drx
