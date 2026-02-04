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
    A component that displays a piano keyboard, whose notes can be clicked on.

    This component will mimic a physical midi keyboard, showing the current state of
    a MidiKeyboardState object. When the on-screen keys are clicked on, it will play these
    notes by calling the noteOn() and noteOff() methods of its MidiKeyboardState object.

    Another feature is that the computer keyboard can also be used to play notes. By
    default it maps the top two rows of a standard qwerty keyboard to the notes, but
    these can be remapped if needed. It will only respond to keypresses when it has
    the keyboard focus, so to disable this feature you can call setWantsKeyboardFocus (false).

    The component is also a ChangeBroadcaster, so if you want to be informed when the
    keyboard is scrolled, you can register a ChangeListener for callbacks.

    @see MidiKeyboardState

    @tags{Audio}
*/
class DRX_API  MidiKeyboardComponent  : public KeyboardComponentBase,
                                         private MidiKeyboardState::Listener,
                                         private Timer
{
public:
    //==============================================================================
    /** Creates a MidiKeyboardComponent.

        @param state        the midi keyboard model that this component will represent
        @param orientation  whether the keyboard is horizontal or vertical
    */
    MidiKeyboardComponent (MidiKeyboardState& state, Orientation orientation);

    /** Destructor. */
    ~MidiKeyboardComponent() override;

    //==============================================================================
    /** Changes the velocity used in midi note-on messages that are triggered by clicking
        on the component.

        Values are 0 to 1.0, where 1.0 is the heaviest.

        @see setMidiChannel
    */
    z0 setVelocity (f32 velocity, b8 useMousePositionForVelocity);

    //==============================================================================
    /** Changes the midi channel number that will be used for events triggered by clicking
        on the component.

        The channel must be between 1 and 16 (inclusive). This is the channel that will be
        passed on to the MidiKeyboardState::noteOn() method when the user clicks the component.

        Although this is the channel used for outgoing events, the component can display
        incoming events from more than one channel - see setMidiChannelsToDisplay()

        @see setVelocity
    */
    z0 setMidiChannel (i32 midiChannelNumber);

    /** Returns the midi channel that the keyboard is using for midi messages.
        @see setMidiChannel
    */
    i32 getMidiChannel() const noexcept            { return midiChannel; }

    /** Sets a mask to indicate which incoming midi channels should be represented by
        key movements.

        The mask is a set of bits, where bit 0 = midi channel 1, bit 1 = midi channel 2, etc.

        If the MidiKeyboardState has a key down for any of the channels whose bits are set
        in this mask, the on-screen keys will also go down.

        By default, this mask is set to 0xffff (all channels displayed).

        @see setMidiChannel
    */
    z0 setMidiChannelsToDisplay (i32 midiChannelMask);

    /** Returns the current set of midi channels represented by the component.
        This is the value that was set with setMidiChannelsToDisplay().
    */
    i32 getMidiChannelsToDisplay() const noexcept  { return midiInChannelMask; }

    //==============================================================================
    /** Deletes all key-mappings.

        @see setKeyPressForNote
    */
    z0 clearKeyMappings();

    /** Maps a key-press to a given note.

        @param key                  the key that should trigger the note
        @param midiNoteOffsetFromC  how many semitones above C the triggered note should
                                    be. The actual midi note that gets played will be
                                    this value + (12 * the current base octave). To change
                                    the base octave, see setKeyPressBaseOctave()
    */
    z0 setKeyPressForNote (const KeyPress& key, i32 midiNoteOffsetFromC);

    /** Removes any key-mappings for a given note.

        For a description of what the note number means, see setKeyPressForNote().
    */
    z0 removeKeyPressForNote (i32 midiNoteOffsetFromC);

    /** Changes the base note above which key-press-triggered notes are played.

        The set of key-mappings that trigger notes can be moved up and down to cover
        the entire scale using this method.

        The value passed in is an octave number between 0 and 10 (inclusive), and
        indicates which C is the base note to which the key-mapped notes are
        relative.
    */
    z0 setKeyPressBaseOctave (i32 newOctaveNumber);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        whiteNoteColorId               = 0x1005000,
        blackNoteColorId               = 0x1005001,
        keySeparatorLineColorId        = 0x1005002,
        mouseOverKeyOverlayColorId     = 0x1005003,  /**< This colour will be overlaid on the normal note colour. */
        keyDownOverlayColorId          = 0x1005004,  /**< This colour will be overlaid on the normal note colour. */
        textLabelColorId               = 0x1005005,
        shadowColorId                  = 0x1005006
    };

    //==============================================================================
    /** Use this method to draw a white note of the keyboard in a given rectangle.

        isOver indicates whether the mouse is over the key, isDown indicates whether the key is
        currently pressed down.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual z0 drawWhiteNote (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area,
                                b8 isDown, b8 isOver, Color lineColor, Color textColor);

    /** Use this method to draw a black note of the keyboard in a given rectangle.

        isOver indicates whether the mouse is over the key, isDown indicates whether the key is
        currently pressed down.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual z0 drawBlackNote (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area,
                                b8 isDown, b8 isOver, Color noteFillColor);

    /** Callback when the mouse is clicked on a key.

        You could use this to do things like handle right-clicks on keys, etc.

        Return true if you want the click to trigger the note, or false if you
        want to handle it yourself and not have the note played.

        @see mouseDraggedToKey
    */
    virtual b8 mouseDownOnKey (i32 midiNoteNumber, const MouseEvent& e);

    /** Callback when the mouse is dragged from one key onto another.

        Return true if you want the drag to trigger the new note, or false if you
        want to handle it yourself and not have the note played.

        @see mouseDownOnKey
    */
    virtual b8 mouseDraggedToKey (i32 midiNoteNumber, const MouseEvent& e);

    /** Callback when the mouse is released from a key.

        @see mouseDownOnKey
    */
    virtual z0 mouseUpOnKey (i32 midiNoteNumber, const MouseEvent& e);

    /** Allows text to be drawn on the white notes.

        By default this is used to label the C in each octave, but could be used for other things.

        @see setOctaveForMiddleC
    */
    virtual Txt getWhiteNoteText (i32 midiNoteNumber);

    //==============================================================================
    /** @internal */
    z0 mouseMove (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 mouseEnter (const MouseEvent&) override;
    /** @internal */
    z0 mouseExit (const MouseEvent&) override;
    /** @internal */
    z0 timerCallback() override;
    /** @internal */
    b8 keyStateChanged (b8 isKeyDown) override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 focusLost (FocusChangeType) override;
    /** @internal */
    z0 colourChanged() override;

private:
    //==============================================================================
    z0 drawKeyboardBackground (Graphics& g, Rectangle<f32> area) override final;
    z0 drawWhiteKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area) override final;
    z0 drawBlackKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area) override final;

    z0 handleNoteOn  (MidiKeyboardState*, i32, i32, f32) override;
    z0 handleNoteOff (MidiKeyboardState*, i32, i32, f32) override;

    //==============================================================================
    z0 resetAnyKeysInUse();
    z0 updateNoteUnderMouse (Point<f32>, b8 isDown, i32 fingerNum);
    z0 updateNoteUnderMouse (const MouseEvent&, b8 isDown);
    z0 repaintNote (i32 midiNoteNumber);

    //==============================================================================
    MidiKeyboardState& state;
    i32 midiChannel = 1, midiInChannelMask = 0xffff;
    i32 keyMappingOctave = 6;

    f32 velocity = 1.0f;
    b8 useMousePositionForVelocity = true;

    Array<i32> mouseOverNotes, mouseDownNotes;
    Array<KeyPress> keyPresses;
    Array<i32> keyPressNotes;
    BigInteger keysPressed, keysCurrentlyDrawnDown;

    std::atomic<b8> noPendingUpdates { true };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiKeyboardComponent)
};

} // namespace drx
