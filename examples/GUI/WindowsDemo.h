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

 name:             WindowsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Displays various types of windows.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        WindowsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Just a simple window that deletes itself when closed. */
class BasicWindow final : public DocumentWindow
{
public:
    BasicWindow (const Txt& name, Color backgroundColor, i32 buttonsNeeded)
        : DocumentWindow (name, backgroundColor, buttonsNeeded)
    {}

    z0 closeButtonPressed() override
    {
        delete this;
    }

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicWindow)
};

//==============================================================================
/** This window contains a ColorSelector which can be used to change the window's colour. */
class ColorSelectorWindow final : public DocumentWindow,
                                   private ChangeListener
{
public:
    ColorSelectorWindow (const Txt& name, Color backgroundColor, i32 buttonsNeeded)
        : DocumentWindow (name, backgroundColor, buttonsNeeded)
    {
        selector.setCurrentColor (backgroundColor);
        selector.setColor (ColorSelector::backgroundColorId, Colors::transparentWhite);
        selector.addChangeListener (this);
        setContentOwned (&selector, false);
    }

    ~ColorSelectorWindow() override
    {
        selector.removeChangeListener (this);
    }

    z0 closeButtonPressed() override
    {
        delete this;
    }

private:
    ColorSelector selector  { ColorSelector::showColorAtTop
                             | ColorSelector::showSliders
                             | ColorSelector::showColorspace };

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &selector)
            setBackgroundColor (selector.getCurrentColor());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorSelectorWindow)
};

//==============================================================================
class BouncingBallComponent final : public Component,
                                    public Timer
{
public:
    BouncingBallComponent()
    {
        setInterceptsMouseClicks (false, false);

        Random random;

        auto size = 10.0f + (f32) random.nextInt (30);

        ballBounds.setBounds (random.nextFloat() * 100.0f,
                              random.nextFloat() * 100.0f,
                              size, size);

        direction.x = random.nextFloat() * 8.0f - 4.0f;
        direction.y = random.nextFloat() * 8.0f - 4.0f;

        colour = Color ((drx::u32) random.nextInt())
                    .withAlpha (0.5f)
                    .withBrightness (0.7f);

        startTimer (60);
    }

    z0 paint (Graphics& g) override
    {
        g.setColor (colour);
        g.fillEllipse (ballBounds - getPosition().toFloat());
    }

    z0 timerCallback() override
    {
        ballBounds += direction;

        if (ballBounds.getX() < 0)                              direction.x =  std::abs (direction.x);
        if (ballBounds.getY() < 0)                              direction.y =  std::abs (direction.y);
        if (ballBounds.getRight()  > (f32) getParentWidth())  direction.x = -std::abs (direction.x);
        if (ballBounds.getBottom() > (f32) getParentHeight()) direction.y = -std::abs (direction.y);

        setBounds (ballBounds.getSmallestIntegerContainer());
    }

private:
    Color colour;
    Rectangle<f32> ballBounds;
    Point<f32> direction;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBallComponent)
};

//==============================================================================
class BouncingBallsContainer final : public Component
{
public:
    BouncingBallsContainer (i32 numBalls)
    {
        for (i32 i = 0; i < numBalls; ++i)
        {
            auto* newBall = new BouncingBallComponent();
            balls.add (newBall);
            addAndMakeVisible (newBall);
        }
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        dragger.startDraggingComponent (this, e);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        // as there's no titlebar we have to manage the dragging ourselves
        dragger.dragComponent (this, e, nullptr);
    }

    z0 paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (Colors::white);
        else
            g.fillAll (Colors::blue.withAlpha (0.2f));

        g.setFont (16.0f);
        g.setColor (Colors::black);
        g.drawFittedText ("This window has no titlebar and a transparent background.",
                          getLocalBounds().reduced (8, 0),
                          Justification::centred, 5);

        g.drawRect (getLocalBounds());
    }

private:
    ComponentDragger dragger;
    OwnedArray<BouncingBallComponent> balls;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBallsContainer)
};

//==============================================================================
class WindowsDemo final : public Component
{
public:
    enum Windows
    {
        dialog,
        document,
        alert,
        numWindows
    };

    WindowsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (showWindowsButton);
        showWindowsButton.onClick = [this] { showAllWindows(); };

        addAndMakeVisible (closeWindowsButton);
        closeWindowsButton.onClick = [this] { closeAllWindows(); };

        addAndMakeVisible (alertWindowResult);
        alertWindowResult.setJustificationType (Justification::centred);

        setSize (250, 250);
    }

    ~WindowsDemo() override
    {
        if (dialogWindow != nullptr)
        {
            dialogWindow->exitModalState (0);

            // we are shutting down: can't wait for the message manager
            // to eventually delete this
            delete dialogWindow;
        }

        closeAllWindows();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Colors::grey));
    }

    z0 resized() override
    {
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.justifyContent = FlexBox::JustifyContent::center;

        constexpr auto buttonWidth = 108.0f;
        constexpr auto componentHeight = 24.0f;
        constexpr auto gap = 4.0f;

        flexBox.items.add (FlexItem { showWindowsButton }.withHeight (componentHeight)
                                                         .withMinWidth (buttonWidth)
                                                         .withAlignSelf (FlexItem::AlignSelf::center));

        flexBox.items.add (FlexItem{}.withHeight (gap));
        flexBox.items.add (FlexItem { closeWindowsButton }.withHeight (componentHeight)
                                                          .withMinWidth (buttonWidth)
                                                          .withAlignSelf (FlexItem::AlignSelf::center));

        flexBox.items.add (FlexItem{}.withHeight (gap));
        flexBox.items.add (FlexItem { alertWindowResult }.withHeight (componentHeight));

        flexBox.performLayout (getLocalBounds());
    }

private:
    // Because in this demo the windows delete themselves, we'll use the
    // Component::SafePointer class to point to them, which automatically becomes
    // null when the component that it points to is deleted.
    Array<Component::SafePointer<Component>> windows;
    SafePointer<DialogWindow> dialogWindow;

    TextButton showWindowsButton   { "Show Windows" },
               closeWindowsButton  { "Close Windows" };
    Label alertWindowResult { "Alert Window result" };

    z0 showAllWindows()
    {
        closeAllWindows();

        showDocumentWindow (false);
        showDocumentWindow (true);
        showTransparentWindow();
        showAlertWindow();
        showDialogWindow();
    }

    z0 closeAllWindows()
    {
        for (auto& window : windows)
            window.deleteAndZero();

        windows.clear();
        alertWindowResult.setText ("", dontSendNotification);
    }

    static auto getDisplayArea()
    {
        return Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced (20);
    }

    z0 showDialogWindow()
    {
        Txt m;

        m << "Dialog Windows can be used to quickly show a component, usually blocking mouse input to other windows." << newLine
          << newLine
          << "They can also be quickly closed with the escape key, try it now.";

        DialogWindow::LaunchOptions options;
        auto* label = new Label();
        label->setText (m, dontSendNotification);
        label->setColor (Label::textColorId, Colors::whitesmoke);
        options.content.setOwned (label);

        Rectangle<i32> area (0, 0, 300, 200);

        options.content->setSize (area.getWidth(), area.getHeight());

        options.dialogTitle                   = "Dialog Window";
        options.dialogBackgroundColor        = Color (0xff0e345a);
        options.escapeKeyTriggersCloseButton  = true;
        options.useNativeTitleBar             = false;
        options.resizable                     = true;

        dialogWindow = options.launchAsync();

        if (dialogWindow != nullptr)
            dialogWindow->centreWithSize (300, 200);
    }

    z0 showDocumentWindow (b8 native)
    {
        auto* dw = new ColorSelectorWindow ("Document Window", getRandomBrightColor(), DocumentWindow::allButtons);
        windows.add (dw);

        Rectangle<i32> area (0, 0, 300, 400);

        RectanglePlacement placement ((native ? RectanglePlacement::xLeft
                                              : RectanglePlacement::xRight)
                                       | RectanglePlacement::yTop
                                       | RectanglePlacement::doNotResize);

        auto result = placement.appliedTo (area, getDisplayArea());
        dw->setBounds (result);

        dw->setResizable (true, ! native);
        dw->setUsingNativeTitleBar (native);
        dw->setVisible (true);
    }

    z0 showTransparentWindow()
    {
        auto* balls = new BouncingBallsContainer (3);
        balls->addToDesktop (ComponentPeer::windowIsTemporary);
        windows.add (balls);

        Rectangle<i32> area (0, 0, 200, 200);

        RectanglePlacement placement (RectanglePlacement::xLeft
                                       | RectanglePlacement::yBottom
                                       | RectanglePlacement::doNotResize);

        auto result = placement.appliedTo (area, getDisplayArea());
        balls->setBounds (result);

        balls->setVisible (true);
    }

    z0 showAlertWindow()
    {
        auto* alertWindow = new AlertWindow ("Alert Window",
                                             "For more complex dialogs, you can easily add components to an AlertWindow, such as...",
                                             MessageBoxIconType::InfoIcon);
        windows.add (alertWindow);

        alertWindow->addTextBlock ("Text block");
        alertWindow->addComboBox ("Combo box", {"Combo box", "Item 2", "Item 3"});
        alertWindow->addTextEditor ("Text editor", "Text editor");
        alertWindow->addTextEditor ("Password", "password", "including for passwords", true);
        alertWindowCustomComponent.emplace();
        alertWindow->addCustomComponent (&(*alertWindowCustomComponent));
        alertWindow->addTextBlock ("Progress bar");
        alertWindow->addProgressBarComponent (alertWindowCustomComponent->value, ProgressBar::Style::linear);
        alertWindow->addProgressBarComponent (alertWindowCustomComponent->value, ProgressBar::Style::circular);
        alertWindow->addTextBlock ("Press any button, or the escape key, to close the window");

        enum AlertWindowResult
        {
            noButtonPressed,
            button1Pressed,
            button2Pressed
        };

        alertWindow->addButton ("Button 1", AlertWindowResult::button1Pressed);
        alertWindow->addButton ("Button 2", AlertWindowResult::button2Pressed);

        RectanglePlacement placement { RectanglePlacement::yMid
                                       | RectanglePlacement::xLeft
                                       | RectanglePlacement::doNotResize };

        alertWindow->setBounds (placement.appliedTo (alertWindow->getBounds(), getDisplayArea()));

        alertWindowResult.setText ("", dontSendNotification);
        alertWindow->enterModalState (false, ModalCallbackFunction::create ([ref = SafePointer { this }] (i32 result)
        {
            if (ref == nullptr)
                return;

            const auto text = [&]
            {
                switch (result)
                {
                    case noButtonPressed:
                        return "Dismissed the Alert Window without pressing a button";
                    case button1Pressed:
                        return "Dismissed the Alert Window using Button 1";
                    case button2Pressed:
                        return "Dismissed the Alert Window using Button 2";
                }

                return "Unhandled event when dismissing the Alert Window";
            }();

            ref->alertWindowResult.setText (text, dontSendNotification);
        }), true);
    }

    class AlertWindowCustomComponent final : public Component,
                                             private Slider::Listener
    {
    public:
        AlertWindowCustomComponent()
        {
            slider.setRange (0.0, 1.0);
            slider.setValue (0.5, NotificationType::dontSendNotification);
            slider.addListener (this);

            addAndMakeVisible (label);
            addAndMakeVisible (slider);

            setSize (200, 50);
        }

        ~AlertWindowCustomComponent() override
        {
            slider.removeListener (this);
        }

        z0 resized() override
        {
            auto bounds =  getLocalBounds();
            label.setBounds (bounds.removeFromTop (getHeight() / 2));
            slider.setBounds (bounds);
        }

        z0 sliderValueChanged (Slider*) override
        {
            value = slider.getValue();
        }

        f64 value { -1.0 };

    private:
        Label label { "Label", "Custom component" };
        Slider slider { Slider::SliderStyle::LinearHorizontal,
                        Slider::TextEntryBoxPosition::NoTextBox };
    };

    std::optional<AlertWindowCustomComponent> alertWindowCustomComponent;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsDemo)
};
