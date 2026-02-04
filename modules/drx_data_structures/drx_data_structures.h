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

  ID:                 drx_data_structures
  vendor:             drx
  version:            8.0.7
  name:               DRX data model helper classes
  description:        Classes for undo/redo management, and smart data structures.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_events

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_DATA_STRUCTURES_H_INCLUDED

//==============================================================================
#include <drx_events/drx_events.h>

#include "undomanager/drx_UndoableAction.h"
#include "undomanager/drx_UndoManager.h"
#include "values/drx_Value.h"
#include "values/drx_ValueTree.h"
#include "values/drx_ValueTreeSynchroniser.h"
#include "values/drx_CachedValue.h"
#include "values/drx_ValueTreePropertyWithDefault.h"
#include "app_properties/drx_PropertiesFile.h"
#include "app_properties/drx_ApplicationProperties.h"
