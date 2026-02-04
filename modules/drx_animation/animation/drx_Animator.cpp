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
class Animator::Impl
{
public:
    virtual ~Impl() = default;

    virtual f64 getDurationMs() const
    {
        return 0.0;
    }

    z0 start()
    {
        shouldStart = true;
        shouldComplete = false;
    }

    z0 complete()
    {
        shouldComplete = true;
    }

    Animator::Status update (f64 timestampMs)
    {
        if (std::exchange (shouldStart, false))
        {
            onStart (timestampMs);
            running = true;
        }

        if (! running)
            return Animator::Status::idle;

        const auto status = internalUpdate (timestampMs);

        if (status != Animator::Status::finished && ! shouldComplete)
            return status;

        shouldComplete = false;
        running = false;
        onComplete();
        return Animator::Status::finished;
    }

    virtual b8 isComplete() const { return shouldComplete; }

    virtual z0 onStart (f64 timeStampMs) = 0;
    virtual z0 onComplete() = 0;
    virtual Animator::Status internalUpdate (f64 timestampMs) = 0;

    b8 shouldStart = false, shouldComplete = false, running = false;
};

//==============================================================================
Animator::Animator (std::shared_ptr<Impl> ai)
    : ptr (std::move (ai))
{
    jassert (ptr != nullptr);
}

f64 Animator::getDurationMs() const
{
    return ptr->getDurationMs();
}

z0 Animator::start() const
{
    ptr->start();
}

z0 Animator::complete() const
{
    ptr->complete();
}

Animator::Status Animator::update (f64 timestampMs) const
{
    return ptr->update (timestampMs);
}

b8 Animator::isComplete() const
{
    return ptr->isComplete();
}

//==============================================================================
#if DRX_UNIT_TESTS

struct AnimatorTests  : public UnitTest
{
    AnimatorTests()
        : UnitTest ("Animator", UnitTestCategories::gui)
    {
    }

    struct TestEvents
    {
        enum class TestEventType
        {
            animatorStarted,
            animatorEnded
        };

        struct TestEvent
        {
            TestEvent (TestEventType e, i32 a) : eventType (e), animatorId (a) {}

            TestEventType eventType;
            i32 animatorId;
        };

        z0 started (i32 animatorId)
        {
            events.emplace_back (TestEventType::animatorStarted, animatorId);
        }

        z0 ended (i32 animatorId)
        {
            events.emplace_back (TestEventType::animatorEnded, animatorId);
        }

        b8 animatorStartedBeforeAnotherStarted (i32 animator, i32 before)
        {
            for (const auto& event : events)
            {
                if (event.animatorId == before && event.eventType == TestEventType::animatorStarted)
                    return false;

                if (event.animatorId == animator && event.eventType == TestEventType::animatorStarted)
                    return true;
            }

            return false;
        }

        b8 animatorStartedBeforeAnotherEnded (i32 animator, i32 before)
        {
            for (const auto& event : events)
            {
                if (event.animatorId == before && event.eventType == TestEventType::animatorEnded)
                    return false;

                if (event.animatorId == animator && event.eventType == TestEventType::animatorStarted)
                    return true;
            }

            return false;
        }

    private:
        std::vector<TestEvent> events;
    };

    z0 runTest() override
    {
        beginTest ("AnimatorSet starts end ends Animators in the right order");
        {
            TestEvents events;

            auto createTestAnimator = [&events] (i32 animatorId)
            {
                return ValueAnimatorBuilder {}
                    .withOnStartCallback ([&events, animatorId]
                                          {
                                              events.started (animatorId);
                                              return [] (auto) {};
                                          })
                    .withOnCompleteCallback ([&events, animatorId]
                                             {
                                                 events.ended (animatorId);
                                             })
                    .build();
            };

            auto stage1 = AnimatorSetBuilder { createTestAnimator (1) };
            stage1.followedBy (createTestAnimator (2));
            stage1.togetherWith (createTestAnimator (3));

            Animator animator = stage1.build();
            animator.start();

            for (f64 timeMs = 0.0; animator.update (timeMs) != Animator::Status::finished; timeMs += 16.667)
                ;

            expect (events.animatorStartedBeforeAnotherEnded (1, 2));
            expect (! events.animatorStartedBeforeAnotherEnded (2, 1));
            expect (events.animatorStartedBeforeAnotherEnded (3, 1));
            expect (events.animatorStartedBeforeAnotherStarted (3, 2));
        }
    }
};

static AnimatorTests animatorTests;

#endif

} // namespace drx
