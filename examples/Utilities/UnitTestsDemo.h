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

 name:             UnitTestsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Performs unit tests.

 dependencies:     drx_analytics, drx_audio_basics, drx_audio_devices,
                   drx_audio_formats, drx_audio_processors, drx_audio_utils,
                   drx_core, drx_cryptography, drx_data_structures, drx_dsp,
                   drx_events, drx_graphics, drx_gui_basics, drx_gui_extra,
                   drx_opengl, drx_osc, drx_product_unlocking, drx_video,
                   drx_midi_ci
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1,DRX_PLUGINHOST_VST3=1,DRX_PLUGINHOST_LV2=1
 defines:          DRX_UNIT_TESTS=1

 type:             Component
 mainClass:        UnitTestsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class UnitTestsDemo final : public Component
{
public:
    UnitTestsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (startTestButton);
        startTestButton.onClick = [this] { start(); };

        addAndMakeVisible (testResultsBox);
        testResultsBox.setMultiLine (true);
        testResultsBox.setFont (FontOptions (Font::getDefaultMonospacedFontName(), 12.0f, Font::plain));

        addAndMakeVisible (categoriesBox);
        categoriesBox.addItem ("All Tests", 1);

        auto categories = UnitTest::getAllCategories();
        categories.sort (true);

        categoriesBox.addItemList (categories, 2);
        categoriesBox.setSelectedId (1);

        logMessage ("This panel runs the built-in DRX unit-tests from the selected category.\n");
        logMessage ("To add your own unit-tests, see the DRX_UNIT_TESTS macro.");

        setSize (500, 500);
    }

    ~UnitTestsDemo() override
    {
        stopTest();
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Colors::grey));
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds().reduced (6);

        auto topSlice = bounds.removeFromTop (25);
        startTestButton.setBounds (topSlice.removeFromLeft (200));
        topSlice.removeFromLeft (10);
        categoriesBox  .setBounds (topSlice.removeFromLeft (250));

        bounds.removeFromTop (5);
        testResultsBox.setBounds (bounds);
    }

    z0 start()
    {
        startTest (categoriesBox.getText());
    }

    z0 startTest (const Txt& category)
    {
        testResultsBox.clear();
        startTestButton.setEnabled (false);

        currentTestThread.reset (new TestRunnerThread (*this, category));
        currentTestThread->startThread();
    }

    z0 stopTest()
    {
        if (currentTestThread.get() != nullptr)
        {
            currentTestThread->stopThread (15000);
            currentTestThread.reset();
        }
    }

    z0 logMessage (const Txt& message)
    {
        testResultsBox.moveCaretToEnd();
        testResultsBox.insertTextAtCaret (message + newLine);
        testResultsBox.moveCaretToEnd();
    }

    z0 testFinished()
    {
        stopTest();
        startTestButton.setEnabled (true);
        logMessage (newLine + "*** Tests finished ***");
    }

private:
    //==============================================================================
    class TestRunnerThread final : public Thread,
                                   private Timer
    {
    public:
        TestRunnerThread (UnitTestsDemo& utd, const Txt& ctg)
            : Thread ("Unit Tests"),
              owner (utd),
              category (ctg)
        {}

        z0 run() override
        {
            CustomTestRunner runner (*this);

            if (category == "All Tests")
                runner.runAllTests();
            else
                runner.runTestsInCategory (category);

            startTimer (50); // when finished, start the timer which will
                             // wait for the thread to end, then tell our component.
        }

        z0 logMessage (const Txt& message)
        {
            WeakReference<UnitTestsDemo> safeOwner (&owner);

            MessageManager::callAsync ([=]
            {
                if (auto* o = safeOwner.get())
                    o->logMessage (message);
            });
        }

        z0 timerCallback() override
        {
            if (! isThreadRunning())
                owner.testFinished(); // inform the demo page when done, so it can delete this thread.
        }

    private:
        //==============================================================================
        // This subclass of UnitTestRunner is used to redirect the test output to our
        // TextBox, and to interrupt the running tests when our thread is asked to stop..
        class CustomTestRunner final : public UnitTestRunner
        {
        public:
            CustomTestRunner (TestRunnerThread& trt)  : owner (trt) {}

            z0 logMessage (const Txt& message) override
            {
                owner.logMessage (message);
            }

            b8 shouldAbortTests() override
            {
                return owner.threadShouldExit();
            }

        private:
            TestRunnerThread& owner;

            DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTestRunner)
        };

        UnitTestsDemo& owner;
        const Txt category;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestRunnerThread)
    };
    std::unique_ptr<TestRunnerThread> currentTestThread;

    TextButton startTestButton { "Run Unit Tests..." };
    ComboBox categoriesBox;
    TextEditor testResultsBox;

    z0 lookAndFeelChanged() override
    {
        testResultsBox.applyFontToAllText (testResultsBox.getFont());
    }

    DRX_DECLARE_WEAK_REFERENCEABLE (UnitTestsDemo)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnitTestsDemo)
};
