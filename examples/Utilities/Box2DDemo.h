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

 name:             Box2DDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases 2D graphics features.

 dependencies:     drx_box2d, drx_core, drx_data_structures, drx_events,
                   drx_graphics, drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        Box2DDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
// (These classes and random functions are used inside the 3rd-party Box2D demo code)
inline float32 RandomFloat()                           { return Random::getSystemRandom().nextFloat() * 2.0f - 1.0f; }
inline float32 RandomFloat (float32 lo, float32 hi)    { return Random::getSystemRandom().nextFloat() * (hi - lo) + lo; }

struct Settings
{
    b2Vec2 viewCenter  { 0.0f, 20.0f };
    float32 hz = 60.0f;
    i32 velocityIterations = 8;
    i32 positionIterations = 3;
    i32 drawShapes         = 1;
    i32 drawJoints         = 1;
    i32 drawAABBs          = 0;
    i32 drawPairs          = 0;
    i32 drawContactPoints  = 0;
    i32 drawContactNormals = 0;
    i32 drawContactForces  = 0;
    i32 drawFrictionForces = 0;
    i32 drawCOMs           = 0;
    i32 drawStats          = 0;
    i32 drawProfile        = 0;
    i32 enableWarmStarting = 1;
    i32 enableContinuous   = 1;
    i32 enableSubStepping  = 0;
    i32 pause              = 0;
    i32 singleStep         = 0;
};

struct Test
{
    Test()          = default;
    virtual ~Test() = default;

    virtual z0 Keyboard (u8 /*key*/)   {}
    virtual z0 KeyboardUp (u8 /*key*/) {}

    std::unique_ptr<b2World> m_world  { new b2World (b2Vec2 (0.0f, -10.0f)) };
};

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-i32-f32-conversion", "-Wsuggest-override")

#include "../Assets/Box2DTests/AddPair.h"
#include "../Assets/Box2DTests/ApplyForce.h"
#include "../Assets/Box2DTests/Dominos.h"
#include "../Assets/Box2DTests/Chain.h"

DRX_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
/** This list box just displays a StringArray and broadcasts a change message when the
    selected row changes.
*/
class Box2DTestList final : public ListBoxModel,
                            public ChangeBroadcaster
{
public:
    Box2DTestList (const StringArray& testList)
        : tests (testList)
    {}

    i32 getNumRows() override                                      { return tests.size(); }

    z0 selectedRowsChanged (i32 /*lastRowSelected*/) override    { sendChangeMessage(); }

    z0 paintListBoxItem (i32 row, Graphics& g, i32 w, i32 h, b8 rowIsSelected) override
    {
        auto& lf = LookAndFeel::getDefaultLookAndFeel();

        if (rowIsSelected)
            g.fillAll (Color::contrasting (lf.findColor (ListBox::textColorId),
                                            lf.findColor (ListBox::backgroundColorId)));

        g.setColor (lf.findColor (ListBox::textColorId));
        g.setFont ((f32) h * 0.7f);
        g.drawText (tests[row], Rectangle<i32> (0, 0, w, h).reduced (2),
                    Justification::centredLeft, true);
    }

private:
    StringArray tests;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box2DTestList)
};

//==============================================================================
struct Box2DRenderComponent final : public Component
{
    Box2DRenderComponent()
    {
        setOpaque (true);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::white);

        if (currentTest.get() != nullptr)
        {
            Box2DRenderer renderer;

            renderer.render (g, *currentTest->m_world,
                             -16.0f, 30.0f, 16.0f, -1.0f,
                             getLocalBounds().toFloat().reduced (8.0f));
        }
    }

    std::unique_ptr<Test> currentTest;
};

//==============================================================================
class Box2DDemo final : public Component,
                        private Timer,
                        private ChangeListener
{
public:
    enum Demos
    {
        addPair = 0,
        applyForce,
        dominoes,
        chain,
        numTests
    };

    Box2DDemo()
        : testsList (getTestsList())
    {
        setOpaque (true);
        setWantsKeyboardFocus (true);

        testsListModel.addChangeListener (this);

        addAndMakeVisible (renderComponent);

        addAndMakeVisible (testsListBox);
        testsListBox.setModel (&testsListModel);
        testsListBox.selectRow (dominoes);

        addAndMakeVisible (instructions);
        instructions.setMultiLine (true);
        instructions.setReadOnly (true);

        startTimerHz (60);

        setSize (500, 500);
    }

    ~Box2DDemo() override
    {
        testsListModel.removeChangeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (4);

        auto area = r.removeFromBottom (150);
        testsListBox.setBounds (area.removeFromLeft (150));

        area.removeFromLeft (4);
        instructions.setBounds (area);

        r.removeFromBottom (6);
        renderComponent.setBounds (r);
    }

    b8 keyPressed (const KeyPress& key) override
    {
        if (renderComponent.currentTest.get() != nullptr)
        {
            // We override this to avoid the system beeping for an unused keypress
            switch (key.getTextCharacter())
            {
                case 'a':
                case 'w':
                case 'd':
                    return true;

                default:
                    break;
            }
        }

        return false;
    }

private:
    StringArray testsList;
    Box2DTestList testsListModel  { testsList };

    Box2DRenderComponent renderComponent;
    ListBox testsListBox;
    TextEditor instructions;

    static Test* createTest (i32 index)
    {
        switch (index)
        {
            case addPair:       return new AddPair();
            case applyForce:    return new ApplyForce();
            case dominoes:      return new Dominos();
            case chain:         return new Chain();
            default:            break;
        }

        return nullptr;
    }

    static Txt getInstructions (i32 index)
    {
        switch (index)
        {
            case applyForce:
                return Txt ("Keys:") + newLine + "Left: \'a\'" + newLine
                        + "Right: \'d\'" + newLine + "Forward: \'w\'";

            default:
                break;
        }

        return {};
    }

    z0 checkKeys()
    {
        if (renderComponent.currentTest.get() == nullptr)
            return;

        checkKeyCode ('a');
        checkKeyCode ('w');
        checkKeyCode ('d');
    }

    z0 checkKeyCode (i32k keyCode)
    {
        if (KeyPress::isKeyCurrentlyDown (keyCode))
            renderComponent.currentTest->Keyboard ((u8) keyCode);
    }

    z0 timerCallback() override
    {
        if (renderComponent.currentTest.get() == nullptr)
            return;

        if (isShowing())
            grabKeyboardFocus();

        checkKeys();
        renderComponent.currentTest->m_world->Step (1.0f / 60.0f, 6, 2);
        repaint();
    }

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &testsListModel)
        {
            auto index = testsListBox.getSelectedRow();

            renderComponent.currentTest.reset (createTest (index));
            instructions.setText (getInstructions (index));

            repaint();
        }
    }

    z0 lookAndFeelChanged() override
    {
        instructions.applyFontToAllText (instructions.getFont());
    }

    static StringArray getTestsList()
    {
        return { "Add Pair Stress Test", "Apply Force", "Dominoes", "Chain" };
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box2DDemo)
};
