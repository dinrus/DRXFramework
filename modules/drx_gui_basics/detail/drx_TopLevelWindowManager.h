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

namespace drx::detail
{

/* Keeps track of the active top level window. */
class TopLevelWindowManager  : private Timer,
                               private DeletedAtShutdown
{
public:
    TopLevelWindowManager() = default;

    ~TopLevelWindowManager() override
    {
        clearSingletonInstance();
    }

    DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (TopLevelWindowManager)

    static z0 checkCurrentlyFocusedTopLevelWindow()
    {
        if (auto* wm = TopLevelWindowManager::getInstanceWithoutCreating())
            wm->checkFocusAsync();
    }

    z0 checkFocusAsync()
    {
        startTimer (10);
    }

    z0 checkFocus()
    {
        startTimer (jmin (1731, getTimerInterval() * 2));

        auto* newActive = findCurrentlyActiveWindow();

        if (newActive != currentActive)
        {
            currentActive = newActive;

            for (i32 i = windows.size(); --i >= 0;)
                if (auto* tlw = windows[i])
                    tlw->setWindowActive (isWindowActive (tlw));

            Desktop::getInstance().triggerFocusCallback();
        }
    }

    b8 addWindow (TopLevelWindow* const w)
    {
        windows.add (w);
        checkFocusAsync();

        return isWindowActive (w);
    }

    z0 removeWindow (TopLevelWindow* const w)
    {
        checkFocusAsync();

        if (currentActive == w)
            currentActive = nullptr;

        windows.removeFirstMatchingValue (w);

        if (windows.isEmpty())
            deleteInstance();
    }

    Array<TopLevelWindow*> windows;

private:
    TopLevelWindow* currentActive = nullptr;

    z0 timerCallback() override
    {
        checkFocus();
    }

    b8 isWindowActive (TopLevelWindow* const tlw) const
    {
        return (tlw == currentActive
                || tlw->isParentOf (currentActive)
                || tlw->hasKeyboardFocus (true))
               && tlw->isShowing();
    }

    TopLevelWindow* findCurrentlyActiveWindow() const
    {
        if (Process::isForegroundProcess())
        {
            auto* focusedComp = Component::getCurrentlyFocusedComponent();
            auto* w = dynamic_cast<TopLevelWindow*> (focusedComp);

            if (w == nullptr && focusedComp != nullptr)
                w = focusedComp->findParentComponentOfClass<TopLevelWindow>();

            if (w == nullptr)
                w = currentActive;

            if (w != nullptr && w->isShowing())
                return w;
        }

        return nullptr;
    }

    DRX_DECLARE_NON_COPYABLE (TopLevelWindowManager)
};

} // namespace drx::detail
