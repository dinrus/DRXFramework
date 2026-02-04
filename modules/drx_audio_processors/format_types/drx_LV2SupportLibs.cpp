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

#include "drx_lv2_config.h"

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wc99-extensions",
                                     "-Wcast-align",
                                     "-Wconversion",
                                     "-Wextra-semi",
                                     "-Wfloat-conversion",
                                     "-Wfloat-equal",
                                     "-Wformat-overflow",
                                     "-Wimplicit-f32-conversion",
                                     "-Wimplicit-i32-conversion",
                                     "-Wmicrosoft-include",
                                     "-Wmissing-field-initializers",
                                     "-Wnullability-extension",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Wparentheses",
                                     "-Wpedantic",
                                     "-Wredundant-decls",
                                     "-Wshorten-64-to-32",
                                     "-Wsign-conversion",
                                     "-Wswitch-enum",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant")
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4100 4200 4244 4267 4389 4702 4706 4800 6308 28182 28183 6385 6386 6387 6011 6282 6323 6330 6001 6031)
DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

extern "C"
{

#include <math.h>

#define is_windows_path serd_is_windows_path

#include "serd/src/base64.c"
#include "serd/src/byte_source.c"
#include "serd/src/env.c"
#include "serd/src/n3.c"
#undef TRY

#include "serd/src/node.c"
#include "serd/src/reader.c"
#include "serd/src/string.c"
#include "serd/src/system.c"
#include "serd/src/uri.c"
#include "serd/src/writer.c"

#undef is_windows_path

#include "sord/src/sord.c"
#include "sord/src/syntax.c"

#include "lilv/src/collections.c"
#include "lilv/src/filesystem.c"
#include "lilv/src/instance.c"
#include "lilv/src/lib.c"
#include "lilv/src/node.c"
#include "lilv/src/plugin.c"
#include "lilv/src/pluginclass.c"
#include "lilv/src/port.c"
#include "lilv/src/query.c"
#include "lilv/src/scalepoint.c"
#include "lilv/src/state.c"
#include "lilv/src/ui.c"
#include "lilv/src/util.c"
#include "lilv/src/world.c"
#include "lilv/src/zix/tree.c"

#undef NS_RDF
#undef NS_XSD
#undef USTR

#define read_object sratom_read_object
#define read_literal sratom_read_literal

#pragma push_macro ("nil")
#undef nil
#include "LV2_SDK/sratom/src/sratom.c"
#pragma pop_macro ("nil")

} // extern "C"

DRX_END_IGNORE_DEPRECATION_WARNINGS
DRX_END_IGNORE_WARNINGS_MSVC
DRX_END_IGNORE_WARNINGS_GCC_LIKE
