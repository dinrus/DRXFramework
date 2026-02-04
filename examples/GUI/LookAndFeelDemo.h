/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             LookAndFeelDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases custom look and feel components.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        LookAndFeelDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Custom Look And Feel subclasss.

    Simply override the methods you need to, anything else will be inherited from the base class.
    It's a good idea not to hard code your colours, use the findColor method along with appropriate
    ColorIds so you can set these on a per-component basis.
*/
struct CustomLookAndFeel : public LookAndFeel_V4
{
    z0 drawRoundThumb (Graphics& g, f32 x, f32 y, f32 diameter, Color colour, f32 outlineThickness)
    {
        auto halfThickness = outlineThickness * 0.5f;

        Path p;
        p.addEllipse (x + halfThickness,
                      y + halfThickness,
                      diameter - outlineThickness,
                      diameter - outlineThickness);

        DropShadow (Colors::black, 1, {}).drawForPath (g, p);

        g.setColor (colour);
        g.fillPath (p);

        g.setColor (colour.brighter());
        g.strokePath (p, PathStrokeType (outlineThickness));
    }

    z0 drawButtonBackground (Graphics& g, Button& button, const Color& backgroundColor,
                               b8 isMouseOverButton, b8 isButtonDown) override
    {
        auto baseColor = backgroundColor.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                          .withMultipliedAlpha      (button.isEnabled() ? 0.9f : 0.5f);

        if (isButtonDown || isMouseOverButton)
            baseColor = baseColor.contrasting (isButtonDown ? 0.2f : 0.1f);

        auto flatOnLeft   = button.isConnectedOnLeft();
        auto flatOnRight  = button.isConnectedOnRight();
        auto flatOnTop    = button.isConnectedOnTop();
        auto flatOnBottom = button.isConnectedOnBottom();

        auto width  = (f32) button.getWidth()  - 1.0f;
        auto height = (f32) button.getHeight() - 1.0f;

        if (width > 0 && height > 0)
        {
            auto cornerSize = jmin (15.0f, jmin (width, height) * 0.45f);
            auto lineThickness = cornerSize    * 0.1f;
            auto halfThickness = lineThickness * 0.5f;

            Path outline;
            outline.addRoundedRectangle (0.5f + halfThickness, 0.5f + halfThickness, width - lineThickness, height - lineThickness,
                                         cornerSize, cornerSize,
                                         ! (flatOnLeft  || flatOnTop),
                                         ! (flatOnRight || flatOnTop),
                                         ! (flatOnLeft  || flatOnBottom),
                                         ! (flatOnRight || flatOnBottom));

            auto outlineColor = button.findColor (button.getToggleState() ? TextButton::textColorOnId
                                                                            : TextButton::textColorOffId);

            g.setColor (baseColor);
            g.fillPath (outline);

            if (! button.getToggleState())
            {
                g.setColor (outlineColor);
                g.strokePath (outline, PathStrokeType (lineThickness));
            }
        }
    }

    z0 drawTickBox (Graphics& g, Component& component,
                      f32 x, f32 y, f32 w, f32 h,
                      b8 ticked,
                      b8 isEnabled,
                      b8 isMouseOverButton,
                      b8 isButtonDown) override
    {
        auto boxSize = w * 0.7f;

        auto isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());

        auto colour = component.findColor (TextButton::buttonColorId)
                               .withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f);

        drawRoundThumb (g, x, y + (h - boxSize) * 0.5f, boxSize, colour,
                        isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

        if (ticked)
        {
            g.setColor (isEnabled ? findColor (TextButton::buttonOnColorId) : Colors::grey);

            auto scale = 9.0f;
            auto trans = AffineTransform::scale (w / scale, h / scale).translated (x - 2.5f, y + 1.0f);

            g.fillPath (LookAndFeel_V4::getTickShape (6.0f), trans);
        }
    }

    z0 drawLinearSliderThumb (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                Slider::SliderStyle style, Slider& slider) override
    {
        auto sliderRadius = (f32) (getSliderThumbRadius (slider) - 2);

        auto isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());

        auto knobColor = slider.findColor (Slider::thumbColorId)
                                .withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                                .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            f32 kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = (f32) x + (f32) width * 0.5f;
                ky = sliderPos;
            }
            else
            {
                kx = sliderPos;
                ky = (f32) y + (f32) height * 0.5f;
            }

            auto outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

            drawRoundThumb (g,
                            kx - sliderRadius,
                            ky - sliderRadius,
                            sliderRadius * 2.0f,
                            knobColor, outlineThickness);
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    z0 drawLinearSlider (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                           Slider::SliderStyle style, Slider& slider) override
    {
        g.fillAll (slider.findColor (Slider::backgroundColorId));

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            Path p;

            if (style == Slider::LinearBarVertical)
                p.addRectangle ((f32) x, sliderPos, (f32) width, 1.0f + (f32) height - sliderPos);
            else
                p.addRectangle ((f32) x, (f32) y, sliderPos - (f32) x, (f32) height);

            auto baseColor = slider.findColor (Slider::rotarySliderFillColorId)
                                    .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                    .withMultipliedAlpha (0.8f);

            g.setColor (baseColor);
            g.fillPath (p);

            auto lineThickness = jmin (15.0f, (f32) jmin (width, height) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        }
        else
        {
            drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb      (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    z0 drawLinearSliderBackground (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                     f32 /*sliderPos*/,
                                     f32 /*minSliderPos*/,
                                     f32 /*maxSliderPos*/,
                                     const Slider::SliderStyle /*style*/, Slider& slider) override
    {
        auto sliderRadius = (f32) getSliderThumbRadius (slider) - 5.0f;
        Path on, off;

        if (slider.isHorizontal())
        {
            auto iy = (f32) y + (f32) height * 0.5f - sliderRadius * 0.5f;
            Rectangle<f32> r ((f32) x - sliderRadius * 0.5f, iy, (f32) width + sliderRadius, sliderRadius);
            auto onW = r.getWidth() * ((f32) slider.valueToProportionOfLength (slider.getValue()));

            on.addRectangle (r.removeFromLeft (onW));
            off.addRectangle (r);
        }
        else
        {
            auto ix = (f32) x + (f32) width * 0.5f - sliderRadius * 0.5f;
            Rectangle<f32> r (ix, (f32) y - sliderRadius * 0.5f, sliderRadius, (f32) height + sliderRadius);
            auto onH = r.getHeight() * ((f32) slider.valueToProportionOfLength (slider.getValue()));

            on.addRectangle (r.removeFromBottom (onH));
            off.addRectangle (r);
        }

        g.setColor (slider.findColor (Slider::rotarySliderFillColorId));
        g.fillPath (on);

        g.setColor (slider.findColor (Slider::trackColorId));
        g.fillPath (off);
    }

    z0 drawRotarySlider (Graphics& g, i32 x, i32 y, i32 width, i32 height, f32 sliderPos,
                           f32 rotaryStartAngle, f32 rotaryEndAngle, Slider& slider) override
    {
        auto radius = (f32) jmin (width / 2, height / 2) - 2.0f;
        auto centreX = (f32) x + (f32) width  * 0.5f;
        auto centreY = (f32) y + (f32) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        if (slider.isEnabled())
            g.setColor (slider.findColor (Slider::rotarySliderFillColorId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColor (Color (0x80808080));

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, 0.0);
            g.fillPath (filledArc);
        }

        {
            auto lineThickness = jmin (15.0f, (f32) jmin (width, height) * 0.45f) * 0.1f;
            Path outlineArc;
            outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.0);
            g.strokePath (outlineArc, PathStrokeType (lineThickness));
        }
    }
};

//==============================================================================
/** Another really simple look and feel that is very flat and square.

    This inherits from CustomLookAndFeel above for the linear bar and slider backgrounds.
*/
struct SquareLookAndFeel final : public CustomLookAndFeel
{
    z0 drawButtonBackground (Graphics& g, Button& button, const Color& backgroundColor,
                               b8 isMouseOverButton, b8 isButtonDown) override
    {
        auto baseColor = backgroundColor.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                          .withMultipliedAlpha      (button.isEnabled() ? 0.9f : 0.5f);

        if (isButtonDown || isMouseOverButton)
            baseColor = baseColor.contrasting (isButtonDown ? 0.2f : 0.1f);

        auto width  = (f32) button.getWidth()  - 1.0f;
        auto height = (f32) button.getHeight() - 1.0f;

        if (width > 0 && height > 0)
        {
            g.setGradientFill (ColorGradient::vertical (baseColor, 0.0f,
                                                         baseColor.darker (0.1f), height));

            g.fillRect (button.getLocalBounds());
        }
    }

    z0 drawTickBox (Graphics& g, Component& component,
                      f32 x, f32 y, f32 w, f32 h,
                      b8 ticked,
                      b8 isEnabled,
                      b8 /*isMouseOverButton*/,
                      b8 /*isButtonDown*/) override
    {
        auto boxSize = w * 0.7f;

        auto isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());

        auto colour = component.findColor (TextButton::buttonOnColorId)
                               .withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f);

        g.setColor (colour);

        Rectangle<f32> r (x, y + (h - boxSize) * 0.5f, boxSize, boxSize);
        g.fillRect (r);

        if (ticked)
        {
            auto tickPath = LookAndFeel_V4::getTickShape (6.0f);
            g.setColor (isEnabled ? findColor (TextButton::buttonColorId) : Colors::grey);

            auto transform = RectanglePlacement (RectanglePlacement::centred)
                               .getTransformToFit (tickPath.getBounds(),
                                                   r.reduced (r.getHeight() * 0.05f));

            g.fillPath (tickPath, transform);
        }
    }

    z0 drawLinearSliderThumb (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                Slider::SliderStyle style, Slider& slider) override
    {
        auto sliderRadius = (f32) getSliderThumbRadius (slider);

        b8 isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());

        auto knobColor = slider.findColor (Slider::rotarySliderFillColorId)
                                .withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                                .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);

        g.setColor (knobColor);

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            f32 kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = (f32) x + (f32) width * 0.5f;
                ky = sliderPos;
                g.fillRect (Rectangle<f32> (kx - sliderRadius, ky - 2.5f, sliderRadius * 2.0f, 5.0f));
            }
            else
            {
                kx = sliderPos;
                ky = (f32) y + (f32) height * 0.5f;
                g.fillRect (Rectangle<f32> (kx - 2.5f, ky - sliderRadius, 5.0f, sliderRadius * 2.0f));
            }
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    z0 drawRotarySlider (Graphics& g, i32 x, i32 y, i32 width, i32 height, f32 sliderPos,
                           f32 rotaryStartAngle, f32 rotaryEndAngle, Slider& slider) override
    {
        auto diameter = (f32) jmin (width, height) - 4.0f;
        auto radius = (diameter / 2.0f) * std::cos (MathConstants<f32>::pi / 4.0f);
        auto centreX = (f32) x + (f32) width  * 0.5f;
        auto centreY = (f32) y + (f32) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        b8 isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        auto baseColor = slider.isEnabled() ? slider.findColor (Slider::rotarySliderFillColorId).withAlpha (isMouseOver ? 0.8f : 1.0f)
                                             : Color (0x80808080);

        Rectangle<f32> r (rx, ry, rw, rw);
        auto transform = AffineTransform::rotation (angle, r.getCentreX(), r.getCentreY());

        auto x1 = r.getTopLeft()   .getX();
        auto y1 = r.getTopLeft()   .getY();
        auto x2 = r.getBottomLeft().getX();
        auto y2 = r.getBottomLeft().getY();

        transform.transformPoints (x1, y1, x2, y2);

        g.setGradientFill (ColorGradient (baseColor, x1, y1,
                                           baseColor.darker (0.1f), x2, y2,
                                           false));

        Path knob;
        knob.addRectangle (r);
        g.fillPath (knob, transform);

        Path needle;
        auto r2 = r * 0.1f;
        needle.addRectangle (r2.withPosition ({ r.getCentreX() - (r2.getWidth() / 2.0f), r.getY() }));

        g.setColor (slider.findColor (Slider::rotarySliderOutlineColorId));
        g.fillPath (needle, AffineTransform::rotation (angle, r.getCentreX(), r.getCentreY()));
    }
};

//==============================================================================
struct LookAndFeelDemoComponent final : public Component
{
    LookAndFeelDemoComponent()
    {
        addAndMakeVisible (rotarySlider);
        rotarySlider.setValue (2.5);

        addAndMakeVisible (verticalSlider);
        verticalSlider.setValue (6.2);

        addAndMakeVisible (barSlider);
        barSlider.setValue (4.5);

        addAndMakeVisible (incDecSlider);
        incDecSlider.setRange (0.0, 10.0, 1.0);
        incDecSlider.setIncDecButtonsMode (Slider::incDecButtonsDraggable_Horizontal);

        addAndMakeVisible (button1);

        addAndMakeVisible (button2);
        button2.setClickingTogglesState (true);
        button2.setToggleState (true, dontSendNotification);

        addAndMakeVisible (button3);

        addAndMakeVisible (button4);
        button4.setToggleState (true, dontSendNotification);

        for (i32 i = 0; i < 3; ++i)
        {
            auto* b = radioButtons.add (new TextButton ("Button " + Txt (i + 1)));

            addAndMakeVisible (b);
            b->setRadioGroupId (42);
            b->setClickingTogglesState (true);

            switch (i)
            {
                case 0:     b->setConnectedEdges (Button::ConnectedOnRight);                            break;
                case 1:     b->setConnectedEdges (Button::ConnectedOnRight + Button::ConnectedOnLeft);  break;
                case 2:     b->setConnectedEdges (Button::ConnectedOnLeft);                             break;
                default:    break;
            }
        }

        radioButtons.getUnchecked (2)->setToggleState (true, dontSendNotification);
    }

    z0 resized() override
    {
        auto area = getLocalBounds().reduced (10);
        auto row = area.removeFromTop (100);

        rotarySlider  .setBounds (row.removeFromLeft (100).reduced (5));
        verticalSlider.setBounds (row.removeFromLeft (100).reduced (5));
        barSlider     .setBounds (row.removeFromLeft (100).reduced (5, 25));
        incDecSlider  .setBounds (row.removeFromLeft (100).reduced (5, 28));

        row = area.removeFromTop (100);
        button1.setBounds (row.removeFromLeft (100).reduced (5));

        auto row2 = row.removeFromTop (row.getHeight() / 2).reduced (0, 10);
        button2.setBounds (row2.removeFromLeft (100).reduced (5, 0));
        button3.setBounds (row2.removeFromLeft (100).reduced (5, 0));
        button4.setBounds (row2.removeFromLeft (100).reduced (5, 0));

        row2 = (row.removeFromTop (row2.getHeight() + 20).reduced (5, 10));

        for (auto* b : radioButtons)
            b->setBounds (row2.removeFromLeft (100));
    }

    Slider rotarySlider    { Slider::RotaryHorizontalVerticalDrag, Slider::NoTextBox},
           verticalSlider  { Slider::LinearVertical, Slider::NoTextBox },
           barSlider       { Slider::LinearBar, Slider::NoTextBox },
           incDecSlider    { Slider::IncDecButtons, Slider::TextBoxBelow };

    TextButton button1  { "Hello World!" },
               button2  { "Hello World!" },
               button3  { "Hello World!" };

    ToggleButton button4 { "Toggle Me" };

    OwnedArray<TextButton> radioButtons;
};

//==============================================================================
class LookAndFeelDemo final : public Component
{
public:
    LookAndFeelDemo()
    {
        descriptionLabel.setMinimumHorizontalScale (1.0f);
        descriptionLabel.setText ("This demonstrates how to create a custom look and feel by overriding only the desired methods.\n\n"
                                  "Components can have their look and feel individually assigned or they will inherit it from their parent. "
                                  "Colors work in a similar way, they can be set for individual components or a look and feel as a whole.",
                                  dontSendNotification);

        addAndMakeVisible (descriptionLabel);
        addAndMakeVisible (lafBox);
        addAndMakeVisible (demoComp);

        addLookAndFeel (new LookAndFeel_V1(), "LookAndFeel_V1");
        addLookAndFeel (new LookAndFeel_V2(), "LookAndFeel_V2");
        addLookAndFeel (new LookAndFeel_V3(), "LookAndFeel_V3");
        addLookAndFeel (new LookAndFeel_V4(), "LookAndFeel_V4 (Dark)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getMidnightColorScheme()), "LookAndFeel_V4 (Midnight)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getGreyColorScheme()),     "LookAndFeel_V4 (Grey)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getLightColorScheme()),    "LookAndFeel_V4 (Light)");

        auto* claf = new CustomLookAndFeel();
        addLookAndFeel (claf, "Custom Look And Feel");
        setupCustomLookAndFeelColors (*claf);

        auto* slaf = new SquareLookAndFeel();
        addLookAndFeel (slaf, "Square Look And Feel");
        setupSquareLookAndFeelColors (*slaf);

        lafBox.onChange = [this] { setAllLookAndFeels (lookAndFeels[lafBox.getSelectedItemIndex()]); };
        lafBox.setSelectedItemIndex (3);

        addAndMakeVisible (randomButton);
        randomButton.onClick = [this] { lafBox.setSelectedItemIndex (Random().nextInt (lafBox.getNumItems())); };

        setSize (500, 500);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Color::greyLevel (0.4f)));
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (10);

        descriptionLabel.setBounds (r.removeFromTop (150));
        lafBox          .setBounds (r.removeFromTop (22).removeFromLeft (250));
        randomButton    .setBounds (lafBox.getBounds().withX (lafBox.getRight() + 20).withWidth (140));
        demoComp        .setBounds (r.withTrimmedTop (10));
    }

private:
    Label descriptionLabel;
    ComboBox lafBox;
    TextButton randomButton  { "Assign Randomly" };
    OwnedArray<LookAndFeel> lookAndFeels;
    LookAndFeelDemoComponent demoComp;

    z0 addLookAndFeel (LookAndFeel* laf, const Txt& name)
    {
        lookAndFeels.add (laf);
        lafBox.addItem (name, lafBox.getNumItems() + 1);
    }

    z0 setupCustomLookAndFeelColors (LookAndFeel& laf)
    {
        laf.setColor (Slider::thumbColorId,               Color::greyLevel (0.95f));
        laf.setColor (Slider::textBoxOutlineColorId,      Colors::transparentWhite);
        laf.setColor (Slider::rotarySliderFillColorId,    Color (0xff00b5f6));
        laf.setColor (Slider::rotarySliderOutlineColorId, Colors::white);

        laf.setColor (TextButton::buttonColorId,  Colors::white);
        laf.setColor (TextButton::textColorOffId, Color (0xff00b5f6));

        laf.setColor (TextButton::buttonOnColorId, laf.findColor (TextButton::textColorOffId));
        laf.setColor (TextButton::textColorOnId,   laf.findColor (TextButton::buttonColorId));
    }

    z0 setupSquareLookAndFeelColors (LookAndFeel& laf)
    {
        auto baseColor = Colors::red;

        laf.setColor (Slider::thumbColorId,               Color::greyLevel (0.95f));
        laf.setColor (Slider::textBoxOutlineColorId,      Colors::transparentWhite);
        laf.setColor (Slider::rotarySliderFillColorId,    baseColor);
        laf.setColor (Slider::rotarySliderOutlineColorId, Colors::white);
        laf.setColor (Slider::trackColorId,               Colors::black);

        laf.setColor (TextButton::buttonColorId,  Colors::white);
        laf.setColor (TextButton::textColorOffId, baseColor);

        laf.setColor (TextButton::buttonOnColorId, laf.findColor (TextButton::textColorOffId));
        laf.setColor (TextButton::textColorOnId,   laf.findColor (TextButton::buttonColorId));
    }

    z0 setAllLookAndFeels (LookAndFeel* laf)
    {
        for (auto* child : demoComp.getChildren())
            child->setLookAndFeel (laf);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeelDemo)
};
