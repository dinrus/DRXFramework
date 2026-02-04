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

 name:             MultithreadingDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Demonstrates multi-threading.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MultithreadingDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class BouncingBall : private ComponentListener
{
public:
    BouncingBall (Component& comp)
        : containerComponent (comp)
    {
        containerComponent.addComponentListener (this);

        auto speed = 5.0f; // give each ball a fixed speed so we can
                           // see the effects of thread priority on how fast
                           // they actually go.

        auto angle = Random::getSystemRandom().nextFloat() * MathConstants<f32>::twoPi;

        dx = std::sin (angle) * speed;
        dy = std::cos (angle) * speed;

        colour = Color ((drx::u32) Random::getSystemRandom().nextInt())
                    .withAlpha (0.5f)
                    .withBrightness (0.7f);

        updateParentSize (comp);

        x = Random::getSystemRandom().nextFloat() * parentWidth;
        y = Random::getSystemRandom().nextFloat() * parentHeight;
    }

    ~BouncingBall() override
    {
        containerComponent.removeComponentListener (this);
    }

    // This will be called from the message thread
    z0 draw (Graphics& g)
    {
        const ScopedLock lock (drawing);

        g.setColor (colour);
        g.fillEllipse (x, y, size, size);

        g.setColor (Colors::black);
        g.setFont (10.0f);
        g.drawText (Txt::toHexString ((z64) threadId), Rectangle<f32> (x, y, size, size), Justification::centred, false);
    }

    z0 moveBall()
    {
        const ScopedLock lock (drawing);

        threadId = Thread::getCurrentThreadId(); // this is so the component can print the thread ID inside the ball

        x += dx;
        y += dy;

        if (x < 0)
            dx = std::abs (dx);

        if (x > parentWidth)
            dx = -std::abs (dx);

        if (y < 0)
            dy = std::abs (dy);

        if (y > parentHeight)
            dy = -std::abs (dy);
    }

private:
    z0 updateParentSize (Component& comp)
    {
        const ScopedLock lock (drawing);

        parentWidth  = (f32) comp.getWidth()  - size;
        parentHeight = (f32) comp.getHeight() - size;
    }

    z0 componentMovedOrResized (Component& comp, b8, b8) override
    {
        updateParentSize (comp);
    }

    f32 x = 0.0f, y = 0.0f,
          size = Random::getSystemRandom().nextFloat() * 30.0f + 30.0f,
          dx = 0.0f, dy = 0.0f,
          parentWidth = 50.0f, parentHeight = 50.0f;

    Color colour;
    Thread::ThreadID threadId = {};
    CriticalSection drawing;

    Component& containerComponent;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBall)
};

//==============================================================================
class DemoThread final : public BouncingBall,
                         public Thread
{
public:
    DemoThread (Component& containerComp)
        : BouncingBall (containerComp),
          Thread ("DRX Demo Thread")
    {
        startThread();
    }

    ~DemoThread() override
    {
        // allow the thread 2 seconds to stop cleanly - should be plenty of time.
        stopThread (2000);
    }

    z0 run() override
    {
        // this is the code that runs this thread - we'll loop continuously,
        // updating the coordinates of our blob.

        // threadShouldExit() returns true when the stopThread() method has been
        // called, so we should check it often, and exit as soon as it gets flagged.
        while (! threadShouldExit())
        {
            // sleep a bit so the threads don't all grind the CPU to a halt..
            wait (interval);

            // because this is a background thread, we mustn't do any UI work without
            // first grabbing a MessageManagerLock..
            const MessageManagerLock mml (Thread::getCurrentThread());

            if (! mml.lockWasGained())  // if something is trying to kill this job, the lock
                return;                 // will fail, in which case we'd better return..

            // now we've got the UI thread locked, we can mess about with the components
            moveBall();
        }
    }

private:
    i32 interval = Random::getSystemRandom().nextInt (50) + 6;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoThread)
};


//==============================================================================
class DemoThreadPoolJob final : public BouncingBall,
                                public ThreadPoolJob
{
public:
    DemoThreadPoolJob (Component& containerComp)
        : BouncingBall (containerComp),
          ThreadPoolJob ("Demo Threadpool Job")
    {}

    JobStatus runJob() override
    {
        // this is the code that runs this job. It'll be repeatedly called until we return
        // jobHasFinished instead of jobNeedsRunningAgain.
        Thread::sleep (30);

        // because this is a background thread, we mustn't do any UI work without
        // first grabbing a MessageManagerLock..
        const MessageManagerLock mml (this);

        // before moving the ball, we need to check whether the lock was actually gained, because
        // if something is trying to stop this job, it will have failed..
        if (mml.lockWasGained())
            moveBall();

        return jobNeedsRunningAgain;
    }

    z0 removedFromQueue()
    {
        // This is called to tell us that our job has been removed from the pool.
        // In this case there's no need to do anything here.
    }

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoThreadPoolJob)
};

//==============================================================================
class MultithreadingDemo final : public Component,
                                 private Timer
{
public:
    //==============================================================================
    MultithreadingDemo()
    {
        setOpaque (true);

        addAndMakeVisible (controlButton);
        controlButton.changeWidthToFitText (24);
        controlButton.setTopLeftPosition (20, 20);
        controlButton.setTriggeredOnMouseDown (true);
        controlButton.setAlwaysOnTop (true);
        controlButton.onClick = [this] { showMenu(); };

        setSize (500, 500);

        resetAllBalls();

        startTimerHz (60);
    }

    ~MultithreadingDemo() override
    {
        pool.removeAllJobs (true, 2000);
    }

    z0 resetAllBalls()
    {
        pool.removeAllJobs (true, 4000);
        balls.clear();

        for (i32 i = 0; i < 5; ++i)
            addABall();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));

        for (auto* ball : balls)
            ball->draw (g);
    }

private:
    //==============================================================================
    z0 setUsingPool (b8 usePool)
    {
        isUsingPool = usePool;
        resetAllBalls();
    }

    z0 addABall()
    {
        if (isUsingPool)
        {
            auto newBall = std::make_unique<DemoThreadPoolJob> (*this);
            pool.addJob (newBall.get(), false);
            balls.add (newBall.release());

        }
        else
        {
            balls.add (new DemoThread (*this));
        }
    }

    z0 timerCallback() override
    {
        repaint();
    }

    z0 showMenu()
    {
        PopupMenu m;
        m.addItem (1, "Use one thread per ball", true, ! isUsingPool);
        m.addItem (2, "Use a thread pool",       true,   isUsingPool);

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (controlButton),
                         ModalCallbackFunction::forComponent (menuItemChosenCallback, this));
    }

    static z0 menuItemChosenCallback (i32 result, MultithreadingDemo* demoComponent)
    {
        if (result != 0 && demoComponent != nullptr)
            demoComponent->setUsingPool (result == 2);
    }

    //==============================================================================
    ThreadPool pool           { ThreadPoolOptions{}.withThreadName ("Demo thread pool")
                                                   .withNumberOfThreads (3) };
    TextButton controlButton  { "Thread type" };
    b8 isUsingPool = false;

    OwnedArray<BouncingBall> balls;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultithreadingDemo)
};
