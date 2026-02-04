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

#ifdef DRX_ANIMATION_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

//==============================================================================
#include "drx_animation.h"

//==============================================================================
namespace chromium
{

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4701 6001)
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-conversion",
                                     "-Wfloat-equal",
                                     "-Wconditional-uninitialized")


#include "detail/chromium/cubic_bezier.h"
#include "detail/chromium/cubic_bezier.cc"

DRX_END_IGNORE_WARNINGS_MSVC
DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace chromium

//==============================================================================
#include "animation/drx_Animator.cpp"
#include "animation/drx_AnimatorSetBuilder.cpp"
#include "animation/drx_AnimatorUpdater.cpp"
#include "animation/drx_Easings.cpp"
#include "animation/drx_ValueAnimatorBuilder.cpp"
