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

  ID:                 drx_build_tools
  vendor:             drx
  version:            8.0.7
  name:               DRX Build Tools
  description:        Classes for generating intermediate files for DRX projects.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial

  dependencies:       drx_gui_basics

 END_DRX_MODULE_DECLARATION

*******************************************************************************/

// This module is shared by juceaide and the Projucer, but should not be
// considered 'public'. That is, its API, functionality, and contents (and
// existence!) may change between releases without warning.

#pragma once
#define DRX_BUILD_TOOLS_H_INCLUDED

#include <drx_gui_basics/drx_gui_basics.h>

#include "utils/drx_ProjectType.h"
#include "utils/drx_BuildHelperFunctions.h"
#include "utils/drx_BinaryResourceFile.h"
#include "utils/drx_RelativePath.h"
#include "utils/drx_Icons.h"
#include "utils/drx_PlistOptions.h"
#include "utils/drx_ResourceFileHelpers.h"
#include "utils/drx_ResourceRc.h"
#include "utils/drx_VersionNumbers.h"
#include "utils/drx_Entitlements.h"
