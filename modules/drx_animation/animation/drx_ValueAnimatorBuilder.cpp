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
class ValueAnimator : public Animator::Impl
{
public:
    explicit ValueAnimator (ValueAnimatorBuilder optionsIn) : options (std::move (optionsIn)) {}

    auto getValue() const
    {
        using namespace detail::ArrayAndTupleOps;

        return options.getEasing() == nullptr ? getProgress() : options.getEasing() (getProgress());
    }

    f32 getProgress() const
    {
        if (isComplete())
            return 1.0f;

        return timeBasedProgress;
    }

    /** Returns the time in milliseconds that it takes for the progress to go from 0.0 to 1.0.

        This is the value returned even if the Animator is infinitely running.
    */
    f64 getDurationMs() const override
    {
        return options.getDurationMs();
    }

    b8 isComplete() const override
    {
        return Animator::Impl::isComplete()
               || (! options.isInfinitelyRunning() && timeBasedProgress >= 1.0f);
    }

private:
    Animator::Status internalUpdate (f64 timestampMs) override
    {
        timeBasedProgress = (f32) ((timestampMs - startedAtMs) / options.getDurationMs());

        NullCheckedInvocation::invoke (onValueChanged, getValue());

        if (! options.isInfinitelyRunning())
            return getProgress() >= 1.0 ? Animator::Status::finished : Animator::Status::inProgress;

        return Animator::Status::inProgress;
    }

    z0 onStart (f64 timeMs) override
    {
        startedAtMs = timeMs;
        timeBasedProgress = 0.0f;

        if (auto fn = options.getOnStartWithValueChanged())
            onValueChanged = fn();
    }

    z0 onComplete() override
    {
        NullCheckedInvocation::invoke (options.getOnComplete());
    }

    f64 startedAtMs = 0.0;
    f32 timeBasedProgress = 0.0f;

    const ValueAnimatorBuilder options;
    ValueAnimatorBuilder::ValueChangedCallback onValueChanged;
};

//==============================================================================
Animator ValueAnimatorBuilder::build() const&  { return Animator { std::make_unique<ValueAnimator> (*this) }; }
Animator ValueAnimatorBuilder::build() &&      { return Animator { std::make_unique<ValueAnimator> (std::move (*this)) }; }

} // namespace drx
