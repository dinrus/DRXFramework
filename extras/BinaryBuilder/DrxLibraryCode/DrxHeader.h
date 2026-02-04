/*

    IMPORTANT! This file is auto-generated each time you save your
    project - if you alter its contents, your changes may be overwritten!

    This is the header file that your files should include in order to get all the
    DRX library headers. You should avoid including the DRX headers directly in
    your own source files, because that wouldn't pick up the correct configuration
    options for your app.

*/

#pragma once


#include <drx_core/drx_core.h>


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
    tukk const  projectName    = "BinaryBuilder";
    tukk const  companyName    = "DinrusPro";
    tukk const  versionString  = "1.0.0";
    i32k          versionNumber  = 0x10000;
}
#endif
