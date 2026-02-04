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

DrawableButton::DrawableButton (const Txt& name, const DrawableButton::ButtonStyle buttonStyle)
    : Button (name), style (buttonStyle)
{
}

DrawableButton::~DrawableButton()
{
}

//==============================================================================
static std::unique_ptr<Drawable> copyDrawableIfNotNull (const Drawable* const d)
{
    if (d != nullptr)
        return d->createCopy();

    return {};
}

z0 DrawableButton::setImages (const Drawable* normal,
                                const Drawable* over,
                                const Drawable* down,
                                const Drawable* disabled,
                                const Drawable* normalOn,
                                const Drawable* overOn,
                                const Drawable* downOn,
                                const Drawable* disabledOn)
{
    jassert (normal != nullptr); // you really need to give it at least a normal image..

    normalImage     = copyDrawableIfNotNull (normal);
    overImage       = copyDrawableIfNotNull (over);
    downImage       = copyDrawableIfNotNull (down);
    disabledImage   = copyDrawableIfNotNull (disabled);
    normalImageOn   = copyDrawableIfNotNull (normalOn);
    overImageOn     = copyDrawableIfNotNull (overOn);
    downImageOn     = copyDrawableIfNotNull (downOn);
    disabledImageOn = copyDrawableIfNotNull (disabledOn);

    currentImage = nullptr;

    buttonStateChanged();
}

//==============================================================================
z0 DrawableButton::setButtonStyle (const DrawableButton::ButtonStyle newStyle)
{
    if (style != newStyle)
    {
        style = newStyle;
        buttonStateChanged();
    }
}

z0 DrawableButton::setEdgeIndent (i32k numPixelsIndent)
{
    edgeIndent = numPixelsIndent;
    repaint();
    resized();
}

Rectangle<f32> DrawableButton::getImageBounds() const
{
    auto r = getLocalBounds();

    if (style != ImageStretched)
    {
        auto indentX = jmin (edgeIndent, proportionOfWidth  (0.3f));
        auto indentY = jmin (edgeIndent, proportionOfHeight (0.3f));

        if (shouldDrawButtonBackground())
        {
            indentX = jmax (getWidth()  / 4, indentX);
            indentY = jmax (getHeight() / 4, indentY);
        }
        else if (style == ImageAboveTextLabel)
        {
            r = r.withTrimmedBottom (jmin (16, proportionOfHeight (0.25f)));
        }

        r = r.reduced (indentX, indentY);
    }

    return r.toFloat();
}

z0 DrawableButton::resized()
{
    Button::resized();

    if (currentImage != nullptr)
    {
        if (style != ImageRaw)
        {
            i32 transformFlags = 0;

            if (style == ImageStretched)
            {
                transformFlags |= RectanglePlacement::stretchToFit;
            }
            else
            {
                transformFlags |= RectanglePlacement::centred;

                if (style == ImageOnButtonBackgroundOriginalSize)
                    transformFlags |= RectanglePlacement::doNotResize;
            }

            currentImage->setTransformToFit (getImageBounds(), transformFlags);
        }
    }
}

z0 DrawableButton::buttonStateChanged()
{
    repaint();

    Drawable* imageToDraw = nullptr;
    f32 opacity = 1.0f;

    if (isEnabled())
    {
        imageToDraw = getCurrentImage();
    }
    else
    {
        imageToDraw = getToggleState() ? disabledImageOn.get()
                                       : disabledImage.get();

        if (imageToDraw == nullptr)
        {
            opacity = 0.4f;
            imageToDraw = getNormalImage();
        }
    }

    if (imageToDraw != currentImage)
    {
        removeChildComponent (currentImage);
        currentImage = imageToDraw;

        if (currentImage != nullptr)
        {
            currentImage->setInterceptsMouseClicks (false, false);
            addAndMakeVisible (currentImage);
            resized();
        }
    }

    if (currentImage != nullptr)
        currentImage->setAlpha (opacity);
}

z0 DrawableButton::enablementChanged()
{
    Button::enablementChanged();
    buttonStateChanged();
}

z0 DrawableButton::colourChanged()
{
    repaint();
}

z0 DrawableButton::paintButton (Graphics& g,
                                  const b8 shouldDrawButtonAsHighlighted,
                                  const b8 shouldDrawButtonAsDown)
{
    auto& lf = getLookAndFeel();

    if (shouldDrawButtonBackground())
        lf.drawButtonBackground (g, *this,
                                 findColor (getToggleState() ? TextButton::buttonOnColorId
                                                              : TextButton::buttonColorId),
                                 shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    else
        lf.drawDrawableButton (g, *this, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

//==============================================================================
Drawable* DrawableButton::getCurrentImage() const noexcept
{
    if (isDown())  return getDownImage();
    if (isOver())  return getOverImage();

    return getNormalImage();
}

Drawable* DrawableButton::getNormalImage() const noexcept
{
    return (getToggleState() && normalImageOn != nullptr) ? normalImageOn.get()
                                                          : normalImage.get();
}

Drawable* DrawableButton::getOverImage() const noexcept
{
    if (getToggleState())
    {
        if (overImageOn   != nullptr)   return overImageOn.get();
        if (normalImageOn != nullptr)   return normalImageOn.get();
    }

    return overImage != nullptr ? overImage.get() : normalImage.get();
}

Drawable* DrawableButton::getDownImage() const noexcept
{
    if (auto* d = getToggleState() ? downImageOn.get() : downImage.get())
        return d;

    return getOverImage();
}

} // namespace drx
