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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 DRX Module Format.md file.


 BEGIN_DRX_MODULE_DECLARATION

  ID:                 drx_box2d
  vendor:             drx
  version:            8.0.7
  name:               DRX wrapper for the Box2D physics engine
  description:        The Box2D physics engine and some utility classes.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_graphics

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_BOX2D_H_INCLUDED

//==============================================================================
#include <drx_graphics/drx_graphics.h>

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                     "-Wdeprecated",
                                     "-Wfloat-equal",
                                     "-Wmaybe-uninitialized",
                                     "-Wshadow-field",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wsuggest-override")

#include <climits>
#include <cfloat>

#include "box2d/Box2D.h"

DRX_END_IGNORE_WARNINGS_GCC_LIKE

#ifndef DOXYGEN // for some reason, Doxygen sees this as a re-definition of Box2DRenderer
 #include "utils/drx_Box2DRenderer.h"
#endif // DOXYGEN
