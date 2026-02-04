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

 name:             GraphicsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases various graphics features.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        GraphicsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Holds the various toggle buttons for the animation modes. */
class ControllersComponent final : public Component
{
public:
    ControllersComponent()
    {
        setOpaque (true);

        initialiseToggle (animatePosition, "Animate Position",  true);
        initialiseToggle (animateRotation, "Animate Rotation",  true);
        initialiseToggle (animateSize,     "Animate Size",      false);
        initialiseToggle (animateShear,    "Animate Shearing",  false);
        initialiseToggle (animateAlpha,    "Animate Alpha",     false);
        initialiseToggle (clipToRectangle, "Clip to Rectangle", false);
        initialiseToggle (clipToPath,      "Clip to Path",      false);
        initialiseToggle (clipToImage,     "Clip to Image",     false);
        initialiseToggle (quality,         "Higher quality image interpolation", false);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (4);

        i32 buttonHeight = 22;

        auto columns = r.removeFromTop (buttonHeight * 4);
        auto col = columns.removeFromLeft (200);

        animatePosition.setBounds (col.removeFromTop (buttonHeight));
        animateRotation.setBounds (col.removeFromTop (buttonHeight));
        animateSize    .setBounds (col.removeFromTop (buttonHeight));
        animateShear   .setBounds (col.removeFromTop (buttonHeight));

        columns.removeFromLeft (20);
        col = columns.removeFromLeft (200);

        animateAlpha   .setBounds (col.removeFromTop (buttonHeight));
        clipToRectangle.setBounds (col.removeFromTop (buttonHeight));
        clipToPath     .setBounds (col.removeFromTop (buttonHeight));
        clipToImage    .setBounds (col.removeFromTop (buttonHeight));

        r.removeFromBottom (6);
        quality.setBounds (r.removeFromTop (buttonHeight));
    }

    z0 initialiseToggle (ToggleButton& b, tukk name, b8 on)
    {
        addAndMakeVisible (b);
        b.setButtonText (name);
        b.setToggleState (on, dontSendNotification);
    }

    ToggleButton animateRotation, animatePosition, animateAlpha, animateSize, animateShear;
    ToggleButton clipToRectangle, clipToPath, clipToImage, quality;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControllersComponent)
};

//==============================================================================
class GraphicsDemoBase : public Component
{
public:
    GraphicsDemoBase (ControllersComponent& cc, const Txt& name)
        : Component (name),
          controls (cc)
    {
        displayFont = FontOptions (Font::getDefaultMonospacedFontName(), 12.0f, Font::bold);
    }

    AffineTransform getTransform()
    {
        auto hw = 0.5f * (f32) getWidth();
        auto hh = 0.5f * (f32) getHeight();

        AffineTransform t;

        if (controls.animateRotation.getToggleState())
            t = t.rotated (rotation.getValue() * MathConstants<f32>::twoPi);

        if (controls.animateSize.getToggleState())
            t = t.scaled (0.3f + size.getValue() * 2.0f);

        if (controls.animatePosition.getToggleState())
            t = t.translated (hw + hw * (offsetX.getValue() - 0.5f),
                              hh + hh * (offsetY.getValue() - 0.5f));
        else
            t = t.translated (hw, hh);

        if (controls.animateShear.getToggleState())
            t = t.sheared (shear.getValue() * 2.0f - 1.0f, 0.0f);

        return t;
    }

    f32 getAlpha() const
    {
        if (controls.animateAlpha.getToggleState())
            return alpha.getValue();

        return 1.0f;
    }

    z0 paint (Graphics& g) override
    {
        auto startTime = 0.0;

        {
            // A ScopedSaveState will return the Graphics context to the state it was at the time of
            // construction when it goes out of scope. We use it here to avoid clipping the fps text
            const Graphics::ScopedSaveState state (g);

            if (controls.clipToRectangle.getToggleState())  clipToRectangle (g);
            if (controls.clipToPath     .getToggleState())  clipToPath (g);
            if (controls.clipToImage    .getToggleState())  clipToImage (g);

            g.setImageResamplingQuality (controls.quality.getToggleState() ? Graphics::highResamplingQuality
                                                                           : Graphics::mediumResamplingQuality);

            // take a note of the time before the render
            startTime = Time::getMillisecondCounterHiRes();

            // then let the demo draw itself..
            drawDemo (g);
        }

        auto now = Time::getMillisecondCounterHiRes();
        auto filtering = 0.08;

        auto elapsedMs = now - startTime;
        averageTimeMs += (elapsedMs - averageTimeMs) * filtering;

        auto sinceLastRender = now - lastRenderStartTime;
        lastRenderStartTime = now;

        auto effectiveFPS = 1000.0 / averageTimeMs;
        auto actualFPS = sinceLastRender > 0 ? (1000.0 / sinceLastRender) : 0;
        averageActualFPS += (actualFPS - averageActualFPS) * filtering;

        GlyphArrangement ga;
        ga.addFittedText (displayFont,
                          "Time: " + Txt (averageTimeMs, 2)
                            + " ms\nEffective FPS: " + Txt (effectiveFPS, 1)
                            + "\nActual FPS: " + Txt (averageActualFPS, 1),
                          0, 10.0f, (f32) getWidth() - 10.0f, (f32) getHeight(), Justification::topRight, 3);

        g.setColor (Colors::white.withAlpha (0.5f));
        g.fillRect (ga.getBoundingBox (0, ga.getNumGlyphs(), true).getSmallestIntegerContainer().expanded (4));

        g.setColor (Colors::black);
        ga.draw (g);
    }

    virtual z0 drawDemo (Graphics&) = 0;

    z0 clipToRectangle (Graphics& g)
    {
        auto w = getWidth()  / 2;
        auto h = getHeight() / 2;

        auto x = (i32) ((f32) w * clipRectX.getValue());
        auto y = (i32) ((f32) h * clipRectY.getValue());

        g.reduceClipRegion (x, y, w, h);
    }

    z0 clipToPath (Graphics& g)
    {
        auto pathSize = (f32) jmin (getWidth(), getHeight());

        Path p;
        p.addStar (Point<f32> (clipPathX.getValue(),
                                 clipPathY.getValue()) * pathSize,
                   7,
                   pathSize * (0.5f + clipPathDepth.getValue()),
                   pathSize * 0.5f,
                   clipPathAngle.getValue());

        g.reduceClipRegion (p, AffineTransform());
    }

    z0 clipToImage (Graphics& g)
    {
        if (! clipImage.isValid())
            createClipImage();

        AffineTransform transform (AffineTransform::translation ((f32) clipImage.getWidth()  / -2.0f,
                                                                 (f32) clipImage.getHeight() / -2.0f)
                                   .rotated (clipImageAngle.getValue() * MathConstants<f32>::twoPi)
                                   .scaled (2.0f + clipImageSize.getValue() * 3.0f)
                                   .translated ((f32) getWidth()  * 0.5f,
                                                (f32) getHeight() * 0.5f));

        g.reduceClipRegion (clipImage, transform);
    }

    z0 createClipImage()
    {
        clipImage = Image (Image::ARGB, 300, 300, true);

        Graphics g (clipImage);

        g.setGradientFill (ColorGradient (Colors::transparentBlack, 0, 0,
                                           Colors::black, 0, 300, false));

        for (i32 i = 0; i < 20; ++i)
            g.fillRect (Random::getSystemRandom().nextInt (200),
                        Random::getSystemRandom().nextInt (200),
                        Random::getSystemRandom().nextInt (100),
                        Random::getSystemRandom().nextInt (100));
    }

    //==============================================================================
    ControllersComponent& controls;

    SlowerBouncingNumber offsetX, offsetY, rotation, size, shear, alpha, clipRectX,
                         clipRectY, clipPathX, clipPathY, clipPathDepth, clipPathAngle,
                         clipImageX, clipImageY, clipImageAngle, clipImageSize;

    f64 lastRenderStartTime = 0.0, averageTimeMs = 0.0, averageActualFPS = 0.0;
    Image clipImage;
    Font displayFont { FontOptions{} };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicsDemoBase)
};

//==============================================================================
class RectangleFillTypesDemo final : public GraphicsDemoBase
{
public:
    RectangleFillTypesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Fill Types: Rectangles")
    {}

    z0 drawDemo (Graphics& g) override
    {
        g.addTransform (getTransform());

        i32k rectSize = jmin (getWidth(), getHeight()) / 2 - 20;

        g.setColor (colour1.withAlpha (getAlpha()));
        g.fillRect (-rectSize, -rectSize, rectSize, rectSize);

        g.setGradientFill (ColorGradient (colour1, 10.0f, (f32) -rectSize,
                                           colour2, 10.0f + (f32) rectSize, 0.0f, false));
        g.setOpacity (getAlpha());
        g.fillRect (10, -rectSize, rectSize, rectSize);

        g.setGradientFill (ColorGradient (colour1, (f32) rectSize * -0.5f, 10.0f + (f32) rectSize * 0.5f,
                                           colour2, 0, 10.0f + (f32) rectSize, true));
        g.setOpacity (getAlpha());
        g.fillRect (-rectSize, 10, rectSize, rectSize);

        g.setGradientFill (ColorGradient (colour1, 10.0f, 10.0f,
                                           colour2, 10.0f + (f32) rectSize, 10.0f + (f32) rectSize, false));
        g.setOpacity (getAlpha());
        g.drawRect (10, 10, rectSize, rectSize, 5);
    }

    Color colour1 { Colors::red }, colour2 { Colors::green };
};

//==============================================================================
class PathsDemo final : public GraphicsDemoBase
{
public:
    PathsDemo (ControllersComponent& cc, b8 linear, b8 radial)
        : GraphicsDemoBase (cc, Txt ("Paths") + (radial ? ": Radial Gradients"
                                                           : (linear ? ": Linear Gradients"
                                                                     : ": Solid"))),
          useLinearGradient (linear), useRadialGradient (radial)
    {
        logoPath = getDRXLogoPath();

        // rescale the logo path so that it's centred about the origin and has the right size.
        logoPath.applyTransform (RectanglePlacement (RectanglePlacement::centred)
                                 .getTransformToFit (logoPath.getBounds(),
                                                     Rectangle<f32> (-120.0f, -120.0f, 240.0f, 240.0f)));

        // Surround it with some other shapes..
        logoPath.addStar ({ -300.0f, -50.0f }, 7, 30.0f, 70.0f, 0.1f);
        logoPath.addStar ({ 300.0f, 50.0f }, 6, 40.0f, 70.0f, 0.1f);
        logoPath.addEllipse (-100.0f, 150.0f, 200.0f, 140.0f);
        logoPath.addRectangle (-100.0f, -280.0f, 200.0f, 140.0f);
    }

    z0 drawDemo (Graphics& g) override
    {
        auto& p = logoPath;

        if (useLinearGradient || useRadialGradient)
        {
            Color c1 (gradientColors[0].getValue(), gradientColors[1].getValue(), gradientColors[2].getValue(), 1.0f);
            Color c2 (gradientColors[3].getValue(), gradientColors[4].getValue(), gradientColors[5].getValue(), 1.0f);
            Color c3 (gradientColors[6].getValue(), gradientColors[7].getValue(), gradientColors[8].getValue(), 1.0f);

            auto x1 = gradientPositions[0].getValue() * (f32) getWidth()  * 0.25f;
            auto y1 = gradientPositions[1].getValue() * (f32) getHeight() * 0.25f;
            auto x2 = gradientPositions[2].getValue() * (f32) getWidth()  * 0.75f;
            auto y2 = gradientPositions[3].getValue() * (f32) getHeight() * 0.75f;

            ColorGradient gradient (c1, x1, y1,
                                     c2, x2, y2,
                                     useRadialGradient);

            gradient.addColor (gradientIntermediate.getValue(), c3);

            g.setGradientFill (gradient);
        }
        else
        {
            g.setColor (Colors::blue);
        }

        g.setOpacity (getAlpha());
        g.fillPath (p, getTransform());
    }

    Path logoPath;
    b8 useLinearGradient, useRadialGradient;
    SlowerBouncingNumber gradientColors[9], gradientPositions[4], gradientIntermediate;
};

//==============================================================================
class StrokesDemo final : public GraphicsDemoBase
{
public:
    StrokesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Paths: Stroked")
    {}

    z0 drawDemo (Graphics& g) override
    {
        auto w = (f32) getWidth();
        auto h = (f32) getHeight();

        Path p;
        p.startNewSubPath (points[0].getValue() * w,
                           points[1].getValue() * h);

        for (i32 i = 2; i < numElementsInArray (points); i += 4)
            p.quadraticTo (points[i]    .getValue() * w,
                           points[i + 1].getValue() * h,
                           points[i + 2].getValue() * w,
                           points[i + 3].getValue() * h);

        p.closeSubPath();

        PathStrokeType stroke (0.5f + 10.0f * thickness.getValue());
        g.setColor (Colors::purple.withAlpha (getAlpha()));
        g.strokePath (p, stroke, AffineTransform());
    }

    SlowerBouncingNumber points[2 + 4 * 8], thickness;
};

//==============================================================================
class ImagesRenderingDemo final : public GraphicsDemoBase
{
public:
    ImagesRenderingDemo (ControllersComponent& cc, b8 argb, b8 tiled)
        : GraphicsDemoBase (cc, Txt ("Images") + (argb ? ": ARGB" : ": RGB") + (tiled ? " Tiled" : Txt() )),
          isArgb (argb), isTiled (tiled)
    {
        argbImage = getImageFromAssets ("drx_icon.png");
        rgbImage  = getImageFromAssets ("portmeirion.jpg");
    }

    z0 drawDemo (Graphics& g) override
    {
        auto image = isArgb ? argbImage : rgbImage;

        AffineTransform transform (AffineTransform::translation ((f32) (image.getWidth()  / -2),
                                                                 (f32) (image.getHeight() / -2))
                                   .followedBy (getTransform()));

        if (isTiled)
        {
            FillType fill (image, transform);
            fill.setOpacity (getAlpha());
            g.setFillType (fill);
            g.fillAll();
        }
        else
        {
            g.setOpacity (getAlpha());
            g.drawImageTransformed (image, transform, false);
        }
    }

    b8 isArgb, isTiled;
    Image rgbImage, argbImage;
};

//==============================================================================
class GlyphsDemo final : public GraphicsDemoBase
{
public:
    GlyphsDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Glyphs")
    {
        glyphs.addFittedText (FontOptions { 20.0f }, "The Quick Brown Fox Jumps Over The Lazy Dog",
                              -120, -50, 240, 100, Justification::centred, 2, 1.0f);
    }

    z0 drawDemo (Graphics& g) override
    {
        g.setColor (Colors::black.withAlpha (getAlpha()));
        glyphs.draw (g, getTransform());
    }

    GlyphArrangement glyphs;
};

//==============================================================================
class SVGDemo final : public GraphicsDemoBase
{
public:
    SVGDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "SVG")
    {
        createSVGDrawable();
    }

    z0 drawDemo (Graphics& g) override
    {
        if (Time::getCurrentTime().toMilliseconds() > lastSVGLoadTime.toMilliseconds() + 2000)
            createSVGDrawable();

        svgDrawable->draw (g, getAlpha(), getTransform());
    }

    z0 createSVGDrawable()
    {
        lastSVGLoadTime = Time::getCurrentTime();

        ZipFile icons (createAssetInputStream ("icons.zip").release(), true);

        // Load a random SVG file from our embedded icons.zip file.
        const std::unique_ptr<InputStream> svgFileStream (icons.createStreamForEntry (Random::getSystemRandom().nextInt (icons.getNumEntries())));

        if (svgFileStream.get() != nullptr)
        {
            svgDrawable = Drawable::createFromImageDataStream (*svgFileStream);

            if (svgDrawable != nullptr)
            {
                // to make our icon the right size, we'll set its bounding box to the size and position that we want.

                if (auto comp = dynamic_cast<DrawableComposite*> (svgDrawable.get()))
                    comp->setBoundingBox ({ -100.0f, -100.0f, 200.0f, 200.0f });
            }
        }
    }

    Time lastSVGLoadTime;
    std::unique_ptr<Drawable> svgDrawable;
};

class BlurDemo final : public GraphicsDemoBase
{
public:
    BlurDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Blur")
    {
        image.setBackupEnabled (false);
    }

    z0 drawDemo (Graphics& g) override
    {
        const auto a = lopassA.next (jmap (frequencyA.getValue(), 0.09f, 0.12f));
        const auto b = lopassB.next (jmap (frequencyB.getValue(), 0.09f, 0.12f));

        initialPhase += 0.01f;
        initialPhase -= (f32) (i32) initialPhase;
        const auto startAngle = initialPhase * MathConstants<f32>::twoPi;
        const auto centreSquare = image.getBounds().reduced (100);

        {
            Graphics g2 { image };
            g2.setColor (Colors::transparentBlack);
            g2.excludeClipRegion (centreSquare);
            g2.getInternalContext().fillRect (image.getBounds(), true);
        }

        if (auto ptr = image.getClippedImage (centreSquare).getPixelData())
        {
            ptr->applyGaussianBlurEffect (7.0f);
            ptr->multiplyAllAlphas (0.98f);
        }

        {
            Graphics g2 { image };
            const auto baseColor = Colors::cyan;
            const auto destColor = Colors::magenta;
            const auto offset = image.getBounds().getCentre().toFloat();
            const auto numSegments = 200;

            for (auto i = 0; i < numSegments; ++i)
            {
                g2.setColor (baseColor.interpolatedWith (destColor, (f32) i / numSegments));

                const auto getPoint = [&] (auto ind)
                {
                    return offset + Point { 200 * std::sin (startAngle + a * (f32) ind),
                                            200 * std::cos (startAngle + b * (f32) ind) };
                };

                g2.drawLine ({ getPoint (i), getPoint (i + 1) }, 2.0f);
            }
        }

        AffineTransform transform (AffineTransform::translation ((f32) (-image.getWidth()  / 2),
                                                                 (f32) (-image.getHeight() / 2))
                                   .followedBy (getTransform()));

        g.setOpacity (getAlpha());
        g.drawImageTransformed (image, transform, false);
    }

    class Lopass
    {
    public:
        f32 next (f32 f) { return value += (f - value) * 0.05f; }

    private:
        f32 value{};
    };

    Image image { Image::ARGB, 512, 512, true };
    SlowerBouncingNumber frequencyA, frequencyB;
    Lopass lopassA, lopassB;
    f32 initialPhase = 0.0f;
};

//==============================================================================
class LinesDemo final : public GraphicsDemoBase
{
public:
    LinesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Lines")
    {}

    z0 drawDemo (Graphics& g) override
    {
        {
            RectangleList<f32> verticalLines;
            verticalLines.ensureStorageAllocated (getWidth());

            auto pos = offset.getValue();

            for (i32 x = 0; x < getWidth(); ++x)
            {
                auto y = (f32) getHeight() * 0.3f;
                auto length = y * std::abs (std::sin ((f32) x / 100.0f + 2.0f * pos));
                verticalLines.addWithoutMerging (Rectangle<f32> ((f32) x, y - length * 0.5f, 1.0f, length));
            }

            g.setColor (Colors::blue.withAlpha (getAlpha()));
            g.fillRectList (verticalLines);
        }

        {
            RectangleList<f32> horizontalLines;
            horizontalLines.ensureStorageAllocated (getHeight());

            auto pos = offset.getValue();

            for (i32 y = 0; y < getHeight(); ++y)
            {
                auto x = (f32) getWidth() * 0.3f;
                auto length = x * std::abs (std::sin ((f32) y / 100.0f + 2.0f * pos));
                horizontalLines.addWithoutMerging (Rectangle<f32> (x - length * 0.5f, (f32) y, length, 1.0f));
            }

            g.setColor (Colors::green.withAlpha (getAlpha()));
            g.fillRectList (horizontalLines);
        }

        g.setColor (Colors::red.withAlpha (getAlpha()));

        auto w = (f32) getWidth();
        auto h = (f32) getHeight();

        g.drawLine (positions[0].getValue() * w,
                    positions[1].getValue() * h,
                    positions[2].getValue() * w,
                    positions[3].getValue() * h);

        g.drawLine (positions[4].getValue() * w,
                    positions[5].getValue() * h,
                    positions[6].getValue() * w,
                    positions[7].getValue() * h,
                    10.0f * thickness.getValue());
    }

    SlowerBouncingNumber offset, positions[8], thickness;
};

//==============================================================================
class ShapesDemo final : public GraphicsDemoBase
{
public:
    explicit ShapesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Shapes")
    {}

    z0 drawDemo (Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const auto rowHeight = bounds.getHeight() / 2.0f;
        const auto spacing = 5.0f;

        const auto drawShapes = [&] (auto area)
        {
            const auto lineThickness = thickness.getValue() * 25.0f;
            const auto cornerSize = 15.0f;
            const auto shapeWidth = area.getWidth() / 4.0f;

            g.drawEllipse (area.removeFromLeft (shapeWidth).reduced (spacing), lineThickness);
            g.fillEllipse (area.removeFromLeft (shapeWidth).reduced (spacing));

            g.drawRoundedRectangle (area.removeFromLeft (shapeWidth).reduced (spacing), cornerSize, lineThickness);
            g.fillRoundedRectangle (area.removeFromLeft (shapeWidth).reduced (spacing), cornerSize);
        };

        g.addTransform (AffineTransform::translation (-(bounds.getWidth() / 2.0f), -(bounds.getHeight() / 2.0f)).followedBy (getTransform()));

        g.setColor (drx::Colors::red.withAlpha (getAlpha()));
        drawShapes (bounds.removeFromTop (rowHeight).reduced (spacing));

        const auto r = bounds.removeFromTop (rowHeight).reduced (spacing);
        g.setGradientFill (drx::ColorGradient (drx::Colors::green, r.getTopLeft(),
                                                 drx::Colors::blue, r.getBottomRight(),
                                                 false));
        g.setOpacity (getAlpha());
        drawShapes (r);
    }

    SlowerBouncingNumber thickness;
};

//==============================================================================
class DemoHolderComponent final : public Component,
                                  private Timer
{
public:
    DemoHolderComponent()
    {
        setOpaque (true);
    }

    z0 paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds().toFloat(), 48.0f, 48.0f,
                            Colors::lightgrey, Colors::white);
    }

    z0 timerCallback() override
    {
        if (currentDemo != nullptr)
            currentDemo->repaint();
    }

    z0 setDemo (GraphicsDemoBase* newDemo)
    {
        if (currentDemo != nullptr)
            removeChildComponent (currentDemo);

        currentDemo = newDemo;

        if (currentDemo != nullptr)
        {
            addAndMakeVisible (currentDemo);
            startTimerHz (60);
            resized();
        }
    }

    z0 resized() override
    {
        if (currentDemo != nullptr)
            currentDemo->setBounds (getLocalBounds());
    }

private:
    GraphicsDemoBase* currentDemo = nullptr;
};

//==============================================================================
class TestListComponent final : public Component,
                                private ListBoxModel
{
public:
    TestListComponent (DemoHolderComponent& holder, ControllersComponent& controls)
        : demoHolder (holder)
    {
        demos.add (new PathsDemo (controls, false, true));
        demos.add (new PathsDemo (controls, true,  false));
        demos.add (new PathsDemo (controls, false, false));
        demos.add (new RectangleFillTypesDemo (controls));
        demos.add (new StrokesDemo (controls));
        demos.add (new ImagesRenderingDemo (controls, false, false));
        demos.add (new ImagesRenderingDemo (controls, false, true));
        demos.add (new ImagesRenderingDemo (controls, true,  false));
        demos.add (new ImagesRenderingDemo (controls, true,  true));
        demos.add (new BlurDemo   (controls));
        demos.add (new GlyphsDemo (controls));
        demos.add (new SVGDemo    (controls));
        demos.add (new LinesDemo  (controls));
        demos.add (new ShapesDemo (controls));

        addAndMakeVisible (listBox);
        listBox.setTitle ("Test List");
        listBox.setModel (this);
        listBox.selectRow (0);
    }

    z0 resized() override
    {
        listBox.setBounds (getLocalBounds());
    }

    i32 getNumRows() override
    {
        return demos.size();
    }

    z0 paintListBoxItem (i32 rowNumber, Graphics& g, i32 width, i32 height, b8 rowIsSelected) override
    {
        if (demos[rowNumber] == nullptr)
            return;

        if (rowIsSelected)
            g.fillAll (Color::contrasting (findColor (ListBox::textColorId),
                                            findColor (ListBox::backgroundColorId)));

        g.setColor (findColor (ListBox::textColorId));
        g.setFont (14.0f);
        g.drawFittedText (getNameForRow (rowNumber), 8, 0, width - 10, height, Justification::centredLeft, 2);
    }

    Txt getNameForRow (i32 rowNumber) override
    {
        if (auto* demo = demos[rowNumber])
            return demo->getName();

        return {};
    }

    z0 selectedRowsChanged (i32 lastRowSelected) override
    {
        demoHolder.setDemo (demos [lastRowSelected]);
    }

private:
    DemoHolderComponent& demoHolder;
    ListBox listBox;
    OwnedArray<GraphicsDemoBase> demos;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestListComponent)
};

//==============================================================================
class GraphicsDemo final : public Component
{
public:
    GraphicsDemo()
        : testList (demoHolder, controllersComponent)
    {
        setOpaque (true);

        addAndMakeVisible (demoHolder);
        addAndMakeVisible (controllersComponent);
        addAndMakeVisible (performanceDisplay);
        addAndMakeVisible (testList);

        setSize (750, 750);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::grey);
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        controllersComponent.setBounds (area.removeFromBottom (150));
        testList            .setBounds (area.removeFromRight (150));
        demoHolder          .setBounds (area);
        performanceDisplay  .setBounds (area.removeFromTop (20).removeFromRight (100));
    }

private:
    ControllersComponent controllersComponent;
    DemoHolderComponent demoHolder;
    Label performanceDisplay;
    TestListComponent testList;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicsDemo)
};
