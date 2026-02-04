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

 name:             WidgetsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases various widgets.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        WidgetsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#ifndef PIP_DEMO_UTILITIES_INCLUDED
#include "../Assets/DemoUtilities.h"
#endif

//==============================================================================
static z0 showBubbleMessage (Component& targetComponent, const Txt& textToShow,
                               std::unique_ptr<BubbleMessageComponent>& bmc,
                               b8 isRunningComponentTransformDemo);

//==============================================================================
/** To demonstrate how sliders can have custom snapping applied to their values,
    this simple class snaps the value to 50 if it comes near.
*/
struct SnappingSlider final : public Slider
{
    f64 snapValue (f64 attemptedValue, DragMode dragMode) override
    {
        if (dragMode == notDragging)
            return attemptedValue;  // if they're entering the value in the text-box, don't mess with it.

        if (attemptedValue > 40 && attemptedValue < 60)
            return 50.0;

        return attemptedValue;
    }
};

/** A TextButton that pops up a colour chooser to change its colours. */
class ColorChangeButton final : public TextButton,
                                 public ChangeListener
{
public:
    ColorChangeButton()
        : TextButton ("Click to change colour...")
    {
        setSize (10, 24);
        changeWidthToFitText();
    }

    z0 clicked() override
    {
        auto colourSelector = std::make_unique<ColorSelector> (ColorSelector::showAlphaChannel
                                                                | ColorSelector::showColorAtTop
                                                                | ColorSelector::editableColor
                                                                | ColorSelector::showSliders
                                                                | ColorSelector::showColorspace);

        colourSelector->setName ("background");
        colourSelector->setCurrentColor (findColor (TextButton::buttonColorId));
        colourSelector->addChangeListener (this);
        colourSelector->setColor (ColorSelector::backgroundColorId, Colors::transparentBlack);
        colourSelector->setSize (300, 400);

        CallOutBox::launchAsynchronously (std::move (colourSelector), getScreenBounds(), nullptr);
    }

    using TextButton::clicked;

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (auto* cs = dynamic_cast<ColorSelector*> (source))
            setColor (TextButton::buttonColorId, cs->getCurrentColor());
    }
};

//==============================================================================
struct SlidersPage final : public Component
{
    SlidersPage()
    {
        Rectangle<i32> layoutArea { 20, 20, 580, 430 };
        auto sliderArea = layoutArea.removeFromTop (320);

        auto* s = createSlider (false);
        s->setSliderStyle (Slider::LinearVertical);
        s->setTextBoxStyle (Slider::TextBoxBelow, false, 100, 20);
        s->setBounds (sliderArea.removeFromLeft (70));
        s->setDoubleClickReturnValue (true, 50.0); // f64-clicking this slider will set it to 50.0
        s->setTextValueSuffix (" units");

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearVertical);
        s->setVelocityBasedMode (true);
        s->setSkewFactor (0.5);
        s->setTextBoxStyle (Slider::TextBoxAbove, true, 100, 20);
        s->setBounds (sliderArea.removeFromLeft (70));
        s->setTextValueSuffix (" rels");

        sliderArea.removeFromLeft (20);
        auto horizontalSliderArea = sliderArea.removeFromLeft (180);

        s = createSlider (true);
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        s->setBounds (horizontalSliderArea.removeFromTop (20));

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        horizontalSliderArea.removeFromTop (20);
        s->setBounds (horizontalSliderArea.removeFromTop (20));
        s->setPopupDisplayEnabled (true, false, this);
        s->setTextValueSuffix (" nuns required to change a lightbulb");

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::TextEntryBoxPosition::TextBoxAbove, false, 70, 20);
        horizontalSliderArea.removeFromTop (20);
        s->setBounds (horizontalSliderArea.removeFromTop (50));
        s->setPopupDisplayEnabled (true, false, this);

        s = createSlider (false);
        s->setSliderStyle (Slider::IncDecButtons);
        s->setTextBoxStyle (Slider::TextBoxLeft, false, 50, 20);
        horizontalSliderArea.removeFromTop (20);
        s->setBounds (horizontalSliderArea.removeFromTop (20));
        s->setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);

        s = createSlider (false);
        s->setSliderStyle (Slider::Rotary);
        s->setRotaryParameters (MathConstants<f32>::pi * 1.2f, MathConstants<f32>::pi * 2.8f, false);
        s->setTextBoxStyle (Slider::TextBoxRight, false, 70, 20);
        horizontalSliderArea.removeFromTop (15);
        s->setBounds (horizontalSliderArea.removeFromTop (70));
        s->setTextValueSuffix (" mm");

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearBar);
        horizontalSliderArea.removeFromTop (10);
        s->setBounds (horizontalSliderArea.removeFromTop (30));
        s->setTextValueSuffix (" gallons");

        sliderArea.removeFromLeft (20);
        auto twoValueSliderArea = sliderArea.removeFromLeft (180);

        s = createSlider (false);
        s->setSliderStyle (Slider::TwoValueHorizontal);
        s->setBounds (twoValueSliderArea.removeFromTop (40));

        s = createSlider (false);
        s->setSliderStyle (Slider::ThreeValueHorizontal);
        s->setPopupDisplayEnabled (true, false, this);
        twoValueSliderArea.removeFromTop (10);
        s->setBounds (twoValueSliderArea.removeFromTop (40));

        s = createSlider (false);
        s->setSliderStyle (Slider::TwoValueVertical);
        twoValueSliderArea.removeFromLeft (30);
        s->setBounds (twoValueSliderArea.removeFromLeft (40));

        s = createSlider (false);
        s->setSliderStyle (Slider::ThreeValueVertical);
        s->setPopupDisplayEnabled (true, false, this);
        twoValueSliderArea.removeFromLeft (30);
        s->setBounds (twoValueSliderArea.removeFromLeft (40));

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearBarVertical);
        s->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        sliderArea.removeFromLeft (20);
        s->setBounds (sliderArea.removeFromLeft (20));
        s->setPopupDisplayEnabled (true, true, this);
        s->setTextValueSuffix (" mickles in a muckle");

        /* Here, we'll create a Value object, and tell a bunch of our sliders to use it as their
           value source. By telling them all to share the same Value, they'll stay in sync with
           each other.

           We could also optionally keep a copy of this Value elsewhere, and by changing it,
           cause all the sliders to automatically update.
        */
        Value sharedValue;
        sharedValue = Random::getSystemRandom().nextDouble() * 100;
        for (i32 i = 0; i < 8; ++i)
            sliders.getUnchecked (i)->getValueObject().referTo (sharedValue);

        // ..and now we'll do the same for all our min/max slider values..
        Value sharedValueMin, sharedValueMax;
        sharedValueMin = Random::getSystemRandom().nextDouble() * 40.0;
        sharedValueMax = Random::getSystemRandom().nextDouble() * 40.0 + 60.0;

        for (i32 i = 8; i <= 11; ++i)
        {
            auto* selectedSlider = sliders.getUnchecked (i);
            selectedSlider->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
            selectedSlider->getMaxValueObject().referTo (sharedValueMax);
            selectedSlider->getMinValueObject().referTo (sharedValueMin);
        }

        hintLabel.setBounds (layoutArea);
        addAndMakeVisible (hintLabel);
    }

private:
    OwnedArray<Slider> sliders;
    Label hintLabel  { "hint", "Try right-clicking on a slider for an options menu. \n\n"
                               "Also, holding down CTRL while dragging will turn on a slider's velocity-sensitive mode" };

    Slider* createSlider (b8 isSnapping)
    {
        auto* s = isSnapping ? new SnappingSlider()
                             : new Slider();

        sliders.add (s);
        addAndMakeVisible (s);
        s->setRange (0.0, 100.0, 0.1);
        s->setPopupMenuEnabled (true);
        s->setValue (Random::getSystemRandom().nextDouble() * 100.0, dontSendNotification);
        return s;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidersPage)
};

//==============================================================================
struct ButtonsPage final : public Component
{
    ButtonsPage (b8 isRunningComponentTransformDemo)
    {
        {
            auto* group = addToList (new GroupComponent ("group", "Radio buttons"));
            group->setBounds (20, 20, 220, 140);
        }

        for (i32 i = 0; i < 4; ++i)
        {
            auto* tb = addToList (new ToggleButton ("Radio Button #" + Txt (i + 1)));

            tb->setRadioGroupId (1234);
            tb->setBounds (45, 46 + i * 22, 180, 22);
            tb->setTooltip ("A set of mutually-exclusive radio buttons");

            if (i == 0)
                tb->setToggleState (true, dontSendNotification);
        }

        for (i32 i = 0; i < 4; ++i)
        {
            DrawablePath normal, over;

            Path p;
            p.addStar ({}, i + 5, 20.0f, 50.0f, -0.2f);
            normal.setPath (p);
            normal.setFill (Colors::lightblue);
            normal.setStrokeFill (Colors::black);
            normal.setStrokeThickness (4.0f);

            over.setPath (p);
            over.setFill (Colors::blue);
            over.setStrokeFill (Colors::black);
            over.setStrokeThickness (4.0f);

            auto* db = addToList (new DrawableButton (Txt (i + 5) + " points", DrawableButton::ImageAboveTextLabel));
            db->setImages (&normal, &over, nullptr);
            db->setClickingTogglesState (true);
            db->setRadioGroupId (23456);

            i32 buttonSize = 50;
            db->setBounds (25 + i * buttonSize, 180, buttonSize, buttonSize);

            if (i == 0)
                db->setToggleState (true, dontSendNotification);
        }

        for (i32 i = 0; i < 4; ++i)
        {
            auto* tb = addToList (new TextButton ("Button " + Txt (i + 1)));

            tb->setClickingTogglesState (true);
            tb->setRadioGroupId (34567);
            tb->setColor (TextButton::textColorOffId,  Colors::black);
            tb->setColor (TextButton::textColorOnId,   Colors::black);
            tb->setColor (TextButton::buttonColorId,   Colors::white);
            tb->setColor (TextButton::buttonOnColorId, Colors::blueviolet.brighter());

            tb->setBounds (20 + i * 55, 260, 55, 24);
            tb->setConnectedEdges (((i != 0) ? Button::ConnectedOnLeft : 0)
                                    | ((i != 3) ? Button::ConnectedOnRight : 0));

            if (i == 0)
                tb->setToggleState (true, dontSendNotification);
        }

        {
            auto* colourChangeButton = new ColorChangeButton();
            components.add (colourChangeButton);
            addAndMakeVisible (colourChangeButton);
            colourChangeButton->setTopLeftPosition (20, 320);
        }

        {
            auto* hyperlink = addToList (new HyperlinkButton ("This is a HyperlinkButton",
                                                              { "http://www.drx.com" }));
            hyperlink->setBounds (260, 20, 200, 24);
        }

        // create some drawables to use for our drawable buttons...
        DrawablePath normal, over;

        {
            Path p;
            p.addStar ({}, 5, 20.0f, 50.0f, 0.2f);
            normal.setPath (p);
            normal.setFill (getRandomDarkColor());
        }

        {
            Path p;
            p.addStar ({}, 9, 25.0f, 50.0f, 0.0f);
            over.setPath (p);
            over.setFill (getRandomBrightColor());
            over.setStrokeFill (getRandomDarkColor());
            over.setStrokeThickness (5.0f);
        }

        DrawableImage down;
        down.setImage (getImageFromAssets ("drx_icon.png"));
        down.setOverlayColor (Colors::black.withAlpha (0.3f));

        auto popupMessageCallback = [this, isRunningComponentTransformDemo]
        {
            if (auto* focused = Component::getCurrentlyFocusedComponent())
                showBubbleMessage (*focused,
                                   "This is a demo of the BubbleMessageComponent, which lets you pop up a message pointing "
                                   "at a component or somewhere on the screen.\n\n"
                                   "The message bubbles will disappear after a timeout period, or when the mouse is clicked.",
                                   this->bubbleMessage,
                                   isRunningComponentTransformDemo);
        };

        {
            // create an image-above-text button from these drawables..
            auto db = addToList (new DrawableButton ("Button 1", DrawableButton::ImageAboveTextLabel));
            db->setImages (&normal, &over, &down);
            db->setBounds (260, 60, 80, 80);
            db->setTooltip ("This is a DrawableButton with a label");
            db->onClick = popupMessageCallback;
        }

        {
            // create an image-only button from these drawables..
            auto db = addToList (new DrawableButton ("Button 2", DrawableButton::ImageFitted));
            db->setImages (&normal, &over, &down);
            db->setClickingTogglesState (true);
            db->setBounds (370, 60, 80, 80);
            db->setTooltip ("This is an image-only DrawableButton");
            db->onClick = popupMessageCallback;
        }

        {
            // create an image-on-button-shape button from the same drawables..
            auto db = addToList (new DrawableButton ("Button 3", DrawableButton::ImageOnButtonBackground));
            db->setImages (&normal, nullptr, nullptr);
            db->setBounds (260, 160, 110, 25);
            db->setTooltip ("This is a DrawableButton on a standard button background");
            db->onClick = popupMessageCallback;
        }

        {
            auto db = addToList (new DrawableButton ("Button 4", DrawableButton::ImageOnButtonBackground));
            db->setImages (&normal, &over, &down);
            db->setClickingTogglesState (true);
            db->setColor (DrawableButton::backgroundColorId,   Colors::white);
            db->setColor (DrawableButton::backgroundOnColorId, Colors::yellow);
            db->setBounds (400, 150, 50, 50);
            db->setTooltip ("This is a DrawableButton on a standard button background");
            db->onClick = popupMessageCallback;
        }

        {
            auto sb = addToList (new ShapeButton ("ShapeButton",
                                                  getRandomDarkColor(),
                                                  getRandomDarkColor(),
                                                  getRandomDarkColor()));
            sb->setShape (getDRXLogoPath(), false, true, false);
            sb->setBounds (260, 220, 200, 120);
        }

        {
            auto ib = addToList (new ImageButton ("ImageButton"));

            auto juceImage = getImageFromAssets ("drx_icon.png");

            ib->setImages (true, true, true,
                           juceImage, 0.7f, Colors::transparentBlack,
                           juceImage, 1.0f, Colors::transparentBlack,
                           juceImage, 1.0f, getRandomBrightColor().withAlpha (0.8f),
                           0.5f);

            ib->setBounds (45, 380, 100, 100);
            ib->setTooltip ("ImageButton - showing alpha-channel hit-testing and colour overlay when clicked");
        }
    }

private:
    OwnedArray<Component> components;
    std::unique_ptr<BubbleMessageComponent> bubbleMessage;

    TooltipWindow tooltipWindow;

    // This little function avoids a bit of code-duplication by adding a component to
    // our list as well as calling addAndMakeVisible on it..
    template <typename ComponentType>
    ComponentType* addToList (ComponentType* newComp)
    {
        components.add (newComp);
        addAndMakeVisible (newComp);
        return newComp;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonsPage)
};


//==============================================================================
struct MiscPage final : public Component,
                        private Timer
{
    MiscPage()
    {
        addAndMakeVisible (textEditor1);
        textEditor1.setBounds (10, 25, 200, 24);
        textEditor1.setText ("Single-line text box");

        addAndMakeVisible (textEditor2);
        textEditor2.setBounds (10, 55, 200, 24);
        textEditor2.setText ("Password");

        addAndMakeVisible (comboBox);
        comboBox.setBounds (10, 85, 200, 24);
        comboBox.setEditableText (true);
        comboBox.setJustificationType (Justification::centred);

        for (i32 i = 1; i < 100; ++i)
            comboBox.addItem ("combo box item " + Txt (i), i);

        comboBox.setSelectedId (1);

        addAndMakeVisible (linearProgressBar);
        linearProgressBar.setStyle (ProgressBar::Style::linear);
        linearProgressBar.setBounds (10, 115, 200, 24);

        addAndMakeVisible (circularProgressBar);
        circularProgressBar.setStyle (ProgressBar::Style::circular);
        circularProgressBar.setBounds (10, 145, 200, 100);

        startTimerHz (10);
    }

    ~MiscPage() override
    {
        stopTimer();
    }

    z0 lookAndFeelChanged() override
    {
        textEditor1.applyFontToAllText (textEditor1.getFont());
        textEditor2.applyFontToAllText (textEditor2.getFont());
    }

    z0 timerCallback() override
    {
        constexpr auto minValue = -0.2;
        constexpr auto maxValue = 1.2;
        constexpr auto maxIncrement = 0.05;

        if (progress >= maxValue)
            progress = minValue;
        else
            progress += Random::getSystemRandom().nextDouble() * maxIncrement;

        if (isPositiveAndNotGreaterThan (progress, 1.0))
        {
            linearProgressBar.setPercentageDisplay (true);
            circularProgressBar.setPercentageDisplay (true);
        }
        else
        {
            linearProgressBar.setTextToDisplay ("Linear progress bar");
            circularProgressBar.setTextToDisplay ("Circular progress bar");
        }
    }

    TextEditor textEditor1,
               textEditor2  { "Password", (t32) 0x2022 };

    ComboBox comboBox  { "Combo" };

    f64 progress { 0.0 };
    ProgressBar linearProgressBar { progress };
    ProgressBar circularProgressBar { progress };
};

//==============================================================================
struct MenuPage final : public Component
{
    MenuPage()
    {
        addAndMakeVisible (shortMenuButton);
        shortMenuButton.onClick = [&]
        {
            PopupMenu menu;
            menu.addItem ("Single Item", nullptr);
            menu.showMenuAsync (PopupMenu::Options{}.withTargetComponent (shortMenuButton));
        };

        addAndMakeVisible (longMenuButton);
        longMenuButton.onClick = [&]
        {
            PopupMenu menu;

            for (i32 i = 0; i < 40; ++i)
                menu.addItem ("Item " + Txt (i), nullptr);

            menu.showMenuAsync (PopupMenu::Options{}.withTargetComponent (longMenuButton));
        };

        addAndMakeVisible (nestedMenusButton);
        nestedMenusButton.onClick = [&]
        {
            PopupMenu menu;

            for (i32 i = 0; i < 15; ++i)
            {
                PopupMenu subMenu;

                for (i32 j = 0; j < 10; ++j)
                {
                    if (j % 2 == 0)
                    {
                        PopupMenu subSubMenu;

                        for (i32 z = 0; z < 5; ++z)
                            subSubMenu.addItem ("Sub-sub-item " + Txt (z), nullptr);

                        subMenu.addSubMenu ("Sub-item " + Txt (j), subSubMenu);
                    }
                    else
                    {
                        subMenu.addItem ("Sub-item " + Txt (j), nullptr);
                    }
                }

                menu.addSubMenu ("Item " + Txt (i), subMenu);
            }

            menu.showMenuAsync (PopupMenu::Options{}.withTargetComponent (nestedMenusButton));
        };

        addAndMakeVisible (multiColumnMenuButton);
        multiColumnMenuButton.onClick = [&]
        {
            PopupMenu menu;

            for (i32 i = 0; i < 200; ++i)
                menu.addItem ("Item " + Txt (i), nullptr);

            menu.showMenuAsync (PopupMenu::Options{}.withTargetComponent (multiColumnMenuButton)
                                                    .withMinimumNumColumns (2)
                                                    .withMaximumNumColumns (4));
        };

        addAndMakeVisible (customItemButton);
        customItemButton.onClick = [&]
        {
            struct CustomComponent final : public PopupMenu::CustomComponent
            {
                CustomComponent (i32 widthIn, i32 heightIn, Color backgroundIn)
                    : PopupMenu::CustomComponent (false),
                      idealWidth (widthIn),
                      idealHeight (heightIn),
                      background (backgroundIn)
                {}

                z0 getIdealSize (i32& width, i32& height) override
                {
                    width = idealWidth;
                    height = idealHeight;
                }

                z0 paint (Graphics& g) override { g.fillAll (background); }

                i32 idealWidth = 0;
                i32 idealHeight = 0;
                Color background;
            };

            PopupMenu menu;

            menu.addCustomItem (-1, std::make_unique<CustomComponent> (100,  20, Colors::darkred));
            menu.addCustomItem (-1, std::make_unique<CustomComponent> (20,  100, Colors::darkgreen));
            menu.addCustomItem (-1, std::make_unique<CustomComponent> (100, 100, Colors::darkblue));
            menu.addCustomItem (-1, std::make_unique<CustomComponent> (100,  50, Colors::darkcyan));
            menu.addCustomItem (-1, std::make_unique<CustomComponent> (50,  100, Colors::darkmagenta));

            menu.showMenuAsync (PopupMenu::Options{}.withTargetComponent (customItemButton)
                                                    .withMinimumNumColumns (5));
        };

        addAndMakeVisible (fancyThemeButton);
        fancyThemeButton.setLookAndFeel (&popupLookAndFeel);
        fancyThemeButton.onClick = [&]
        {
            const auto colour = Color::fromHSL (randomColorGenerator.nextFloat(), 0.5f, 0.5f, 1.0f);
            fancyThemeButton.setColor (TextButton::buttonColorId, colour);

            PopupMenu menu;
            menu.setLookAndFeel (&popupLookAndFeel);

            for (auto length : { 5, 10, 7, 3 })
            {
                for (i32 i = 0; i < length; ++i)
                    menu.addItem ("Item " + Txt (i), nullptr);

                menu.addColumnBreak();
            }

            menu.showMenuAsync (PopupMenu::Options{}.withTargetComponent (&fancyThemeButton));
        };
    }

    z0 resized() override
    {
        const auto makeItem = [] (Component& comp)
        {
            return FlexItem { comp }.withWidth (200).withHeight (24).withMargin ({ 4 });
        };

        FlexBox box;
        box.flexDirection = FlexBox::Direction::column;
        box.items = { makeItem (shortMenuButton),
                      makeItem (longMenuButton),
                      makeItem (nestedMenusButton),
                      makeItem (multiColumnMenuButton),
                      makeItem (customItemButton),
                      makeItem (fancyThemeButton) };

        box.performLayout (getLocalBounds());
    }

    struct PopupMenuLookAndFeel : public LookAndFeel_V4
    {
        z0 drawPopupMenuColumnSeparatorWithOptions (Graphics& g,
                                                      const Rectangle<i32>& bounds,
                                                      const PopupMenu::Options& opt) override
        {
            if (auto* target = opt.getTargetComponent())
            {
                const auto baseColor = target->findColor (TextButton::buttonColorId);
                g.setColor (baseColor.brighter (0.4f));

                const f32 dashes[] { 5.0f, 5.0f };
                const auto centre = bounds.toFloat().getCentre();

                g.drawDashedLine ({ centre.withY ((f32) bounds.getY()),
                                    centre.withY ((f32) bounds.getBottom()) },
                                  dashes,
                                  numElementsInArray (dashes),
                                  3.0f);
            }
        }

        z0 drawPopupMenuBackgroundWithOptions (Graphics& g, i32, i32, const PopupMenu::Options& opt) override
        {
            if (auto* target = opt.getTargetComponent())
            {
                g.fillAll (target->findColor (TextButton::buttonColorId));
            }
        }

        // Return the amount of space that should be left between popup menu columns.
        i32 getPopupMenuColumnSeparatorWidthWithOptions (const PopupMenu::Options&) override
        {
            return 10;
        }
    };

    Random randomColorGenerator;
    PopupMenuLookAndFeel popupLookAndFeel;

    TextButton shortMenuButton       { "Short" },
               longMenuButton        { "Long" },
               nestedMenusButton     { "Nested Sub-Menus" },
               multiColumnMenuButton { "Multi Column" },
               customItemButton      { "Custom Items" },
               fancyThemeButton      { "Fancy Theme with Column Breaks" };
};

//==============================================================================
class ToolbarDemoComp final : public Component,
                              private Slider::Listener
{
public:
    ToolbarDemoComp()
    {
        // Create and add the toolbar...
        addAndMakeVisible (toolbar);

        // And use our item factory to add a set of default icons to it...
        toolbar.addDefaultItems (factory);

        // Now we'll just create the other sliders and buttons on the demo page, which adjust
        // the toolbar's properties...
        addAndMakeVisible (infoLabel);
        infoLabel.setJustificationType (Justification::topLeft);
        infoLabel.setBounds (80, 80, 450, 100);
        infoLabel.setInterceptsMouseClicks (false, false);

        addAndMakeVisible (depthSlider);
        depthSlider.setRange (10.0, 200.0, 1.0);
        depthSlider.setValue (50, dontSendNotification);
        depthSlider.addListener (this);
        depthSlider.setBounds (80, 210, 300, 22);
        depthLabel.attachToComponent (&depthSlider, false);

        addAndMakeVisible (orientationButton);
        orientationButton.onClick = [this] { toolbar.setVertical (! toolbar.isVertical()); resized(); };
        orientationButton.changeWidthToFitText (22);
        orientationButton.setTopLeftPosition (depthSlider.getX(), depthSlider.getBottom() + 20);

        addAndMakeVisible (customiseButton);
        customiseButton.onClick = [this] { toolbar.showCustomisationDialog (factory); };
        customiseButton.changeWidthToFitText (22);
        customiseButton.setTopLeftPosition (orientationButton.getRight() + 20, orientationButton.getY());
    }

    z0 resized() override
    {
        auto toolbarThickness = (i32) depthSlider.getValue();

        if (toolbar.isVertical())
            toolbar.setBounds (getLocalBounds().removeFromLeft (toolbarThickness));
        else
            toolbar.setBounds (getLocalBounds().removeFromTop  (toolbarThickness));
    }

    z0 sliderValueChanged (Slider*) override
    {
        resized();
    }

private:
    Toolbar toolbar;

    Slider depthSlider  { Slider::LinearHorizontal, Slider::TextBoxLeft };

    Label depthLabel  { {}, "Toolbar depth:" },
          infoLabel   { {}, "As well as showing off toolbars, this demo illustrates how to store "
                            "a set of SVG files in a Zip file, embed that in your application, and read "
                            "them back in at runtime.\n\nThe icon images here are taken from the open-source "
                            "Tango icon project."};

    TextButton orientationButton  { "Vertical/Horizontal" },
               customiseButton    { "Customise..." };

    //==============================================================================
    class DemoToolbarItemFactory final : public ToolbarItemFactory
    {
    public:
        DemoToolbarItemFactory() {}

        //==============================================================================
        // Each type of item a toolbar can contain must be given a unique ID. These
        // are the ones we'll use in this demo.
        enum DemoToolbarItemIds
        {
            doc_new         = 1,
            doc_open        = 2,
            doc_save        = 3,
            doc_saveAs      = 4,
            edit_copy       = 5,
            edit_cut        = 6,
            edit_paste      = 7,
            juceLogoButton  = 8,
            customComboBox  = 9
        };

        z0 getAllToolbarItemIds (Array<i32>& ids) override
        {
            // This returns the complete list of all item IDs that are allowed to
            // go in our toolbar. Any items you might want to add must be listed here. The
            // order in which they are listed will be used by the toolbar customisation panel.

            ids.add (doc_new);
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (edit_copy);
            ids.add (edit_cut);
            ids.add (edit_paste);
            ids.add (juceLogoButton);
            ids.add (customComboBox);

            // If you're going to use separators, then they must also be added explicitly
            // to the list.
            ids.add (separatorBarId);
            ids.add (spacerId);
            ids.add (flexibleSpacerId);
        }

        z0 getDefaultItemSet (Array<i32>& ids) override
        {
            // This returns an ordered list of the set of items that make up a
            // toolbar's default set. Not all items need to be on this list, and
            // items can appear multiple times (e.g. the separators used here).
            ids.add (doc_new);
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (spacerId);
            ids.add (separatorBarId);
            ids.add (edit_copy);
            ids.add (edit_cut);
            ids.add (edit_paste);
            ids.add (separatorBarId);
            ids.add (flexibleSpacerId);
            ids.add (customComboBox);
            ids.add (flexibleSpacerId);
            ids.add (separatorBarId);
            ids.add (juceLogoButton);
        }

        ToolbarItemComponent* createItem (i32 itemId) override
        {
            switch (itemId)
            {
                case doc_new:           return createButtonFromZipFileSVG (itemId, "new",     "document-new.svg");
                case doc_open:          return createButtonFromZipFileSVG (itemId, "open",    "document-open.svg");
                case doc_save:          return createButtonFromZipFileSVG (itemId, "save",    "document-save.svg");
                case doc_saveAs:        return createButtonFromZipFileSVG (itemId, "save as", "document-save-as.svg");
                case edit_copy:         return createButtonFromZipFileSVG (itemId, "copy",    "edit-copy.svg");
                case edit_cut:          return createButtonFromZipFileSVG (itemId, "cut",     "edit-cut.svg");
                case edit_paste:        return createButtonFromZipFileSVG (itemId, "paste",   "edit-paste.svg");

                case juceLogoButton:
                {
                    auto drawable = std::make_unique<DrawableImage>();
                    drawable->setImage (getImageFromAssets ("drx_icon.png"));
                    return new ToolbarButton (itemId, "drx!", std::move (drawable), {});
                }

                case customComboBox:    return new CustomToolbarComboBox (itemId);
                default:                break;
            }

            return nullptr;
        }

    private:
        StringArray iconNames;
        OwnedArray<Drawable> iconsFromZipFile;

        // This is a little utility to create a button with one of the SVG images in
        // our embedded ZIP file "icons.zip"
        ToolbarButton* createButtonFromZipFileSVG (i32k itemId, const Txt& text, const Txt& filename)
        {
            if (iconsFromZipFile.size() == 0)
            {
                // If we've not already done so, load all the images from the zip file..
                ZipFile icons (createAssetInputStream ("icons.zip").release(), true);

                for (i32 i = 0; i < icons.getNumEntries(); ++i)
                {
                    std::unique_ptr<InputStream> svgFileStream (icons.createStreamForEntry (i));

                    if (svgFileStream.get() != nullptr)
                    {
                        iconNames.add (icons.getEntry (i)->filename);
                        iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                    }
                }
            }

            auto* image = iconsFromZipFile[iconNames.indexOf (filename)];
            return new ToolbarButton (itemId, text, image->createCopy(), {});
        }

        // Demonstrates how to put a custom component into a toolbar - this one contains
        // a ComboBox.
        class CustomToolbarComboBox final : public ToolbarItemComponent
        {
        public:
            CustomToolbarComboBox (i32k toolbarItemId)
                : ToolbarItemComponent (toolbarItemId, "Custom Toolbar Item", false)
            {
                addAndMakeVisible (comboBox);

                for (i32 i = 1; i < 20; ++i)
                    comboBox.addItem ("Toolbar ComboBox item " + Txt (i), i);

                comboBox.setSelectedId (1);
                comboBox.setEditableText (true);
            }

            b8 getToolbarItemSizes (i32 /*toolbarDepth*/, b8 isVertical,
                                      i32& preferredSize, i32& minSize, i32& maxSize) override
            {
                if (isVertical)
                    return false;

                preferredSize = 250;
                minSize = 80;
                maxSize = 300;
                return true;
            }

            z0 paintButtonArea (Graphics&, i32, i32, b8, b8) override
            {
            }

            z0 contentAreaChanged (const Rectangle<i32>& newArea) override
            {
                comboBox.setSize (newArea.getWidth() - 2,
                                  jmin (newArea.getHeight() - 2, 22));

                comboBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }

        private:
            ComboBox comboBox  { "demo toolbar combo box" };
        };
    };

    DemoToolbarItemFactory factory;
};


//==============================================================================
/**
    This class shows how to implement a TableListBoxModel to show in a TableListBox.
*/
class TableDemoComponent final : public Component,
                                 public TableListBoxModel
{
public:
    TableDemoComponent()
    {
        // Load some data from an embedded XML file..
        loadData();

        // Create our table component and add it to this component..
        addAndMakeVisible (table);
        table.setModel (this);

        // give it a border
        table.setColor (ListBox::outlineColorId, Colors::grey);
        table.setOutlineThickness (1);

        // Add some columns to the table header, based on the column list in our database..
        for (auto* columnXml : columnList->getChildIterator())
        {
            table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                         columnXml->getIntAttribute ("columnId"),
                                         columnXml->getIntAttribute ("width"),
                                         50, 400,
                                         TableHeaderComponent::defaultFlags);
        }

        // we could now change some initial settings..
        table.getHeader().setSortColumnId (1, true); // sort forwards by the ID column
        table.getHeader().setColumnVisible (7, false); // hide the "length" column until the user shows it

        // un-comment this line to have a go of stretch-to-fit mode
        // table.getHeader().setStretchToFitActive (true);

        table.setMultipleSelectionEnabled (true);
    }

    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    i32 getNumRows() override
    {
        return numRows;
    }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    z0 paintRowBackground (Graphics& g, i32 rowNumber, i32 /*width*/, i32 /*height*/, b8 rowIsSelected) override
    {
        auto alternateColor = getLookAndFeel().findColor (ListBox::backgroundColorId)
                                               .interpolatedWith (getLookAndFeel().findColor (ListBox::textColorId), 0.03f);
        if (rowIsSelected)
            g.fillAll (Colors::lightblue);
        else if (rowNumber % 2)
            g.fillAll (alternateColor);
    }

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    z0 paintCell (Graphics& g, i32 rowNumber, i32 columnId,
                    i32 width, i32 height, b8 /*rowIsSelected*/) override
    {
        g.setColor (getLookAndFeel().findColor (ListBox::textColorId));
        g.setFont (font);

        if (auto* rowElement = dataList->getChildElement (rowNumber))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

            g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }

        g.setColor (getLookAndFeel().findColor (ListBox::backgroundColorId));
        g.fillRect (width - 1, 0, 1, height);
    }

    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    z0 sortOrderChanged (i32 newSortColumnId, b8 isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            dataList->sortChildElements (sorter);

            table.updateContent();
        }
    }

    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (i32 rowNumber, i32 columnId, b8 /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
        if (columnId == 1 || columnId == 7) // The ID and Length columns do not have a custom component
        {
            jassert (existingComponentToUpdate == nullptr);
            return nullptr;
        }

        if (columnId == 5) // For the ratings column, we return the custom combobox component
        {
            auto* ratingsBox = static_cast<RatingColumnCustomComponent*> (existingComponentToUpdate);

            // If an existing component is being passed-in for updating, we'll re-use it, but
            // if not, we'll have to create one.
            if (ratingsBox == nullptr)
                ratingsBox = new RatingColumnCustomComponent (*this);

            ratingsBox->setRowAndColumn (rowNumber, columnId);
            return ratingsBox;
        }

        // The other columns are editable text columns, for which we use the custom Label component
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        // same as above...
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent (*this);

        textLabel->setRowAndColumn (rowNumber, columnId);
        return textLabel;
    }

    // This is overloaded from TableListBoxModel, and should choose the best width for the specified
    // column.
    i32 getColumnAutoSizeWidth (i32 columnId) override
    {
        if (columnId == 5)
            return 100; // (this is the ratings column, containing a custom combobox component)

        i32 widest = 32;

        // find the widest bit of text in this column..
        for (i32 i = getNumRows(); --i >= 0;)
        {
            if (auto* rowElement = dataList->getChildElement (i))
            {
                auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

                widest = jmax (widest, GlyphArrangement::getStringWidthInt (font, text));
            }
        }

        return widest + 8;
    }

    // A couple of quick methods to set and get cell values when the user changes them
    i32 getRating (i32k rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute ("Rating");
    }

    z0 setRating (i32k rowNumber, i32k newRating)
    {
        dataList->getChildElement (rowNumber)->setAttribute ("Rating", newRating);
    }

    Txt getText (i32k columnNumber, i32k rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getStringAttribute (getAttributeNameForColumnId (columnNumber));
    }

    z0 setText (i32k columnNumber, i32k rowNumber, const Txt& newText)
    {
        auto columnName = table.getHeader().getColumnName (columnNumber);
        dataList->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }

    //==============================================================================
    z0 resized() override
    {
        // position our table with a gap around its edge
        table.setBoundsInset (BorderSize<i32> (8));
    }


private:
    TableListBox table;     // the table component itself
    Font font { FontOptions { 14.0f } };

    std::unique_ptr<XmlElement> demoData;  // This is the XML document loaded from the embedded file "demo table data.xml"
    XmlElement* columnList = nullptr;     // A pointer to the sub-node of demoData that contains the list of columns
    XmlElement* dataList   = nullptr;     // A pointer to the sub-node of demoData that contains the list of data rows
    i32 numRows;                          // The number of rows of data we've got

    //==============================================================================
    // This is a custom Label component, which we use for the table's editable text columns.
    class EditableTextCustomComponent final : public Label
    {
    public:
        EditableTextCustomComponent (TableDemoComponent& td)  : owner (td)
        {
            // f64 click to edit the label text; single click handled below
            setEditable (false, true, false);
        }

        z0 mouseDown (const MouseEvent& event) override
        {
            // single click on the label should simply select the row
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }

        z0 textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }

        // Our demo code will call this when we may need to update our contents
        z0 setRowAndColumn (i32k newRow, i32k newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText (columnId, row), dontSendNotification);
        }

        z0 paint (Graphics& g) override
        {
            auto& lf = getLookAndFeel();
            if (! dynamic_cast<LookAndFeel_V4*> (&lf))
                lf.setColor (textColorId, Colors::black);

            Label::paint (g);
        }

    private:
        TableDemoComponent& owner;
        i32 row, columnId;
        Color textColor;
    };

    //==============================================================================
    // This is a custom component containing a combo box, which we're going to put inside
    // our table's "rating" column.
    class RatingColumnCustomComponent final : public Component
    {
    public:
        RatingColumnCustomComponent (TableDemoComponent& td)  : owner (td)
        {
            // just put a combo box inside this component
            addAndMakeVisible (comboBox);
            comboBox.addItem ("fab",        1);
            comboBox.addItem ("groovy",     2);
            comboBox.addItem ("hep",        3);
            comboBox.addItem ("mad for it", 4);
            comboBox.addItem ("neat",       5);
            comboBox.addItem ("swingin",    6);
            comboBox.addItem ("wild",       7);

            comboBox.onChange = [this] { owner.setRating (row, comboBox.getSelectedId()); };
            comboBox.setWantsKeyboardFocus (false);
        }

        z0 resized() override
        {
            comboBox.setBoundsInset (BorderSize<i32> (2));
        }

        // Our demo code will call this when we may need to update our contents
        z0 setRowAndColumn (i32 newRow, i32 newColumn)
        {
            row = newRow;
            columnId = newColumn;
            comboBox.setSelectedId (owner.getRating (row), dontSendNotification);
        }

    private:
        TableDemoComponent& owner;
        ComboBox comboBox;
        i32 row, columnId;
    };

    //==============================================================================
    // A comparator used to sort our data when the user clicks a column header
    class DemoDataSorter
    {
    public:
        DemoDataSorter (const Txt& attributeToSortBy, b8 forwards)
            : attributeToSort (attributeToSortBy),
              direction (forwards ? 1 : -1)
        {
        }

        i32 compareElements (XmlElement* first, XmlElement* second) const
        {
            auto result = first->getStringAttribute (attributeToSort)
                                .compareNatural (second->getStringAttribute (attributeToSort));

            if (result == 0)
                result = first->getStringAttribute ("ID")
                               .compareNatural (second->getStringAttribute ("ID"));

            return direction * result;
        }

    private:
        Txt attributeToSort;
        i32 direction;
    };

    //==============================================================================
    // this loads the embedded database XML file into memory
    z0 loadData()
    {
        demoData = parseXML (loadEntireAssetIntoString ("demo table data.xml"));

        dataList   = demoData->getChildByName ("DATA");
        columnList = demoData->getChildByName ("COLUMNS");

        numRows = dataList->getNumChildElements();
    }

    // (a utility method to search our XML for the attribute that matches a column ID)
    Txt getAttributeNameForColumnId (i32k columnId) const
    {
        for (auto* columnXml : columnList->getChildIterator())
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("name");
        }

        return {};
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableDemoComponent)
};

//==============================================================================
class DragAndDropDemo final : public Component,
                              public DragAndDropContainer
{
public:
    DragAndDropDemo()
    {
        setName ("Drag-and-Drop");

        sourceListBox.setModel (&sourceModel);
        sourceListBox.setMultipleSelectionEnabled (true);

        addAndMakeVisible (sourceListBox);
        addAndMakeVisible (target);
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (8);

        sourceListBox.setBounds (r.withSize (250, 180));
        target       .setBounds (r.removeFromBottom (150).removeFromRight (250));
    }

private:
    //==============================================================================
    struct SourceItemListboxContents final : public ListBoxModel
    {
        // The following methods implement the necessary virtual functions from ListBoxModel,
        // telling the listbox how many rows there are, painting them, etc.
        i32 getNumRows() override
        {
            return 30;
        }

        z0 paintListBoxItem (i32 rowNumber, Graphics& g,
                               i32 width, i32 height, b8 rowIsSelected) override
        {
            if (rowIsSelected)
                g.fillAll (Colors::lightblue);

            g.setColor (LookAndFeel::getDefaultLookAndFeel().findColor (Label::textColorId));
            g.setFont ((f32) height * 0.7f);

            g.drawText ("Draggable Thing #" + Txt (rowNumber + 1),
                        5, 0, width, height,
                        Justification::centredLeft, true);
        }

        var getDragSourceDescription (const SparseSet<i32>& selectedRows) override
        {
            // for our drag description, we'll just make a comma-separated list of the selected row
            // numbers - this will be picked up by the drag target and displayed in its box.
            StringArray rows;

            for (i32 i = 0; i < selectedRows.size(); ++i)
                rows.add (Txt (selectedRows[i] + 1));

            return rows.joinIntoString (", ");
        }
    };

    //==============================================================================
    // and this is a component that can have things dropped onto it..
    class DragAndDropDemoTarget final : public Component,
                                        public DragAndDropTarget,
                                        public FileDragAndDropTarget,
                                        public TextDragAndDropTarget
    {
    public:
        DragAndDropDemoTarget()    {}

        z0 paint (Graphics& g) override
        {
            g.fillAll (Colors::green.withAlpha (0.2f));

            // draw a red line around the comp if the user's currently dragging something over it..
            if (somethingIsBeingDraggedOver)
            {
                g.setColor (Colors::red);
                g.drawRect (getLocalBounds(), 3);
            }

            g.setColor (getLookAndFeel().findColor (Label::textColorId));
            g.setFont (14.0f);
            g.drawFittedText (message, getLocalBounds().reduced (10, 0), Justification::centred, 4);
        }

        //==============================================================================
        // These methods implement the DragAndDropTarget interface, and allow our component
        // to accept drag-and-drop of objects from other DRX components..

        b8 isInterestedInDragSource (const SourceDetails& /*dragSourceDetails*/) override
        {
            // normally you'd check the sourceDescription value to see if it's the
            // sort of object that you're interested in before returning true, but for
            // the demo, we'll say yes to anything..
            return true;
        }

        z0 itemDragEnter (const SourceDetails& /*dragSourceDetails*/) override
        {
            somethingIsBeingDraggedOver = true;
            repaint();
        }

        z0 itemDragMove (const SourceDetails& /*dragSourceDetails*/) override
        {
        }

        z0 itemDragExit (const SourceDetails& /*dragSourceDetails*/) override
        {
            somethingIsBeingDraggedOver = false;
            repaint();
        }

        z0 itemDropped (const SourceDetails& dragSourceDetails) override
        {
            message = "Items dropped: " + dragSourceDetails.description.toString();

            somethingIsBeingDraggedOver = false;
            repaint();
        }

        //==============================================================================
        // These methods implement the FileDragAndDropTarget interface, and allow our component
        // to accept drag-and-drop of files..

        b8 isInterestedInFileDrag (const StringArray& /*files*/) override
        {
            // normally you'd check these files to see if they're something that you're
            // interested in before returning true, but for the demo, we'll say yes to anything..
            return true;
        }

        z0 fileDragEnter (const StringArray& /*files*/, i32 /*x*/, i32 /*y*/) override
        {
            somethingIsBeingDraggedOver = true;
            repaint();
        }

        z0 fileDragMove (const StringArray& /*files*/, i32 /*x*/, i32 /*y*/) override
        {
        }

        z0 fileDragExit (const StringArray& /*files*/) override
        {
            somethingIsBeingDraggedOver = false;
            repaint();
        }

        z0 filesDropped (const StringArray& files, i32 /*x*/, i32 /*y*/) override
        {
            message = "Files dropped: " + files.joinIntoString ("\n");

            somethingIsBeingDraggedOver = false;
            repaint();
        }

        //==============================================================================
        // These methods implement the TextDragAndDropTarget interface, and allow our component
        // to accept drag-and-drop of text..

        b8 isInterestedInTextDrag (const Txt& /*text*/) override
        {
            return true;
        }

        z0 textDragEnter (const Txt& /*text*/, i32 /*x*/, i32 /*y*/) override
        {
            somethingIsBeingDraggedOver = true;
            repaint();
        }

        z0 textDragMove (const Txt& /*text*/, i32 /*x*/, i32 /*y*/) override
        {
        }

        z0 textDragExit (const Txt& /*text*/) override
        {
            somethingIsBeingDraggedOver = false;
            repaint();
        }

        z0 textDropped (const Txt& text, i32 /*x*/, i32 /*y*/) override
        {
            message = "Text dropped:\n" + text;

            somethingIsBeingDraggedOver = false;
            repaint();
        }

    private:
        Txt message  { "Drag-and-drop some rows from the top-left box onto this component!\n\n"
                          "You can also drag-and-drop files and text from other apps"};
        b8 somethingIsBeingDraggedOver = false;
    };

    //==============================================================================
    ListBox sourceListBox  { "D+D source", nullptr };
    SourceItemListboxContents sourceModel;
    DragAndDropDemoTarget target;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragAndDropDemo)
};

//==============================================================================
struct DemoTabbedComponent final : public TabbedComponent
{
    DemoTabbedComponent (b8 isRunningComponentTransformsDemo)
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        auto colour = findColor (ResizableWindow::backgroundColorId);

        addTab ("Buttons",     colour, new ButtonsPage (isRunningComponentTransformsDemo), true);
        addTab ("Sliders",     colour, new SlidersPage(),                                 true);
        addTab ("Toolbars",    colour, new ToolbarDemoComp(),                             true);
        addTab ("Misc",        colour, new MiscPage(),                                    true);
        addTab ("Menus",       colour, new MenuPage(),                                    true);
        addTab ("Tables",      colour, new TableDemoComponent(),                          true);
        addTab ("Drag & Drop", colour, new DragAndDropDemo(),                             true);

        getTabbedButtonBar().getTabButton (5)->setExtraComponent (new CustomTabButton (isRunningComponentTransformsDemo),
                                                                  TabBarButton::afterText);
    }

    // This is a small star button that is put inside one of the tabs. You can
    // use this technique to create things like "close tab" buttons, etc.
    class CustomTabButton final : public Component
    {
    public:
        CustomTabButton (b8 isRunningComponentTransformsDemo)
            : runningComponentTransformsDemo (isRunningComponentTransformsDemo)
        {
            setSize (20, 20);
        }

        z0 paint (Graphics& g) override
        {
            Path star;
            star.addStar ({}, 7, 1.0f, 2.0f);

            g.setColor (Colors::green);
            g.fillPath (star, star.getTransformToScaleToFit (getLocalBounds().reduced (2).toFloat(), true));
        }

        z0 mouseDown (const MouseEvent&) override
        {
            showBubbleMessage (*this,
                               "This is a custom tab component\n"
                               "\n"
                               "You can use these to implement things like close-buttons "
                               "or status displays for your tabs.",
                               bubbleMessage,
                               runningComponentTransformsDemo);
        }
    private:
        b8 runningComponentTransformsDemo;
        std::unique_ptr<BubbleMessageComponent> bubbleMessage;
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoTabbedComponent)
};

//==============================================================================
struct WidgetsDemo final : public Component
{
    WidgetsDemo (b8 isRunningComponenTransformsDemo = false)
        : tabs (isRunningComponenTransformsDemo)
    {
        setOpaque (true);
        addAndMakeVisible (tabs);

        setSize (700, 500);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::lightgrey);
    }

    z0 resized() override
    {
        tabs.setBounds (getLocalBounds().reduced (4));
    }

    DemoTabbedComponent tabs;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WidgetsDemo)
};

//==============================================================================
z0 showBubbleMessage (Component& targetComponent, const Txt& textToShow,
                        std::unique_ptr<BubbleMessageComponent>& bmc,
                        b8 isRunningComponentTransformDemo)
{
    bmc.reset (new BubbleMessageComponent());

    if (isRunningComponentTransformDemo)
    {
        targetComponent.findParentComponentOfClass<WidgetsDemo>()->addChildComponent (bmc.get());
    }
    else if (Desktop::canUseSemiTransparentWindows())
    {
        bmc->setAlwaysOnTop (true);
        bmc->addToDesktop (0);
    }
    else
    {
        targetComponent.getTopLevelComponent()->addChildComponent (bmc.get());
    }

    AttributedString text (textToShow);
    text.setJustification (Justification::centred);
    text.setColor (targetComponent.findColor (TextButton::textColorOffId));

    bmc->showAt (&targetComponent, text, 2000, true, false);
}
