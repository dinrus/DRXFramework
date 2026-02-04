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


struct BlankCanvas final : public AnimatedContent
{
    Txt getName() const override      { return "Blank Canvas"; }
    z0 reset() override {}
    z0 handleTouch (Point<f32>) override {}
    z0 generateCanvas (Graphics&, SharedCanvasDescription&, Rectangle<f32>) override {}
};

//==============================================================================
struct GridLines final : public AnimatedContent
{
    Txt getName() const override      { return "Grid Lines"; }
    z0 reset() override {}
    z0 handleTouch (Point<f32>) override {}

    z0 generateCanvas (Graphics& g, SharedCanvasDescription& canvas, Rectangle<f32>) override
    {
        auto limits = canvas.getLimits();
        f32 lineThickness = 0.1f;

        g.setColor (Colors::blue);
        g.drawRect (canvas.getLimits(), lineThickness);

        for (f32 y = limits.getY(); y < limits.getBottom(); y += 2.0f)
            g.drawLine (limits.getX(), y, limits.getRight(), y, lineThickness);

        for (f32 x = limits.getX(); x < limits.getRight(); x += 2.0f)
            g.drawLine (x, limits.getY(), x, limits.getBottom(), lineThickness);

        g.setColor (Colors::darkred);
        g.drawLine (limits.getX(), limits.getCentreY(), limits.getRight(), limits.getCentreY(), lineThickness);
        g.drawLine (limits.getCentreX(), limits.getY(), limits.getCentreX(), limits.getBottom(), lineThickness);

        g.setColor (Colors::lightgrey);
        g.drawLine (limits.getX(), limits.getY(), limits.getRight(), limits.getBottom(), lineThickness);
        g.drawLine (limits.getX(), limits.getBottom(), limits.getRight(), limits.getY(), lineThickness);
    }
};

//==============================================================================
struct BackgroundLogo : public AnimatedContent
{
    BackgroundLogo()
    {
        static const t8 logoData[] = R"blahblah(
            <svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
                 viewBox="0 0 239.2 239.2" enable-background="new 0 0 239.2 239.2" xml:space="preserve">
            <path fill="#6CC04A" d="M118.8,201.3c-44.6,0-81-36.3-81-81s36.3-81,81-81s81,36.3,81,81S163.4,201.3,118.8,201.3z M118.8,44.8c-41.7,0-75.6,33.9-75.6,75.6s33.9,75.6,75.6,75.6s75.6-33.9,75.6-75.6S160.4,44.8,118.8,44.8z"/>
            <path fill="#3B5CAD" d="M182.6,117.6c1.4,0,2.7-0.5,3.7-1.5c1.1-1.1,1.6-2.5,1.4-4c-1.5-12.7-6.5-24.7-14.4-34.8c-1-1.2-2.3-1.9-3.8-1.9c-1.3,0-2.6,0.5-3.6,1.5l-39,39c-0.6,0.6-0.2,1.6,0.7,1.6L182.6,117.6z"/>
            <path fill="#E73E51" d="M169.5,165.2L169.5,165.2c1.5,0,2.8-0.7,3.8-1.9c7.9-10.1,12.9-22.1,14.4-34.8c0.2-1.5-0.3-2.9-1.4-4c-1-1-2.3-1.5-3.7-1.5l-55,0c-0.9,0-1.3,1-0.7,1.6l39,39C166.9,164.7,168.2,165.2,169.5,165.2z"/>
            <path fill="#E67E3C" d="M122.9,188L122.9,188c1,1,2.5,1.5,4,1.3c12.7-1.5,24.8-6.5,34.8-14.4c1.2-0.9,1.8-2.3,1.9-3.8c0-1.4-0.6-2.7-1.6-3.7l-38.9-38.9c-0.6-0.6-1.6-0.2-1.6,0.7l0,55.2C121.4,185.8,122,187,122.9,188z"/>
            <path fill="#F0E049" d="M68,75.4c-1.5,0-2.8,0.7-3.8,1.9c-7.9,10.1-12.9,22.1-14.4,34.8c-0.2,1.5,0.3,2.9,1.4,4c1,1,2.3,1.5,3.7,1.5l55,0c0.9,0,1.3-1,0.7-1.6l-39-39C70.6,76,69.3,75.4,68,75.4z"/>
            <path fill="#D5D755" d="M114.6,52.7c-1-1-2.5-1.5-4-1.3c-12.7,1.5-24.8,6.5-34.8,14.4c-1.2,0.9-1.8,2.3-1.9,3.8c0,1.4,0.6,2.7,1.6,3.7l38.9,38.9c0.6,0.6,1.6,0.2,1.6-0.7l0-55.2C116.1,54.9,115.5,53.6,114.6,52.7z"/>
            <path fill="#9CB6D3" d="M163.7,69.6c0-1.5-0.7-2.8-1.9-3.8c-10.1-7.9-22.1-12.9-34.8-14.4c-1.5-0.2-2.9,0.3-4,1.4c-1,1-1.5,2.3-1.5,3.7l0,55c0,0.9,1,1.3,1.6,0.7l39-39C163.1,72.1,163.7,70.9,163.7,69.6z"/>
            <path fill="#F5BD47" d="M109.9,123l-55,0c-1.4,0-2.7,0.5-3.7,1.5c-1.1,1.1-1.6,2.5-1.4,4c1.5,12.7,6.5,24.7,14.4,34.8c1,1.2,2.3,1.9,3.8,1.9c1.3,0,2.6-0.5,3.5-1.5c0,0,0,0,0,0l39-39C111.2,124,110.8,123,109.9,123z"/>
            <path fill="#F19F53" d="M114.4,128.5l-38.9,38.9c-1,1-1.6,2.3-1.6,3.7c0,1.5,0.7,2.9,1.9,3.8c10,7.9,22.1,12.9,34.8,14.4c1.6,0.2,3-0.3,4-1.3c0.9-0.9,1.4-2.2,1.4-3.6c0,0,0,0,0,0l0-55.2C116.1,128.3,115,127.9,114.4,128.5z"/>
            </svg>
            )blahblah";

        logo = Drawable::createFromSVG (*parseXML (logoData));
    }

    Txt getName() const override      { return "Background Image"; }
    z0 reset() override {}
    z0 handleTouch (Point<f32>) override {}

    z0 generateCanvas (Graphics& g, SharedCanvasDescription& canvas, Rectangle<f32>) override
    {
        logo->drawWithin (g, canvas.getLimits().reduced (3.0f), RectanglePlacement (RectanglePlacement::centred), 0.6f);
    }

    std::unique_ptr<Drawable> logo;
};

//==============================================================================
struct FlockDemo : public BackgroundLogo
{
    Txt getName() const override      { return "Flock"; }

    z0 setNumBirds (i32 numBirds)
    {
        BackgroundLogo::reset();

        birds.clear();

        for (i32 i = numBirds; --i >= 0;)
            birds.add ({});

        centreOfGravity = {};
        lastGravityMove = {};
        fakeMouseTouchLengthToRun = 0;
        fakeMouseTouchPosition = {};
        fakeMouseTouchVelocity = {};
    }

    z0 reset() override
    {
        BackgroundLogo::reset();
        setNumBirds (100);
    }

    z0 generateCanvas (Graphics& g, SharedCanvasDescription& canvas, Rectangle<f32> activeArea) override
    {
        BackgroundLogo::generateCanvas (g, canvas, activeArea);

        if (Time::getCurrentTime() > lastGravityMove + RelativeTime::seconds (0.5))
        {
            if (fakeMouseTouchLengthToRun > 0)
            {
                --fakeMouseTouchLengthToRun;
                fakeMouseTouchPosition += fakeMouseTouchVelocity;
                centreOfGravity = fakeMouseTouchPosition;
            }
            else
            {
                centreOfGravity = {};

                if (rng.nextInt (300) == 2 && canvas.clients.size() > 0)
                {
                    fakeMouseTouchLengthToRun = 50;
                    fakeMouseTouchPosition = canvas.clients.getReference (rng.nextInt (canvas.clients.size())).centre;
                    fakeMouseTouchVelocity = { rng.nextFloat() * 0.3f - 0.15f,
                                               rng.nextFloat() * 0.3f - 0.15f };
                }
            }
        }

        g.setColor (Colors::white.withAlpha (0.2f));

        if (! centreOfGravity.isOrigin())
            g.fillEllipse (centreOfGravity.getX() - 1.0f, centreOfGravity.getY() - 1.0f, 2.0f, 2.0f);

        for (i32 i = 0; i < birds.size(); ++i)
            for (i32 j = i + 1; j < birds.size(); ++j)
                attractBirds (birds.getReference (i), birds.getReference (j));

        for (auto& b : birds)
        {
            if (! centreOfGravity.isOrigin())
                b.move (centreOfGravity, 0.4f);

            b.update();
            b.draw (g);
            b.bounceOffEdges (canvas.getLimits().expanded (1.0f));
        }

        for (i32 i = rings.size(); --i >= 0;)
        {
            if (rings.getReference (i).update())
                rings.getReference (i).draw (g);
            else
                rings.remove (i);
        }
    }

    b8 isRingNear (Point<f32> p) const
    {
        for (auto& r : rings)
            if (r.centre.getDistanceFrom (p) < 1.0f)
                return true;

        return false;
    }

    z0 handleTouch (Point<f32> position) override
    {
        lastGravityMove = Time::getCurrentTime();
        centreOfGravity = position;
        fakeMouseTouchLengthToRun = 0;

        if (! isRingNear (position))
            rings.add ({ position, 1.0f, 0.5f });
    }

    //==============================================================================
    struct Bird
    {
        Bird()
        {
            Random randGen;
            pos.x = randGen.nextFloat() * 10.0f - 5.0f;
            pos.y = randGen.nextFloat() * 10.0f - 5.0f;
            velocity.x = randGen.nextFloat() * 0.001f;
            velocity.y = randGen.nextFloat() * 0.001f;

            colour = Color::fromHSV (randGen.nextFloat(), 0.2f, 0.9f, randGen.nextFloat() * 0.4f + 0.2f);

            shape.addTriangle (0.0f, 0.0f, -0.3f, 1.0f, 0.3f, 1.0f);
            shape = shape.createPathWithRoundedCorners (0.2f);

            shape.applyTransform (AffineTransform::scale (randGen.nextFloat() + 1.0f));
        }

        Point<f32> pos, velocity, acc;
        Color colour;
        Path shape;

        z0 move (Point<f32> target, f32 strength)
        {
            auto r = target - pos;
            f32 rSquared = jmax (0.1f, (r.x * r.x) + (r.y * r.y));

            if (rSquared > 1.0f)
                velocity += (r * strength / rSquared);

            acc = {};
        }

        z0 accelerate (Point<f32> acceleration)
        {
            acc += acceleration;
        }

        z0 bounceOffEdges (Rectangle<f32> limits)
        {
            if (pos.x < limits.getX())      { velocity.x =  std::abs (velocity.x); acc = {}; }
            if (pos.x > limits.getRight())  { velocity.x = -std::abs (velocity.x); acc = {}; }
            if (pos.y < limits.getY())      { velocity.y =  std::abs (velocity.y); acc = {}; }
            if (pos.y > limits.getBottom()) { velocity.y = -std::abs (velocity.y); acc = {}; }
        }

        z0 update()
        {
            velocity += acc;

            f32 length = velocity.getDistanceFromOrigin();
            const f32 maxSpeed = 0.5f;

            if (length > maxSpeed)
                velocity = getVectorWithLength (velocity, maxSpeed);

            pos += velocity;
        }

        z0 draw (Graphics& g)
        {
            g.setColor (colour);
            g.fillPath (shape, AffineTransform::rotation (Point<f32>().getAngleToPoint (velocity)).translated (pos));
        }
    };

    static Point<f32> getVectorWithLength (Point<f32> v, f32 newLength)
    {
        return v * (newLength / v.getDistanceFromOrigin());
    }

    static z0 attractBirds (Bird& b1, Bird& b2)
    {
        auto delta = b1.pos - b2.pos;

        const f32 zoneRadius = 10.0f;
        const f32 low = 0.4f;
        const f32 high = 0.65f;
        const f32 strength = 0.01f;

        const f32 distanceSquared = (delta.x * delta.x) * (delta.y * delta.y);

        if (distanceSquared < zoneRadius * zoneRadius && distanceSquared > 0.01f)
        {
            f32 proportion = distanceSquared / (zoneRadius * zoneRadius);

            if (proportion < low)
            {
                const f32 F = (low / proportion - 1.0f) * strength * 0.003f;
                delta = getVectorWithLength (delta, F);

                b1.accelerate (delta);
                b2.accelerate (-delta);
            }
            else if (proportion < high)
            {
                const f32 regionSize = high - low;
                const f32 adjustedProportion = (proportion - low) / regionSize;
                const f32 F = (0.5f - std::cos (adjustedProportion * MathConstants<f32>::twoPi) * 0.5f + 0.5f) * strength;

                b1.accelerate (getVectorWithLength (b2.velocity, F));
                b2.accelerate (getVectorWithLength (b1.velocity, F));
            }
            else
            {
                const f32 regionSize = 1.0f - high;
                const f32 adjustedProportion = (proportion - high) / regionSize;
                const f32 F = (0.5f - std::cos (adjustedProportion * MathConstants<f32>::twoPi) * 0.5f + 0.5f) * strength;
                delta = getVectorWithLength (delta, F);

                b1.accelerate (-delta);
                b2.accelerate (delta);
            }
        }
    }

    Random rng;
    Array<Bird> birds;
    Point<f32> centreOfGravity;
    Time lastGravityMove;

    i32 fakeMouseTouchLengthToRun = 0;
    Point<f32> fakeMouseTouchPosition, fakeMouseTouchVelocity;

    //==============================================================================
    struct Ring
    {
        Point<f32> centre;
        f32 diameter, opacity;

        b8 update()
        {
            diameter += 0.7f;
            opacity -= 0.01f;

            return opacity > 0;
        }

        z0 draw (Graphics& g)
        {
            const f32 thickness = 0.2f;

            auto r = Rectangle<f32> (diameter, diameter).withCentre (centre);

            Path p;
            p.addEllipse (r);
            p.addEllipse (r.reduced (thickness));
            p.setUsingNonZeroWinding (false);

            g.setColor (Colors::white.withAlpha (opacity));
            g.fillPath (p);

        }
    };

    Array<Ring> rings;
};

//==============================================================================
struct FlockWithText final : public FlockDemo
{
    FlockWithText()
    {
        messages.add ("DRX is our cross-platform C++ framework\n\n"
                      "In this demo, the same C++ app is running natively on NUMDEVICES devices,\n"
                      "which are sharing their graphic state via the network");

        messages.add ("No other libraries were needed to create this demo.\n"
                      "DRX provides thousands of classes for cross-platform GUI,\n"
                      "audio, networking, data-structures and many other common tasks");

        messages.add ("As well as a code library, DRX provides tools for managing\n"
                      "cross-platform projects that are built with Xcode,\n"
                      "Visual Studio, Android Studio, GCC and other compilers");

        messages.add ("DRX can be used to build desktop or mobile apps, and also\n"
                      "audio plug-ins in the VST2, VST3, AudioUnit and AAX formats");
    }

    Txt getName() const override      { return "Flock with text"; }

    z0 reset() override
    {
        FlockDemo::reset();

        currentMessage = 0;
        currentMessageStart = {};
        clientIndex = 0;
    }

    z0 generateCanvas (Graphics& g, SharedCanvasDescription& canvas, Rectangle<f32> activeArea) override
    {
        FlockDemo::generateCanvas (g, canvas, activeArea);

        const f32 textSize = 0.5f; // inches
        const f32 textBlockWidth = 20.0f;  // inches

        tick();

        Graphics::ScopedSaveState ss (g);
        const f32 scale = 20.0f; // scaled to allow the fonts to use more reasonable sizes
        g.addTransform (AffineTransform::scale (1.0f / scale));

        Txt text = Txt (messages[currentMessage]).replace ("NUMDEVICES", Txt (canvas.clients.size()));

        AttributedString as;
        as.append (text, FontOptions (textSize * scale), Color (0x80ffffff).withMultipliedAlpha (alpha));

        as.setJustification (Justification::centred);
        auto middle = canvas.clients[clientIndex % canvas.clients.size()].centre * scale;
        as.draw (g, Rectangle<f32> (textBlockWidth * scale, textBlockWidth * scale).withCentre (middle));
    }

    z0 tick()
    {
        const f64 displayTimeSeconds = 5.0;
        const f64 fadeTimeSeconds = 1.0;

        Time now = Time::getCurrentTime();
        const f64 secondsSinceStart = (now - currentMessageStart).inSeconds();

        if (secondsSinceStart > displayTimeSeconds)
        {
            currentMessageStart = now;
            currentMessage = (currentMessage + 1) % messages.size();
            ++clientIndex;
            alpha = 0;
        }
        else if (secondsSinceStart > displayTimeSeconds - fadeTimeSeconds)
        {
            alpha = (f32) jlimit (0.0, 1.0, (displayTimeSeconds - secondsSinceStart) / fadeTimeSeconds);
        }
        else if (secondsSinceStart < fadeTimeSeconds)
        {
            alpha = (f32) jlimit (0.0, 1.0, secondsSinceStart / fadeTimeSeconds);
        }
    }

    StringArray messages;
    i32 currentMessage = 0, clientIndex = 0;
    f32 alpha = 0;
    Point<f32> centre;
    Time currentMessageStart;
};

//==============================================================================
struct SmallFlock final : public FlockDemo
{
    Txt getName() const override      { return "Small Flock"; }

    z0 reset() override
    {
        setNumBirds (20);
    }
};

//==============================================================================
struct BigFlock final : public FlockDemo
{
    Txt getName() const override      { return "Big Flock"; }

    z0 reset() override
    {
        setNumBirds (200);
    }
};

//==============================================================================
template <i32 numHorizontalLogos>
struct MultiLogo final : public BackgroundLogo
{
    Txt getName() const override      { return "Multi-Logo " + Txt ((i32) numHorizontalLogos); }

    z0 generateCanvas (Graphics& g, SharedCanvasDescription& canvas, Rectangle<f32>) override
    {
        f32 indent = 0.5f;
        f32 logoSize = canvas.getLimits().getWidth() / numHorizontalLogos;
        auto limits = canvas.getLimits();

        for (f32 x = limits.getX(); x < limits.getRight(); x += logoSize)
        {
            for (f32 y = limits.getY(); y < limits.getBottom(); y += logoSize)
            {
                logo->drawWithin (g, Rectangle<f32> (x, y, logoSize, logoSize).reduced (indent),
                                  RectanglePlacement (RectanglePlacement::centred), 0.5f);
            }
        }
    }
};

//==============================================================================
inline z0 createAllDemos (OwnedArray<AnimatedContent>& demos)
{
    demos.add (new FlockDemo());
    demos.add (new FlockWithText());
    demos.add (new SmallFlock());
    demos.add (new BigFlock());
    demos.add (new BackgroundLogo());
    demos.add (new MultiLogo<5>());
    demos.add (new MultiLogo<10>());
    demos.add (new GridLines());
    demos.add (new BlankCanvas());
}
