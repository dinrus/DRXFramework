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
    A component that displays an MPE-compatible keyboard, whose notes can be clicked on.

    This component will mimic a physical MPE-compatible keyboard, showing the current state
    of an MPEInstrument object. When the on-screen keys are clicked on, it will play these
    notes by calling the noteOn() and noteOff() methods of its MPEInstrument object. Moving
    the mouse will update the pitchbend and timbre dimensions of the MPEInstrument.

    @see MPEInstrument

    @tags{Audio}
*/
class DRX_API  MPEKeyboardComponent  : public KeyboardComponentBase,
                                        private MPEInstrument::Listener,
                                        private Timer
{
public:
    //==============================================================================
    /** Creates an MPEKeyboardComponent.

        @param instrument   the MPEInstrument that this component represents
        @param orientation  whether the keyboard is horizontal or vertical
    */
    MPEKeyboardComponent (MPEInstrument& instrument, Orientation orientation);

    /** Destructor. */
    virtual ~MPEKeyboardComponent() override;

    //==============================================================================
    /** Sets the note-on velocity, or "strike", value that will be used when triggering new notes. */
    z0 setVelocity (f32 newVelocity)                                 { velocity = jlimit (0.0f, 1.0f, newVelocity); }

    /** Sets the pressure value that will be used for new notes. */
    z0 setPressure (f32 newPressure)                                 { pressure = jlimit (0.0f, 1.0f, newPressure); }

    /** Sets the note-off velocity, or "lift", value that will be used when notes are released. */
    z0 setLift (f32 newLift)                                         { lift = jlimit (0.0f, 1.0f, newLift); }

    /** Use this to enable the mouse source pressure to be used for the initial note-on
        velocity, or "strike", value if the mouse source supports it.
    */
    z0 setUseMouseSourcePressureForStrike (b8 usePressureForStrike)  { useMouseSourcePressureForStrike = usePressureForStrike; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        whiteNoteColorId         = 0x1006000,
        blackNoteColorId         = 0x1006001,
        textLabelColorId         = 0x1006002,
        noteCircleFillColorId    = 0x1006003,
        noteCircleOutlineColorId = 0x1006004
    };

    //==============================================================================
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 focusLost (FocusChangeType) override;
    /** @internal */
    z0 colourChanged() override;

private:
    //==============================================================================
    struct MPENoteComponent;

    //==============================================================================
    z0 drawKeyboardBackground (Graphics& g, Rectangle<f32> area) override;
    z0 drawWhiteKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area) override;
    z0 drawBlackKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area) override;

    z0 updateNoteData (MPENote&);

    z0 noteAdded (MPENote) override;
    z0 notePressureChanged (MPENote) override;
    z0 notePitchbendChanged (MPENote) override;
    z0 noteTimbreChanged (MPENote) override;
    z0 noteReleased (MPENote) override;
    z0 zoneLayoutChanged() override;

    z0 timerCallback() override;

    //==============================================================================
    MPEValue mousePositionToPitchbend (i32, Point<f32>);
    MPEValue mousePositionToTimbre (Point<f32>);

    z0 addNewNote (MPENote);
    z0 removeNote (MPENote);

    z0 handleNoteOns  (std::set<MPENote>&);
    z0 handleNoteOffs (std::set<MPENote>&);
    z0 updateNoteComponentBounds (const MPENote&, MPENoteComponent&);
    z0 updateNoteComponents();

    z0 updateZoneLayout();

    //==============================================================================
    MPEInstrument& instrument;
    std::unique_ptr<MPEChannelAssigner> channelAssigner;

    CriticalSection activeNotesLock;
    std::vector<std::pair<MPENote, b8>> activeNotes;
    std::vector<std::unique_ptr<MPENoteComponent>> noteComponents;
    std::map<i32, u16> sourceIDMap;

    f32 velocity = 0.7f, pressure = 1.0f, lift = 0.0f;
    b8 useMouseSourcePressureForStrike = false;
    i32 perNotePitchbendRange = 48;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPEKeyboardComponent)
};

} // namespace drx
