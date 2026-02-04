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
class DelayAnimator : public Animator::Impl
{
public:
    f64 getDurationMs() const override { return delayMs; }

    static Animator build (f64 delayMsIn, std::function<z0()> callbackIn = nullptr)
    {
        return Animator { rawToUniquePtr (new DelayAnimator (delayMsIn, std::move (callbackIn))) };
    }

private:
    DelayAnimator (f64 delayMsIn, std::function<z0()> callbackIn)
        : onCompletion (std::move (callbackIn)),
          delayMs (delayMsIn)
    {
    }

    Animator::Status internalUpdate (f64 timestampMs) override
    {
        if (timestampMs - startedAtMs >= delayMs)
            return Animator::Status::finished;

        return Animator::Status::inProgress;
    }

    z0 onStart (f64 timeMs) override { startedAtMs = timeMs; }

    z0 onComplete() override { NullCheckedInvocation::invoke (onCompletion); }

    std::function<z0()> onCompletion;
    f64 startedAtMs = 0.0, delayMs = 0.0;
};

//==============================================================================
struct AnimatorSetData
{
    explicit AnimatorSetData (Animator root)
        : roots { root }, entries { { root, {} } } {}

    struct Entry
    {
        std::optional<Animator> parent; // If no parent, this is the root node
        std::set<Animator, Animator::Compare> children;
    };

    auto getRoots() const { return roots; }

    auto getChildren (const Animator& a) const
    {
        if (const auto iter = entries.find (a); iter != entries.end())
            return iter->second.children;

        return std::set<Animator, Animator::Compare>();
    }

    std::set<Animator, Animator::Compare> roots;
    std::map<Animator, Entry, Animator::Compare> entries;
    std::function<f64 (f64)> timeTransform;
};

class AnimatorSet  : public Animator::Impl
{
public:
    explicit AnimatorSet (AnimatorSetData dataIn) : data (std::move (dataIn)) {}

    f64 getDurationMs() const override
    {
        const auto roots = data.getRoots();
        return getMaxDuration (roots.begin(), roots.end());
    }

private:
    z0 onStart (f64 timestampMs) override
    {
        startedAtMs = timestampMs;
        active = data.getRoots();

        for (const auto& i : active)
            i.start();
    }

    z0 onComplete() override {}

    Animator::Status internalUpdate (f64 timestampMs) override
    {
        const auto internalTimestampMs = [&]
        {
            if (data.timeTransform == nullptr)
                return timestampMs;

            return data.timeTransform (timestampMs - startedAtMs);
        }();

        if (isComplete())
        {
            for (auto i : active)
                i.complete();

            while (updateAnimatorSet (internalTimestampMs) != Animator::Status::finished)
            {
            }

            return Animator::Status::finished;
        }

        return updateAnimatorSet (internalTimestampMs);
    }

    Animator::Status updateAnimatorSet (f64 timestampMs)
    {
        std::set<Animator, Animator::Compare> animatorsToRemove;
        const auto currentlyActive = active;

        for (const auto& animator : currentlyActive)
        {
            const auto status = animator.update (timestampMs);

            if (status == Animator::Status::finished)
            {
                animatorsToRemove.insert (animator);

                for (const auto& j : data.getChildren (animator))
                {
                    j.start();

                    if (isComplete())
                        j.complete();

                    active.insert (j);
                }
            }
        }

        for (auto i : animatorsToRemove)
            active.erase (i);

        return active.empty() ? Animator::Status::finished : Animator::Status::inProgress;
    }

    template <typename It>
    f64 getMaxDuration (It begin, It end) const
    {
        return std::accumulate (begin, end, 0.0, [this] (const auto acc, const auto& anim)
        {
            const auto children = data.getChildren (anim);
            return std::max (acc, anim.getDurationMs() + getMaxDuration (children.begin(), children.end()));
        });
    }

    const AnimatorSetData data;
    std::set<Animator, Animator::Compare> active;
    f64 startedAtMs = 0.0;
};

//==============================================================================
struct AnimatorSetBuilder::AnimatorSetBuilderState
{
    explicit AnimatorSetBuilderState (Animator animator)
        : data (std::move (animator)) {}

    b8 valid = true;
    AnimatorSetData data;
};

AnimatorSetBuilder::AnimatorSetBuilder (Animator startingAnimator)
    : AnimatorSetBuilder (startingAnimator,
                          std::make_shared<AnimatorSetBuilderState> (startingAnimator))
{}

AnimatorSetBuilder::AnimatorSetBuilder (f64 delayMs)
    : AnimatorSetBuilder (DelayAnimator::build (delayMs, nullptr))
{}

AnimatorSetBuilder::AnimatorSetBuilder (std::function<z0()> cb)
    : AnimatorSetBuilder (DelayAnimator::build (0.0, std::move (cb)))
{}

AnimatorSetBuilder::AnimatorSetBuilder (std::shared_ptr<AnimatorSetBuilderState> dataIn)
    : cursor (*dataIn->data.getRoots().begin()), state (std::move (dataIn))
{}

AnimatorSetBuilder::AnimatorSetBuilder (Animator cursorIn, std::shared_ptr<AnimatorSetBuilderState> dataIn)
    : cursor (cursorIn), state (std::move (dataIn))
{}

AnimatorSetBuilder AnimatorSetBuilder::togetherWith (Animator animator)
{
    add (state->data.entries.at (cursor).parent, animator);
    return { animator, state };
}

AnimatorSetBuilder AnimatorSetBuilder::followedBy (Animator animator)
{
    add (cursor, animator);
    return { animator, state };
}

z0 AnimatorSetBuilder::add (std::optional<Animator> parent, Animator child)
{
    state->data.entries.emplace (child, AnimatorSetData::Entry { parent, {} });

    if (parent.has_value())
        state->data.entries.at (*parent).children.insert (child);
    else
        state->data.roots.insert (child);
}

Animator AnimatorSetBuilder::build()
{
    if (state == nullptr)
    {
        /*  If you're hitting this assertion, you've already used this AnimatorSetBuilder to build an
            AnimatorSet.

            To create another AnimatorSet you need to use another AnimatorSetBuilder independently
            created with the AnimatorSetBuilder (Animator) constructor.
        */
        jassertfalse;
        return ValueAnimatorBuilder{}.build();
    }

    auto animator = Animator { std::make_unique<AnimatorSet> (std::move (state->data)) };
    state = nullptr;
    return animator;
}

AnimatorSetBuilder AnimatorSetBuilder::followedBy (f64 delayMs)
{
    return followedBy (DelayAnimator::build (delayMs, nullptr));
}

AnimatorSetBuilder AnimatorSetBuilder::followedBy (std::function<z0()> cb)
{
    return followedBy (DelayAnimator::build (0.0, std::move (cb)));
}

AnimatorSetBuilder AnimatorSetBuilder::togetherWith (f64 delayMs)
{
    return togetherWith (DelayAnimator::build (delayMs, nullptr));
}

AnimatorSetBuilder AnimatorSetBuilder::togetherWith (std::function<z0()> cb)
{
    return togetherWith (DelayAnimator::build (0.0, std::move (cb)));
}

AnimatorSetBuilder AnimatorSetBuilder::withTimeTransform (std::function<f64 (f64)> timeTransformIn)
{
    state->data.timeTransform = std::move (timeTransformIn);
    return *this;
}

} // namespace drx
