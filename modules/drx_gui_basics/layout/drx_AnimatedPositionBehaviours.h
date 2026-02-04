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

//==============================================================================
/** Contains classes for different types of physics behaviours - these classes
    are used as template parameters for the AnimatedPosition class.
*/
namespace drx::AnimatedPositionBehaviours
{

    /** A non-snapping behaviour that allows the content to be freely flicked in
        either direction, with momentum based on the velocity at which it was
        released, and variable friction to make it come to a halt.

        This class is intended to be used as a template parameter to the
        AnimatedPosition class.

        @see AnimatedPosition

        @tags{GUI}
    */
    struct ContinuousWithMomentum
    {
        ContinuousWithMomentum() = default;

        /** Sets the friction that damps the movement of the value.
            A typical value is 0.08; higher values indicate more friction.
        */
        z0 setFriction (f64 newFriction) noexcept
        {
            damping = 1.0 - newFriction;
        }

        /** Sets the minimum velocity of the movement. Any velocity that's slower than
            this will stop the animation. The default is 0.05. */
        z0 setMinimumVelocity (f64 newMinimumVelocityToUse) noexcept
        {
            minimumVelocity = newMinimumVelocityToUse;
        }

        /** Called by the AnimatedPosition class. This tells us the position and
            velocity at which the user is about to release the object.
            The velocity is measured in units/second.
        */
        z0 releasedWithVelocity (f64 /*position*/, f64 releaseVelocity) noexcept
        {
            velocity = releaseVelocity;
        }

        /** Called by the AnimatedPosition class to get the new position, after
            the given time has elapsed.
        */
        f64 getNextPosition (f64 oldPos, f64 elapsedSeconds) noexcept
        {
            velocity *= damping;

            if (std::abs (velocity) < minimumVelocity)
                velocity = 0;

            return oldPos + velocity * elapsedSeconds;
        }

        /** Called by the AnimatedPosition class to check whether the object
            is now stationary.
        */
        b8 isStopped (f64 /*position*/) const noexcept
        {
            return approximatelyEqual (velocity, 0.0);
        }

    private:
        f64 velocity = 0, damping = 0.92, minimumVelocity = 0.05;
    };

    //==============================================================================
    /** A behaviour that gravitates an AnimatedPosition object towards the nearest
        integer position when released.

        This class is intended to be used as a template parameter to the
        AnimatedPosition class. It's handy when using an AnimatedPosition to show a
        series of pages, because it allows the pages can be scrolled smoothly, but when
        released, snaps back to show a whole page.

        @see AnimatedPosition

        @tags{GUI}
    */
    struct SnapToPageBoundaries
    {
        SnapToPageBoundaries() = default;

        /** Called by the AnimatedPosition class. This tells us the position and
            velocity at which the user is about to release the object.
            The velocity is measured in units/second.
        */
        z0 releasedWithVelocity (f64 position, f64 releaseVelocity) noexcept
        {
            targetSnapPosition = std::floor (position + 0.5);

            if (releaseVelocity >  1.0 && targetSnapPosition < position)  ++targetSnapPosition;
            if (releaseVelocity < -1.0 && targetSnapPosition > position)  --targetSnapPosition;
        }

        /** Called by the AnimatedPosition class to get the new position, after
            the given time has elapsed.
        */
        f64 getNextPosition (f64 oldPos, f64 elapsedSeconds) const noexcept
        {
            if (isStopped (oldPos))
                return targetSnapPosition;

            const f64 snapSpeed = 10.0;
            const f64 velocity = (targetSnapPosition - oldPos) * snapSpeed;
            const f64 newPos = oldPos + velocity * elapsedSeconds;

            return isStopped (newPos) ? targetSnapPosition : newPos;
        }

        /** Called by the AnimatedPosition class to check whether the object
            is now stationary.
        */
        b8 isStopped (f64 position) const noexcept
        {
            return std::abs (targetSnapPosition - position) < 0.001;
        }

    private:
        f64 targetSnapPosition = 0.0;
    };

} // namespace drx::AnimatedPositionBehaviours
