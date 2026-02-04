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

constexpr u8 whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
constexpr u8 blackNotes[] = { 1, 3, 6, 8, 10 };

//==============================================================================
struct KeyboardComponentBase::UpDownButton final : public Button
{
    UpDownButton (KeyboardComponentBase& c, i32 d)
        : Button ({}), owner (c), delta (d)
    {
    }

    z0 clicked() override
    {
        auto note = owner.getLowestVisibleKey();

        note = delta < 0 ? (note - 1) / 12 : note / 12 + 1;

        owner.setLowestVisibleKey (note * 12);
    }

    using Button::clicked;

    z0 paintButton (Graphics& g, b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override
    {
        owner.drawUpDownButton (g, getWidth(), getHeight(),
                                shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown,
                                delta > 0);
    }

private:
    KeyboardComponentBase& owner;
    i32 delta;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UpDownButton)
};

//==============================================================================
KeyboardComponentBase::KeyboardComponentBase (Orientation o)  : orientation (o)
{
    scrollDown = std::make_unique<UpDownButton> (*this, -1);
    scrollUp   = std::make_unique<UpDownButton> (*this, 1);

    addChildComponent (*scrollDown);
    addChildComponent (*scrollUp);

    colourChanged();
}

//==============================================================================
z0 KeyboardComponentBase::setKeyWidth (f32 widthInPixels)
{
    jassert (widthInPixels > 0);

    if (! approximatelyEqual (keyWidth, widthInPixels)) // Prevent infinite recursion if the width is being computed in a 'resized()' callback
    {
        keyWidth = widthInPixels;
        resized();
    }
}

z0 KeyboardComponentBase::setScrollButtonWidth (i32 widthInPixels)
{
    jassert (widthInPixels > 0);

    if (scrollButtonWidth != widthInPixels)
    {
        scrollButtonWidth = widthInPixels;
        resized();
    }
}

z0 KeyboardComponentBase::setOrientation (Orientation newOrientation)
{
    if (orientation != newOrientation)
    {
        orientation = newOrientation;
        resized();
    }
}

z0 KeyboardComponentBase::setAvailableRange (i32 lowestNote, i32 highestNote)
{
    jassert (lowestNote >= 0 && lowestNote <= 127);
    jassert (highestNote >= 0 && highestNote <= 127);
    jassert (lowestNote <= highestNote);

    if (rangeStart != lowestNote || rangeEnd != highestNote)
    {
        rangeStart = jlimit (0, 127, lowestNote);
        rangeEnd = jlimit (0, 127, highestNote);
        firstKey = jlimit ((f32) rangeStart, (f32) rangeEnd, firstKey);
        resized();
    }
}

z0 KeyboardComponentBase::setLowestVisibleKey (i32 noteNumber)
{
    setLowestVisibleKeyFloat ((f32) noteNumber);
}

z0 KeyboardComponentBase::setLowestVisibleKeyFloat (f32 noteNumber)
{
    noteNumber = jlimit ((f32) rangeStart, (f32) rangeEnd, noteNumber);

    if (! approximatelyEqual (noteNumber, firstKey))
    {
        b8 hasMoved = (((i32) firstKey) != (i32) noteNumber);
        firstKey = noteNumber;

        if (hasMoved)
            sendChangeMessage();

        resized();
    }
}

f32 KeyboardComponentBase::getWhiteNoteLength() const noexcept
{
    return (orientation == horizontalKeyboard) ? (f32) getHeight() : (f32) getWidth();
}

z0 KeyboardComponentBase::setBlackNoteLengthProportion (f32 ratio) noexcept
{
    jassert (ratio >= 0.0f && ratio <= 1.0f);

    if (! approximatelyEqual (blackNoteLengthRatio, ratio))
    {
        blackNoteLengthRatio = ratio;
        resized();
    }
}

f32 KeyboardComponentBase::getBlackNoteLength() const noexcept
{
    auto whiteNoteLength = orientation == horizontalKeyboard ? getHeight() : getWidth();
    return (f32) whiteNoteLength * blackNoteLengthRatio;
}

z0 KeyboardComponentBase::setBlackNoteWidthProportion (f32 ratio) noexcept
{
    jassert (ratio >= 0.0f && ratio <= 1.0f);

    if (! approximatelyEqual (blackNoteWidthRatio, ratio))
    {
        blackNoteWidthRatio = ratio;
        resized();
    }
}

z0 KeyboardComponentBase::setScrollButtonsVisible (b8 newCanScroll)
{
    if (canScroll != newCanScroll)
    {
        canScroll = newCanScroll;
        resized();
    }
}

//==============================================================================
Range<f32> KeyboardComponentBase::getKeyPos (i32 midiNoteNumber) const
{
    return getKeyPosition (midiNoteNumber, keyWidth)
             - xOffset
             - getKeyPosition (rangeStart, keyWidth).getStart();
}

f32 KeyboardComponentBase::getKeyStartPosition (i32 midiNoteNumber) const
{
    return getKeyPos (midiNoteNumber).getStart();
}

f32 KeyboardComponentBase::getTotalKeyboardWidth() const noexcept
{
    return getKeyPos (rangeEnd).getEnd();
}

KeyboardComponentBase::NoteAndVelocity KeyboardComponentBase::getNoteAndVelocityAtPosition (Point<f32> pos, b8 children)
{
    if (! reallyContains (pos, children))
        return { -1, 0.0f };

    auto p = pos;

    if (orientation != horizontalKeyboard)
    {
        p = { p.y, p.x };

        if (orientation == verticalKeyboardFacingLeft)
            p = { p.x, (f32) getWidth() - p.y };
        else
            p = { (f32) getHeight() - p.x, p.y };
    }

    return remappedXYToNote (p + Point<f32> (xOffset, 0));
}

KeyboardComponentBase::NoteAndVelocity KeyboardComponentBase::remappedXYToNote (Point<f32> pos) const
{
    auto blackNoteLength = getBlackNoteLength();

    if (pos.getY() < blackNoteLength)
    {
        for (i32 octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
        {
            for (i32 i = 0; i < 5; ++i)
            {
                auto note = octaveStart + blackNotes[i];

                if (rangeStart <= note && note <= rangeEnd)
                {
                    if (getKeyPos (note).contains (pos.x - xOffset))
                    {
                        return { note, jmax (0.0f, pos.y / blackNoteLength) };
                    }
                }
            }
        }
    }

    for (i32 octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
    {
        for (i32 i = 0; i < 7; ++i)
        {
            auto note = octaveStart + whiteNotes[i];

            if (note >= rangeStart && note <= rangeEnd)
            {
                if (getKeyPos (note).contains (pos.x - xOffset))
                {
                    auto whiteNoteLength = (orientation == horizontalKeyboard) ? getHeight() : getWidth();
                    return { note, jmax (0.0f, pos.y / (f32) whiteNoteLength) };
                }
            }
        }
    }

    return { -1, 0 };
}

Rectangle<f32> KeyboardComponentBase::getRectangleForKey (i32 note) const
{
    jassert (note >= rangeStart && note <= rangeEnd);

    auto pos = getKeyPos (note);
    auto x = pos.getStart();
    auto w = pos.getLength();

    if (MidiMessage::isMidiNoteBlack (note))
    {
        auto blackNoteLength = getBlackNoteLength();

        switch (orientation)
        {
            case horizontalKeyboard:            return { x, 0, w, blackNoteLength };
            case verticalKeyboardFacingLeft:    return { (f32) getWidth() - blackNoteLength, x, blackNoteLength, w };
            case verticalKeyboardFacingRight:   return { 0, (f32) getHeight() - x - w, blackNoteLength, w };
            default:                            jassertfalse; break;
        }
    }
    else
    {
        switch (orientation)
        {
            case horizontalKeyboard:            return { x, 0, w, (f32) getHeight() };
            case verticalKeyboardFacingLeft:    return { 0, x, (f32) getWidth(), w };
            case verticalKeyboardFacingRight:   return { 0, (f32) getHeight() - x - w, (f32) getWidth(), w };
            default:                            jassertfalse; break;
        }
    }

    return {};
}

//==============================================================================
z0 KeyboardComponentBase::setOctaveForMiddleC (i32 octaveNum)
{
    octaveNumForMiddleC = octaveNum;
    repaint();
}

//==============================================================================
z0 KeyboardComponentBase::drawUpDownButton (Graphics& g, i32 w, i32 h, b8 mouseOver, b8 buttonDown, b8 movesOctavesUp)
{
    g.fillAll (findColor (upDownButtonBackgroundColorId));

    f32 angle = 0;

    switch (getOrientation())
    {
        case horizontalKeyboard:            angle = movesOctavesUp ? 0.0f  : 0.5f;  break;
        case verticalKeyboardFacingLeft:    angle = movesOctavesUp ? 0.25f : 0.75f; break;
        case verticalKeyboardFacingRight:   angle = movesOctavesUp ? 0.75f : 0.25f; break;
        default:                            jassertfalse; break;
    }

    Path path;
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (MathConstants<f32>::twoPi * angle, 0.5f, 0.5f));

    g.setColor (findColor (upDownButtonArrowColorId)
                  .withAlpha (buttonDown ? 1.0f : (mouseOver ? 0.6f : 0.4f)));

    g.fillPath (path, path.getTransformToScaleToFit (1.0f, 1.0f, (f32) w - 2.0f, (f32) h - 2.0f, true));
}

Range<f32> KeyboardComponentBase::getKeyPosition (i32 midiNoteNumber, f32 targetKeyWidth) const
{
    auto ratio = getBlackNoteWidthProportion();

    static const f32 notePos[] = { 0.0f, 1 - ratio * 0.6f,
                                     1.0f, 2 - ratio * 0.4f,
                                     2.0f,
                                     3.0f, 4 - ratio * 0.7f,
                                     4.0f, 5 - ratio * 0.5f,
                                     5.0f, 6 - ratio * 0.3f,
                                     6.0f };

    auto octave = midiNoteNumber / 12;
    auto note   = midiNoteNumber % 12;

    auto start = (f32) octave * 7.0f * targetKeyWidth + notePos[note] * targetKeyWidth;
    auto width = MidiMessage::isMidiNoteBlack (note) ? blackNoteWidthRatio * targetKeyWidth : targetKeyWidth;

    return { start, start + width };
}

//==============================================================================
z0 KeyboardComponentBase::paint (Graphics& g)
{
    drawKeyboardBackground (g, getLocalBounds().toFloat());

    for (i32 octaveBase = 0; octaveBase < 128; octaveBase += 12)
    {
        for (auto noteNum : whiteNotes)
        {
            const auto key = octaveBase + noteNum;

            if (rangeStart <= key && key <= rangeEnd)
                drawWhiteKey (key, g, getRectangleForKey (key));
        }

        for (auto noteNum : blackNotes)
        {
            const auto key = octaveBase + noteNum;

            if (rangeStart <= key && key <= rangeEnd)
                drawBlackKey (key, g, getRectangleForKey (key));
        }
    }
}

z0 KeyboardComponentBase::resized()
{
    auto w = getWidth();
    auto h = getHeight();

    if (w > 0 && h > 0)
    {
        if (orientation != horizontalKeyboard)
            std::swap (w, h);

        auto kx2 = getKeyPos (rangeEnd).getEnd();

        if ((i32) firstKey != rangeStart)
        {
            auto kx1 = getKeyPos (rangeStart).getStart();

            if (kx2 - kx1 <= (f32) w)
            {
                firstKey = (f32) rangeStart;
                sendChangeMessage();
                repaint();
            }
        }

        scrollDown->setVisible (canScroll && firstKey > (f32) rangeStart);

        xOffset = 0;

        if (canScroll)
        {
            auto scrollButtonW = jmin (scrollButtonWidth, w / 2);
            auto r = getLocalBounds();

            if (orientation == horizontalKeyboard)
            {
                scrollDown->setBounds (r.removeFromLeft  (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromRight (scrollButtonW));
            }
            else if (orientation == verticalKeyboardFacingLeft)
            {
                scrollDown->setBounds (r.removeFromTop    (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromBottom (scrollButtonW));
            }
            else
            {
                scrollDown->setBounds (r.removeFromBottom (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromTop    (scrollButtonW));
            }

            auto endOfLastKey = getKeyPos (rangeEnd).getEnd();

            auto spaceAvailable = w;
            auto lastStartKey = remappedXYToNote ({ endOfLastKey - (f32) spaceAvailable, 0 }).note + 1;

            if (lastStartKey >= 0 && ((i32) firstKey) > lastStartKey)
            {
                firstKey = (f32) jlimit (rangeStart, rangeEnd, lastStartKey);
                sendChangeMessage();
            }

            xOffset = getKeyPos ((i32) firstKey).getStart();
        }
        else
        {
            firstKey = (f32) rangeStart;
        }

        scrollUp->setVisible (canScroll && getKeyPos (rangeEnd).getStart() > (f32) w);
        repaint();
    }
}

//==============================================================================
z0 KeyboardComponentBase::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    auto amount = (orientation == horizontalKeyboard && ! approximatelyEqual (wheel.deltaX, 0.0f))
                       ? wheel.deltaX : (orientation == verticalKeyboardFacingLeft ? wheel.deltaY
                                                                                   : -wheel.deltaY);

    setLowestVisibleKeyFloat (firstKey - amount * keyWidth);
}

} // namespace drx
