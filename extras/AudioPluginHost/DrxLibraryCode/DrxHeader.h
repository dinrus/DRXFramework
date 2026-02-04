/*

    IMPORTANT! This file is auto-generated each time you save your
    project - if you alter its contents, your changes may be overwritten!

    This is the header file that your files should include in order to get all the
    DRX library headers. You should avoid including the DRX headers directly in
    your own source files, because that wouldn't pick up the correct configuration
    options for your app.

*/

#pragma once


#include <drx_audio_basics/drx_audio_basics.h>
#include <drx_audio_devices/drx_audio_devices.h>
#include <drx_audio_formats/drx_audio_formats.h>
#include <drx_audio_processors/drx_audio_processors.h>
#include <drx_audio_utils/drx_audio_utils.h>
#include <drx_core/drx_core.h>
#include <drx_cryptography/drx_cryptography.h>
#include <drx_data_structures/drx_data_structures.h>
#include <drx_dsp/drx_dsp.h>
#include <drx_events/drx_events.h>
#include <drx_graphics/drx_graphics.h>
#include <drx_gui_basics/drx_gui_basics.h>
#include <drx_gui_extra/drx_gui_extra.h>
#include <drx_opengl/drx_opengl.h>

#include "BinaryData.h"

#if defined (DRX_PROJUCER_VERSION) && DRX_PROJUCER_VERSION < DRX_VERSION
 /** If you've hit this error then the version of the Projucer that was used to generate this project is
     older than the version of the DRX modules being included. To fix this error, re-save your project
     using the latest version of the Projucer or, if you aren't using the Projucer to manage your project,
     remove the DRX_PROJUCER_VERSION define.
 */
 #error "This project was last saved using an outdated version of the Projucer! Re-save this project with the latest version to fix this error."
#endif

#if ! DONT_SET_USING_DRX_NAMESPACE
 // If your code uses a lot of DRX classes, then this will obviously save you
 // a lot of typing, but can be disabled by setting DONT_SET_USING_DRX_NAMESPACE.
 using namespace drx;
#endif

#if ! DRX_DONT_DECLARE_PROJECTINFO
namespace ProjectInfo
{
    tukk const  projectName    = "AudioPluginHost";
    tukk const  companyName    = "DinrusPro";
    tukk const  versionString  = "1.0.0";
    i32k          versionNumber  = 0x10000;
}
#endif
