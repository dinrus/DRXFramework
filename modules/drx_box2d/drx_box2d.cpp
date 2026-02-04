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

#ifdef DRX_BOX2D_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#include "drx_box2d.h"

DRX_BEGIN_IGNORE_WARNINGS_MSVC (6011)
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wconversion",
                                     "-Wfloat-conversion",
                                     "-Wfloat-equal",
                                     "-Wmaybe-uninitialized",
                                     "-Wsign-conversion",
                                     "-Wswitch-default",
                                     "-Wswitch-enum",
                                     "-Wunused-but-set-variable",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wsuggest-override")

#include <cstdarg>

using i8   = drx::i8;
using i16  = drx::i16;
using i32  = drx::i32;
using u8  = drx::u8;
using u16 = drx::u16;
using u32 = drx::u32;

#include "box2d/Collision/b2BroadPhase.cpp"
#include "box2d/Collision/b2CollideCircle.cpp"
#include "box2d/Collision/b2CollideEdge.cpp"
#include "box2d/Collision/b2CollidePolygon.cpp"
#include "box2d/Collision/b2Collision.cpp"
#include "box2d/Collision/b2Distance.cpp"
#include "box2d/Collision/b2DynamicTree.cpp"
#include "box2d/Collision/b2TimeOfImpact.cpp"
#include "box2d/Collision/Shapes/b2ChainShape.cpp"
#include "box2d/Collision/Shapes/b2CircleShape.cpp"
#include "box2d/Collision/Shapes/b2EdgeShape.cpp"
#include "box2d/Collision/Shapes/b2PolygonShape.cpp"
#include "box2d/Common/b2BlockAllocator.cpp"
#include "box2d/Common/b2Draw.cpp"
#include "box2d/Common/b2Math.cpp"
#include "box2d/Common/b2Settings.cpp"
#include "box2d/Common/b2StackAllocator.cpp"
#include "box2d/Common/b2Timer.cpp"
#include "box2d/Dynamics/b2Body.cpp"
#include "box2d/Dynamics/b2ContactManager.cpp"
#include "box2d/Dynamics/b2Fixture.cpp"
#include "box2d/Dynamics/b2Island.cpp"
#include "box2d/Dynamics/b2World.cpp"
#include "box2d/Dynamics/b2WorldCallbacks.cpp"
#include "box2d/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
#include "box2d/Dynamics/Contacts/b2CircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2Contact.cpp"
#include "box2d/Dynamics/Contacts/b2ContactSolver.cpp"
#include "box2d/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
#include "box2d/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2PolygonContact.cpp"
#include "box2d/Dynamics/Joints/b2DistanceJoint.cpp"
#include "box2d/Dynamics/Joints/b2FrictionJoint.cpp"
#include "box2d/Dynamics/Joints/b2GearJoint.cpp"
#include "box2d/Dynamics/Joints/b2Joint.cpp"
#include "box2d/Dynamics/Joints/b2MouseJoint.cpp"
#include "box2d/Dynamics/Joints/b2PrismaticJoint.cpp"
#include "box2d/Dynamics/Joints/b2PulleyJoint.cpp"
#include "box2d/Dynamics/Joints/b2RevoluteJoint.cpp"
#include "box2d/Dynamics/Joints/b2RopeJoint.cpp"
#include "box2d/Dynamics/Joints/b2WeldJoint.cpp"
#include "box2d/Dynamics/Joints/b2WheelJoint.cpp"
#include "box2d/Rope/b2Rope.cpp"

#include "utils/drx_Box2DRenderer.cpp"

DRX_END_IGNORE_WARNINGS_GCC_LIKE
DRX_END_IGNORE_WARNINGS_MSVC
