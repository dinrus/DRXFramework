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

  ID:                 drx_cryptography
  vendor:             drx
  version:            8.0.7
  name:               DRX cryptography classes
  description:        Classes for various basic cryptography functions, including RSA, Blowfish, MD5, SHA, etc.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_core

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_CRYPTOGRAPHY_H_INCLUDED

//==============================================================================
#include <drx_core/drx_core.h>

#include "encryption/drx_BlowFish.h"
#include "encryption/drx_Primes.h"
#include "encryption/drx_RSAKey.h"
#include "hashing/drx_MD5.h"
#include "hashing/drx_SHA256.h"
#include "hashing/drx_Whirlpool.h"
