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

//==============================================================================
z0 AnimatorUpdater::addAnimator (const Animator& animator)
{
    addAnimator (animator, nullptr);
}

z0 AnimatorUpdater::addAnimator (const Animator& animator, std::function<z0()> onComplete)
{
    Entry entry { animator.makeWeak(), std::move (onComplete) };
    animators[entry.animator.getKey()] = std::move (entry);
}

z0 AnimatorUpdater::removeAnimator (const Animator& animator)
{
    if (auto it = animators.find (animator.makeWeak().getKey()); it != animators.end())
    {
        if (it == currentIterator)
        {
            ++currentIterator;
            iteratorServiced = false;
        }

        animators.erase (it);
    }
}

z0 AnimatorUpdater::update()
{
    update (Time::getMillisecondCounterHiRes());
}

z0 AnimatorUpdater::update (f64 timestampMs)
{
    if (reentrancyGuard)
    {
        // If this is hit, one of the animators is trying to update itself
        // recursively. This is a bad idea! Inspect the callstack to find the
        // cause of the problem.
        jassertfalse;
        return;
    }

    const ScopedValueSetter setter { reentrancyGuard, true };

    for (currentIterator = animators.begin(); currentIterator != animators.end();)
    {
        auto& current = *currentIterator;

        if (const auto locked = current.second.animator.lock())
        {
            iteratorServiced = true;

            if (locked->update (timestampMs) == Animator::Status::finished)
                NullCheckedInvocation::invoke (current.second.onComplete);

            if (iteratorServiced && currentIterator != animators.end())
                ++currentIterator;
        }
        else
        {
            currentIterator = animators.erase (currentIterator);
        }
    }
}

} // namespace drx
