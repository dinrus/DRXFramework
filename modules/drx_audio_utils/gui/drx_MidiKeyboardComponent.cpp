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
MidiKeyboardComponent::MidiKeyboardComponent (MidiKeyboardState& stateToUse, Orientation orientationToUse)
    : KeyboardComponentBase (orientationToUse), state (stateToUse)
{
    state.addListener (this);

    // initialise with a default set of qwerty key-mappings.
    const std::string_view keys { "awsedftgyhujkolp;" };

    for (const t8& c : keys)
        setKeyPressForNote ({c, 0, 0}, (i32) std::distance (keys.data(), &c));

    mouseOverNotes.insertMultiple (0, -1, 32);
    mouseDownNotes.insertMultiple (0, -1, 32);

    colourChanged();
    setWantsKeyboardFocus (true);

    startTimerHz (20);
}

MidiKeyboardComponent::~MidiKeyboardComponent()
{
    state.removeListener (this);
}

//==============================================================================
z0 MidiKeyboardComponent::setVelocity (f32 v, b8 useMousePosition)
{
    velocity = v;
    useMousePositionForVelocity = useMousePosition;
}

//==============================================================================
z0 MidiKeyboardComponent::setMidiChannel (i32 midiChannelNumber)
{
    jassert (midiChannelNumber > 0 && midiChannelNumber <= 16);

    if (midiChannel != midiChannelNumber)
    {
        resetAnyKeysInUse();
        midiChannel = jlimit (1, 16, midiChannelNumber);
    }
}

z0 MidiKeyboardComponent::setMidiChannelsToDisplay (i32 midiChannelMask)
{
    midiInChannelMask = midiChannelMask;
    noPendingUpdates.store (false);
}

//==============================================================================
z0 MidiKeyboardComponent::clearKeyMappings()
{
    resetAnyKeysInUse();
    keyPressNotes.clear();
    keyPresses.clear();
}

z0 MidiKeyboardComponent::setKeyPressForNote (const KeyPress& key, i32 midiNoteOffsetFromC)
{
    removeKeyPressForNote (midiNoteOffsetFromC);

    keyPressNotes.add (midiNoteOffsetFromC);
    keyPresses.add (key);
}

z0 MidiKeyboardComponent::removeKeyPressForNote (i32 midiNoteOffsetFromC)
{
    for (i32 i = keyPressNotes.size(); --i >= 0;)
    {
        if (keyPressNotes.getUnchecked (i) == midiNoteOffsetFromC)
        {
            keyPressNotes.remove (i);
            keyPresses.remove (i);
        }
    }
}

z0 MidiKeyboardComponent::setKeyPressBaseOctave (i32 newOctaveNumber)
{
    jassert (newOctaveNumber >= 0 && newOctaveNumber <= 10);

    keyMappingOctave = newOctaveNumber;
}

//==============================================================================
z0 MidiKeyboardComponent::resetAnyKeysInUse()
{
    if (! keysPressed.isZero())
    {
        for (i32 i = 128; --i >= 0;)
            if (keysPressed[i])
                state.noteOff (midiChannel, i, 0.0f);

        keysPressed.clear();
    }

    for (i32 i = mouseDownNotes.size(); --i >= 0;)
    {
        auto noteDown = mouseDownNotes.getUnchecked (i);

        if (noteDown >= 0)
        {
            state.noteOff (midiChannel, noteDown, 0.0f);
            mouseDownNotes.set (i, -1);
        }

        mouseOverNotes.set (i, -1);
    }
}

z0 MidiKeyboardComponent::updateNoteUnderMouse (const MouseEvent& e, b8 isDown)
{
    updateNoteUnderMouse (e.getEventRelativeTo (this).position, isDown, e.source.getIndex());
}

z0 MidiKeyboardComponent::updateNoteUnderMouse (Point<f32> pos, b8 isDown, i32 fingerNum)
{
    const auto noteInfo = getNoteAndVelocityAtPosition (pos);
    const auto newNote = noteInfo.note;
    const auto oldNote = mouseOverNotes.getUnchecked (fingerNum);
    const auto oldNoteDown = mouseDownNotes.getUnchecked (fingerNum);
    const auto eventVelocity = useMousePositionForVelocity ? noteInfo.velocity * velocity : velocity;

    if (oldNote != newNote)
    {
        repaintNote (oldNote);
        repaintNote (newNote);
        mouseOverNotes.set (fingerNum, newNote);
    }

    if (isDown)
    {
        if (newNote != oldNoteDown)
        {
            if (oldNoteDown >= 0)
            {
                mouseDownNotes.set (fingerNum, -1);

                if (! mouseDownNotes.contains (oldNoteDown))
                    state.noteOff (midiChannel, oldNoteDown, eventVelocity);
            }

            if (newNote >= 0 && ! mouseDownNotes.contains (newNote))
            {
                state.noteOn (midiChannel, newNote, eventVelocity);
                mouseDownNotes.set (fingerNum, newNote);
            }
        }
    }
    else if (oldNoteDown >= 0)
    {
        mouseDownNotes.set (fingerNum, -1);

        if (! mouseDownNotes.contains (oldNoteDown))
            state.noteOff (midiChannel, oldNoteDown, eventVelocity);
    }
}

z0 MidiKeyboardComponent::repaintNote (i32 noteNum)
{
    if (getRangeStart() <= noteNum && noteNum <= getRangeEnd())
        repaint (getRectangleForKey (noteNum).getSmallestIntegerContainer());
}


z0 MidiKeyboardComponent::mouseMove (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

z0 MidiKeyboardComponent::mouseDrag (const MouseEvent& e)
{
    auto newNote = getNoteAndVelocityAtPosition (e.position).note;

    if (newNote >= 0 && mouseDraggedToKey (newNote, e))
        updateNoteUnderMouse (e, true);
}

z0 MidiKeyboardComponent::mouseDown (const MouseEvent& e)
{
    auto newNote = getNoteAndVelocityAtPosition (e.position).note;

    if (newNote >= 0 && mouseDownOnKey (newNote, e))
        updateNoteUnderMouse (e, true);
}

z0 MidiKeyboardComponent::mouseUp (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);

    auto note = getNoteAndVelocityAtPosition (e.position).note;

    if (note >= 0)
        mouseUpOnKey (note, e);
}

z0 MidiKeyboardComponent::mouseEnter (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

z0 MidiKeyboardComponent::mouseExit (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

z0 MidiKeyboardComponent::timerCallback()
{
    if (noPendingUpdates.exchange (true))
        return;

    for (auto i = getRangeStart(); i <= getRangeEnd(); ++i)
    {
        const auto isOn = state.isNoteOnForChannels (midiInChannelMask, i);

        if (keysCurrentlyDrawnDown[i] != isOn)
        {
            keysCurrentlyDrawnDown.setBit (i, isOn);
            repaintNote (i);
        }
    }
}

b8 MidiKeyboardComponent::keyStateChanged (b8 /*isKeyDown*/)
{
    b8 keyPressUsed = false;

    for (i32 i = keyPresses.size(); --i >= 0;)
    {
        auto note = 12 * keyMappingOctave + keyPressNotes.getUnchecked (i);

        if (keyPresses.getReference (i).isCurrentlyDown())
        {
            if (! keysPressed[note])
            {
                keysPressed.setBit (note);
                state.noteOn (midiChannel, note, velocity);
                keyPressUsed = true;
            }
        }
        else
        {
            if (keysPressed[note])
            {
                keysPressed.clearBit (note);
                state.noteOff (midiChannel, note, 0.0f);
                keyPressUsed = true;
            }
        }
    }

    return keyPressUsed;
}

b8 MidiKeyboardComponent::keyPressed (const KeyPress& key)
{
    return keyPresses.contains (key);
}

z0 MidiKeyboardComponent::focusLost (FocusChangeType)
{
    resetAnyKeysInUse();
}

//==============================================================================
z0 MidiKeyboardComponent::drawKeyboardBackground (Graphics& g, Rectangle<f32> area)
{
    g.fillAll (findColor (whiteNoteColorId));

    auto width = area.getWidth();
    auto height = area.getHeight();
    auto currentOrientation = getOrientation();
    Point<f32> shadowGradientStart, shadowGradientEnd;

    if (currentOrientation == verticalKeyboardFacingLeft)
    {
        shadowGradientStart.x = width - 1.0f;
        shadowGradientEnd.x   = width - 5.0f;
    }
    else if (currentOrientation == verticalKeyboardFacingRight)
    {
        shadowGradientEnd.x = 5.0f;
    }
    else
    {
        shadowGradientEnd.y = 5.0f;
    }

    auto keyboardWidth = getRectangleForKey (getRangeEnd()).getRight();
    auto shadowColor = findColor (shadowColorId);

    if (! shadowColor.isTransparent())
    {
        g.setGradientFill ({ shadowColor, shadowGradientStart,
                             shadowColor.withAlpha (0.0f), shadowGradientEnd,
                             false });

        switch (currentOrientation)
        {
            case horizontalKeyboard:            g.fillRect (0.0f, 0.0f, keyboardWidth, 5.0f); break;
            case verticalKeyboardFacingLeft:    g.fillRect (width - 5.0f, 0.0f, 5.0f, keyboardWidth); break;
            case verticalKeyboardFacingRight:   g.fillRect (0.0f, 0.0f, 5.0f, keyboardWidth); break;
            default: break;
        }
    }

    auto lineColor = findColor (keySeparatorLineColorId);

    if (! lineColor.isTransparent())
    {
        g.setColor (lineColor);

        switch (currentOrientation)
        {
            case horizontalKeyboard:            g.fillRect (0.0f, height - 1.0f, keyboardWidth, 1.0f); break;
            case verticalKeyboardFacingLeft:    g.fillRect (0.0f, 0.0f, 1.0f, keyboardWidth); break;
            case verticalKeyboardFacingRight:   g.fillRect (width - 1.0f, 0.0f, 1.0f, keyboardWidth); break;
            default: break;
        }
    }
}

z0 MidiKeyboardComponent::drawWhiteNote (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area,
                                           b8 isDown, b8 isOver, Color lineColor, Color textColor)
{
    auto c = Colors::transparentWhite;

    if (isDown)  c = findColor (keyDownOverlayColorId);
    if (isOver)  c = c.overlaidWith (findColor (mouseOverKeyOverlayColorId));

    g.setColor (c);
    g.fillRect (area);

    const auto currentOrientation = getOrientation();

    auto text = getWhiteNoteText (midiNoteNumber);

    if (text.isNotEmpty())
    {
        auto fontHeight = jmin (12.0f, getKeyWidth() * 0.9f);

        g.setColor (textColor);
        g.setFont (withDefaultMetrics (FontOptions { fontHeight }).withHorizontalScale (0.8f));

        switch (currentOrientation)
        {
            case horizontalKeyboard:            g.drawText (text, area.withTrimmedLeft (1.0f).withTrimmedBottom (2.0f), Justification::centredBottom, false); break;
            case verticalKeyboardFacingLeft:    g.drawText (text, area.reduced (2.0f), Justification::centredLeft,   false); break;
            case verticalKeyboardFacingRight:   g.drawText (text, area.reduced (2.0f), Justification::centredRight,  false); break;
            default: break;
        }
    }

    if (! lineColor.isTransparent())
    {
        g.setColor (lineColor);

        switch (currentOrientation)
        {
            case horizontalKeyboard:            g.fillRect (area.withWidth (1.0f)); break;
            case verticalKeyboardFacingLeft:    g.fillRect (area.withHeight (1.0f)); break;
            case verticalKeyboardFacingRight:   g.fillRect (area.removeFromBottom (1.0f)); break;
            default: break;
        }

        if (midiNoteNumber == getRangeEnd())
        {
            switch (currentOrientation)
            {
                case horizontalKeyboard:            g.fillRect (area.expanded (1.0f, 0).removeFromRight (1.0f)); break;
                case verticalKeyboardFacingLeft:    g.fillRect (area.expanded (0, 1.0f).removeFromBottom (1.0f)); break;
                case verticalKeyboardFacingRight:   g.fillRect (area.expanded (0, 1.0f).removeFromTop (1.0f)); break;
                default: break;
            }
        }
    }
}

z0 MidiKeyboardComponent::drawBlackNote (i32 /*midiNoteNumber*/, Graphics& g, Rectangle<f32> area,
                                           b8 isDown, b8 isOver, Color noteFillColor)
{
    auto c = noteFillColor;

    if (isDown)  c = c.overlaidWith (findColor (keyDownOverlayColorId));
    if (isOver)  c = c.overlaidWith (findColor (mouseOverKeyOverlayColorId));

    g.setColor (c);
    g.fillRect (area);

    if (isDown)
    {
        g.setColor (noteFillColor);
        g.drawRect (area);
    }
    else
    {
        g.setColor (c.brighter());
        auto sideIndent = 1.0f / 8.0f;
        auto topIndent = 7.0f / 8.0f;
        auto w = area.getWidth();
        auto h = area.getHeight();

        switch (getOrientation())
        {
            case horizontalKeyboard:            g.fillRect (area.reduced (w * sideIndent, 0).removeFromTop   (h * topIndent)); break;
            case verticalKeyboardFacingLeft:    g.fillRect (area.reduced (0, h * sideIndent).removeFromRight (w * topIndent)); break;
            case verticalKeyboardFacingRight:   g.fillRect (area.reduced (0, h * sideIndent).removeFromLeft  (w * topIndent)); break;
            default: break;
        }
    }
}

Txt MidiKeyboardComponent::getWhiteNoteText (i32 midiNoteNumber)
{
    if (midiNoteNumber % 12 == 0)
        return MidiMessage::getMidiNoteName (midiNoteNumber, true, true, getOctaveForMiddleC());

    return {};
}

z0 MidiKeyboardComponent::colourChanged()
{
    setOpaque (findColor (whiteNoteColorId).isOpaque());
    repaint();
}

//==============================================================================
z0 MidiKeyboardComponent::drawWhiteKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area)
{
    drawWhiteNote (midiNoteNumber, g, area, state.isNoteOnForChannels (midiInChannelMask, midiNoteNumber),
                   mouseOverNotes.contains (midiNoteNumber), findColor (keySeparatorLineColorId), findColor (textLabelColorId));
}

z0 MidiKeyboardComponent::drawBlackKey (i32 midiNoteNumber, Graphics& g, Rectangle<f32> area)
{
    drawBlackNote (midiNoteNumber, g, area, state.isNoteOnForChannels (midiInChannelMask, midiNoteNumber),
                   mouseOverNotes.contains (midiNoteNumber), findColor (blackNoteColorId));
}

//==============================================================================
z0 MidiKeyboardComponent::handleNoteOn (MidiKeyboardState*, i32 /*midiChannel*/, i32 /*midiNoteNumber*/, f32 /*velocity*/)
{
    noPendingUpdates.store (false);
}

z0 MidiKeyboardComponent::handleNoteOff (MidiKeyboardState*, i32 /*midiChannel*/, i32 /*midiNoteNumber*/, f32 /*velocity*/)
{
    noPendingUpdates.store (false);
}

//==============================================================================
b8 MidiKeyboardComponent::mouseDownOnKey    ([[maybe_unused]] i32 midiNoteNumber, [[maybe_unused]] const MouseEvent& e)  { return true; }
b8 MidiKeyboardComponent::mouseDraggedToKey ([[maybe_unused]] i32 midiNoteNumber, [[maybe_unused]] const MouseEvent& e)  { return true; }
z0 MidiKeyboardComponent::mouseUpOnKey      ([[maybe_unused]] i32 midiNoteNumber, [[maybe_unused]] const MouseEvent& e)  {}

} // namespace drx
