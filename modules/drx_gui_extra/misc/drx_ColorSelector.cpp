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

struct ColorComponentSlider final : public Slider
{
    ColorComponentSlider (const Txt& name)  : Slider (name)
    {
        setRange (0.0, 255.0, 1.0);
    }

    Txt getTextFromValue (f64 value) override
    {
        return Txt::toHexString ((i32) value).toUpperCase().paddedLeft ('0', 2);
    }

    f64 getValueFromText (const Txt& text) override
    {
        return (f64) text.getHexValue32();
    }
};

//==============================================================================
class ColorSelector::ColorSpaceView final : public Component
{
public:
    ColorSpaceView (ColorSelector& cs, f32& hue, f32& sat, f32& val, i32 edgeSize)
        : owner (cs), h (hue), s (sat), v (val), edge (edgeSize)
    {
        addAndMakeVisible (marker);
        setMouseCursor (MouseCursor::CrosshairCursor);
    }

    z0 paint (Graphics& g) override
    {
        if (colours.isNull())
        {
            auto width = getWidth() / 2;
            auto height = getHeight() / 2;
            colours = Image (Image::RGB, width, height, false);

            Image::BitmapData pixels (colours, Image::BitmapData::writeOnly);

            for (i32 y = 0; y < height; ++y)
            {
                auto val = 1.0f - (f32) y / (f32) height;

                for (i32 x = 0; x < width; ++x)
                {
                    auto sat = (f32) x / (f32) width;
                    pixels.setPixelColor (x, y, Color (h, sat, val, 1.0f));
                }
            }
        }

        g.setOpacity (1.0f);
        g.drawImageTransformed (colours,
                                RectanglePlacement (RectanglePlacement::stretchToFit)
                                    .getTransformToFit (colours.getBounds().toFloat(),
                                                        getLocalBounds().reduced (edge).toFloat()),
                                false);
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        auto sat =        (f32) (e.x - edge) / (f32) (getWidth()  - edge * 2);
        auto val = 1.0f - (f32) (e.y - edge) / (f32) (getHeight() - edge * 2);

        owner.setSV (sat, val);
    }

    z0 updateIfNeeded()
    {
        if (! approximatelyEqual (lastHue, h))
        {
            lastHue = h;
            colours = {};
            repaint();
        }

        updateMarker();
    }

    z0 resized() override
    {
        colours = {};
        updateMarker();
    }

private:
    ColorSelector& owner;
    f32& h;
    f32& s;
    f32& v;
    f32 lastHue = 0;
    i32k edge;
    Image colours;

    struct ColorSpaceMarker final : public Component
    {
        ColorSpaceMarker()
        {
            setInterceptsMouseClicks (false, false);
        }

        z0 paint (Graphics& g) override
        {
            g.setColor (Color::greyLevel (0.1f));
            g.drawEllipse (1.0f, 1.0f, (f32) getWidth() - 2.0f, (f32) getHeight() - 2.0f, 1.0f);
            g.setColor (Color::greyLevel (0.9f));
            g.drawEllipse (2.0f, 2.0f, (f32) getWidth() - 4.0f, (f32) getHeight() - 4.0f, 1.0f);
        }
    };

    ColorSpaceMarker marker;

    z0 updateMarker()
    {
        auto markerSize = jmax (14, edge * 2);
        auto area = getLocalBounds().reduced (edge);

        marker.setBounds (Rectangle<i32> (markerSize, markerSize)
                            .withCentre (area.getRelativePoint (s, 1.0f - v)));
    }

    DRX_DECLARE_NON_COPYABLE (ColorSpaceView)
};

//==============================================================================
class ColorSelector::HueSelectorComp final : public Component
{
public:
    HueSelectorComp (ColorSelector& cs, f32& hue, i32 edgeSize)
        : owner (cs), h (hue), edge (edgeSize)
    {
        addAndMakeVisible (marker);
    }

    z0 paint (Graphics& g) override
    {
        ColorGradient cg;
        cg.isRadial = false;
        cg.point1.setXY (0.0f, (f32) edge);
        cg.point2.setXY (0.0f, (f32) getHeight());

        for (f32 i = 0.0f; i <= 1.0f; i += 0.02f)
            cg.addColor (i, Color (i, 1.0f, 1.0f, 1.0f));

        g.setGradientFill (cg);
        g.fillRect (getLocalBounds().reduced (edge));
    }

    z0 resized() override
    {
        auto markerSize = jmax (14, edge * 2);
        auto area = getLocalBounds().reduced (edge);

        marker.setBounds (Rectangle<i32> (getWidth(), markerSize)
                            .withCentre (area.getRelativePoint (0.5f, h)));
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        owner.setHue ((f32) (e.y - edge) / (f32) (getHeight() - edge * 2));
    }

    z0 updateIfNeeded()
    {
        resized();
    }

private:
    ColorSelector& owner;
    f32& h;
    i32k edge;

    struct HueSelectorMarker final : public Component
    {
        HueSelectorMarker()
        {
            setInterceptsMouseClicks (false, false);
        }

        z0 paint (Graphics& g) override
        {
            auto cw = (f32) getWidth();
            auto ch = (f32) getHeight();

            Path p;
            p.addTriangle (1.0f, 1.0f,
                           cw * 0.3f, ch * 0.5f,
                           1.0f, ch - 1.0f);

            p.addTriangle (cw - 1.0f, 1.0f,
                           cw * 0.7f, ch * 0.5f,
                           cw - 1.0f, ch - 1.0f);

            g.setColor (Colors::white.withAlpha (0.75f));
            g.fillPath (p);

            g.setColor (Colors::black.withAlpha (0.75f));
            g.strokePath (p, PathStrokeType (1.2f));
        }
    };

    HueSelectorMarker marker;

    DRX_DECLARE_NON_COPYABLE (HueSelectorComp)
};

//==============================================================================
class ColorSelector::SwatchComponent final : public Component
{
public:
    SwatchComponent (ColorSelector& cs, i32 itemIndex)
        : owner (cs), index (itemIndex)
    {
    }

    z0 paint (Graphics& g) override
    {
        auto col = owner.getSwatchColor (index);

        g.fillCheckerBoard (getLocalBounds().toFloat(), 6.0f, 6.0f,
                            Color (0xffdddddd).overlaidWith (col),
                            Color (0xffffffff).overlaidWith (col));
    }

    z0 mouseDown (const MouseEvent&) override
    {
        PopupMenu m;
        m.addItem (1, TRANS ("Use this swatch as the current colour"));
        m.addSeparator();
        m.addItem (2, TRANS ("Set this swatch to the current colour"));

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this),
                         ModalCallbackFunction::forComponent (menuStaticCallback, this));
    }

private:
    ColorSelector& owner;
    i32k index;

    static z0 menuStaticCallback (i32 result, SwatchComponent* comp)
    {
        if (comp != nullptr)
        {
            if (result == 1)  comp->setColorFromSwatch();
            if (result == 2)  comp->setSwatchFromColor();
        }
    }

    z0 setColorFromSwatch()
    {
        owner.setCurrentColor (owner.getSwatchColor (index));
    }

    z0 setSwatchFromColor()
    {
        if (owner.getSwatchColor (index) != owner.getCurrentColor())
        {
            owner.setSwatchColor (index, owner.getCurrentColor());
            repaint();
        }
    }

    DRX_DECLARE_NON_COPYABLE (SwatchComponent)
};

//==============================================================================
class ColorSelector::ColorPreviewComp final : public Component
{
public:
    ColorPreviewComp (ColorSelector& cs, b8 isEditable)
        : owner (cs)
    {
        colourLabel.setFont (labelFont);
        colourLabel.setJustificationType (Justification::centred);

        if (isEditable)
        {
            colourLabel.setEditable (true);

            colourLabel.onEditorShow = [this]
            {
                if (auto* ed = colourLabel.getCurrentTextEditor())
                    ed->setInputRestrictions ((owner.flags & showAlphaChannel) ? 8 : 6, "1234567890ABCDEFabcdef");
            };

            colourLabel.onEditorHide = [this]
            {
                updateColorIfNecessary (colourLabel.getText());
            };
        }

        addAndMakeVisible (colourLabel);
    }

    z0 updateIfNeeded()
    {
        auto newColor = owner.getCurrentColor();

        if (currentColor != newColor)
        {
            currentColor = newColor;
            auto textColor = (Colors::white.overlaidWith (currentColor).contrasting());

            colourLabel.setColor (Label::textColorId,            textColor);
            colourLabel.setColor (Label::textWhenEditingColorId, textColor);
            colourLabel.setText (currentColor.toDisplayString ((owner.flags & showAlphaChannel) != 0), dontSendNotification);

            labelWidth = GlyphArrangement::getStringWidthInt (labelFont, colourLabel.getText());

            repaint();
        }
    }

    z0 paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds().toFloat(), 10.0f, 10.0f,
                            Color (0xffdddddd).overlaidWith (currentColor),
                            Color (0xffffffff).overlaidWith (currentColor));
    }

    z0 resized() override
    {
        colourLabel.centreWithSize (labelWidth + 10, (i32) labelFont.getHeight() + 10);
    }

private:
    z0 updateColorIfNecessary (const Txt& newColorString)
    {
        auto newColor = Color::fromString (newColorString);

        if (newColor != currentColor)
            owner.setCurrentColor (newColor);
    }

    ColorSelector& owner;

    Color currentColor;
    Font labelFont { withDefaultMetrics (FontOptions { 14.0f, Font::bold }) };
    i32 labelWidth = 0;
    Label colourLabel;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorPreviewComp)
};

//==============================================================================
ColorSelector::ColorSelector (i32 sectionsToShow, i32 edge, i32 gapAroundColorSpaceComponent)
    : colour (Colors::white),
      flags (sectionsToShow),
      edgeGap (edge)
{
    // not much point having a selector with no components in it!
    jassert ((flags & (showColorAtTop | showSliders | showColorspace)) != 0);

    updateHSV();

    if ((flags & showColorAtTop) != 0)
    {
        previewComponent.reset (new ColorPreviewComp (*this, (flags & editableColor) != 0));
        addAndMakeVisible (previewComponent.get());
    }

    if ((flags & showSliders) != 0)
    {
        sliders[0].reset (new ColorComponentSlider (TRANS ("red")));
        sliders[1].reset (new ColorComponentSlider (TRANS ("green")));
        sliders[2].reset (new ColorComponentSlider (TRANS ("blue")));
        sliders[3].reset (new ColorComponentSlider (TRANS ("alpha")));

        addAndMakeVisible (sliders[0].get());
        addAndMakeVisible (sliders[1].get());
        addAndMakeVisible (sliders[2].get());
        addChildComponent (sliders[3].get());

        sliders[3]->setVisible ((flags & showAlphaChannel) != 0);

        for (auto& slider : sliders)
            slider->onValueChange = [this] { changeColor(); };
    }

    if ((flags & showColorspace) != 0)
    {
        colourSpace.reset (new ColorSpaceView (*this, h, s, v, gapAroundColorSpaceComponent));
        hueSelector.reset (new HueSelectorComp (*this, h, gapAroundColorSpaceComponent));

        addAndMakeVisible (colourSpace.get());
        addAndMakeVisible (hueSelector.get());
    }

    update (dontSendNotification);
}

ColorSelector::~ColorSelector()
{
    dispatchPendingMessages();
    swatchComponents.clear();
}

//==============================================================================
Color ColorSelector::getCurrentColor() const
{
    return ((flags & showAlphaChannel) != 0) ? colour : colour.withAlpha ((u8) 0xff);
}

z0 ColorSelector::setCurrentColor (Color c, NotificationType notification)
{
    if (c != colour)
    {
        colour = ((flags & showAlphaChannel) != 0) ? c : c.withAlpha ((u8) 0xff);

        updateHSV();
        update (notification);
    }
}

z0 ColorSelector::setHue (f32 newH)
{
    newH = jlimit (0.0f, 1.0f, newH);

    if (! approximatelyEqual (h, newH))
    {
        h = newH;
        colour = Color (h, s, v, colour.getFloatAlpha());
        update (sendNotification);
    }
}

z0 ColorSelector::setSV (f32 newS, f32 newV)
{
    newS = jlimit (0.0f, 1.0f, newS);
    newV = jlimit (0.0f, 1.0f, newV);

    if (! approximatelyEqual (s, newS) || ! approximatelyEqual (v, newV))
    {
        s = newS;
        v = newV;
        colour = Color (h, s, v, colour.getFloatAlpha());
        update (sendNotification);
    }
}

//==============================================================================
z0 ColorSelector::updateHSV()
{
    colour.getHSB (h, s, v);
}

z0 ColorSelector::update (NotificationType notification)
{
    if (sliders[0] != nullptr)
    {
        sliders[0]->setValue ((i32) colour.getRed(),   notification);
        sliders[1]->setValue ((i32) colour.getGreen(), notification);
        sliders[2]->setValue ((i32) colour.getBlue(),  notification);
        sliders[3]->setValue ((i32) colour.getAlpha(), notification);
    }

    if (colourSpace != nullptr)
    {
        colourSpace->updateIfNeeded();
        hueSelector->updateIfNeeded();
    }

    if (previewComponent != nullptr)
        previewComponent->updateIfNeeded();

    if (notification != dontSendNotification)
        sendChangeMessage();

    if (notification == sendNotificationSync)
        dispatchPendingMessages();
}

//==============================================================================
z0 ColorSelector::paint (Graphics& g)
{
    g.fillAll (findColor (backgroundColorId));

    if ((flags & showSliders) != 0)
    {
        g.setColor (findColor (labelTextColorId));
        g.setFont (11.0f);

        for (auto& slider : sliders)
        {
            if (slider->isVisible())
                g.drawText (slider->getName() + ":",
                            0, slider->getY(),
                            slider->getX() - 8, slider->getHeight(),
                            Justification::centredRight, false);
        }
    }
}

z0 ColorSelector::resized()
{
    i32k swatchesPerRow = 8;
    i32k swatchHeight = 22;

    i32k numSliders = ((flags & showAlphaChannel) != 0) ? 4 : 3;
    i32k numSwatches = getNumSwatches();

    i32k swatchSpace = numSwatches > 0 ? edgeGap + swatchHeight * ((numSwatches + 7) / swatchesPerRow) : 0;
    i32k sliderSpace = ((flags & showSliders) != 0)  ? jmin (22 * numSliders + edgeGap, proportionOfHeight (0.3f)) : 0;
    i32k topSpace = ((flags & showColorAtTop) != 0) ? jmin (30 + edgeGap * 2, proportionOfHeight (0.2f)) : edgeGap;

    if (previewComponent != nullptr)
        previewComponent->setBounds (edgeGap, edgeGap, getWidth() - edgeGap * 2, topSpace - edgeGap * 2);

    i32 y = topSpace;

    if ((flags & showColorspace) != 0)
    {
        i32k hueWidth = jmin (50, proportionOfWidth (0.15f));

        colourSpace->setBounds (edgeGap, y,
                                getWidth() - hueWidth - edgeGap - 4,
                                getHeight() - topSpace - sliderSpace - swatchSpace - edgeGap);

        hueSelector->setBounds (colourSpace->getRight() + 4, y,
                                getWidth() - edgeGap - (colourSpace->getRight() + 4),
                                colourSpace->getHeight());

        y = getHeight() - sliderSpace - swatchSpace - edgeGap;
    }

    if ((flags & showSliders) != 0)
    {
        auto sliderHeight = jmax (4, sliderSpace / numSliders);

        for (i32 i = 0; i < numSliders; ++i)
        {
            sliders[i]->setBounds (proportionOfWidth (0.2f), y,
                                   proportionOfWidth (0.72f), sliderHeight - 2);

            y += sliderHeight;
        }
    }

    if (numSwatches > 0)
    {
        i32k startX = 8;
        i32k xGap = 4;
        i32k yGap = 4;
        i32k swatchWidth = (getWidth() - startX * 2) / swatchesPerRow;
        y += edgeGap;

        if (swatchComponents.size() != numSwatches)
        {
            swatchComponents.clear();

            for (i32 i = 0; i < numSwatches; ++i)
            {
                auto* sc = new SwatchComponent (*this, i);
                swatchComponents.add (sc);
                addAndMakeVisible (sc);
            }
        }

        i32 x = startX;

        for (i32 i = 0; i < swatchComponents.size(); ++i)
        {
            auto* sc = swatchComponents.getUnchecked (i);

            sc->setBounds (x + xGap / 2,
                           y + yGap / 2,
                           swatchWidth - xGap,
                           swatchHeight - yGap);

            if (((i + 1) % swatchesPerRow) == 0)
            {
                x = startX;
                y += swatchHeight;
            }
            else
            {
                x += swatchWidth;
            }
        }
    }
}

z0 ColorSelector::changeColor()
{
    if (sliders[0] != nullptr)
        setCurrentColor (Color ((u8) sliders[0]->getValue(),
                                  (u8) sliders[1]->getValue(),
                                  (u8) sliders[2]->getValue(),
                                  (u8) sliders[3]->getValue()));
}

//==============================================================================
i32 ColorSelector::getNumSwatches() const
{
    return 0;
}

Color ColorSelector::getSwatchColor (i32) const
{
    jassertfalse; // if you've overridden getNumSwatches(), you also need to implement this method
    return Colors::black;
}

z0 ColorSelector::setSwatchColor (i32, const Color&)
{
    jassertfalse; // if you've overridden getNumSwatches(), you also need to implement this method
}

} // namespace drx
