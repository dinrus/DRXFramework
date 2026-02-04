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

#ifdef DRX_AUDIO_PROCESSORS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_GUI_BASICS_INCLUDE_XHEADERS 1
#define DRX_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1
#define DRX_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1

#include "drx_audio_processors.h"
#include <drx_gui_extra/drx_gui_extra.h>

//==============================================================================
#if (DRX_PLUGINHOST_VST || DRX_PLUGINHOST_VST3) && (DRX_LINUX || DRX_BSD)
 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wvariadic-macros")
 #include <X11/Xlib.h>
 DRX_END_IGNORE_WARNINGS_GCC_LIKE
 #include <X11/Xutil.h>
 #include <sys/utsname.h>
 #undef KeyPress
#endif

#if ! DRX_WINDOWS && ! DRX_MAC && ! DRX_LINUX
 #undef DRX_PLUGINHOST_VST3
 #define DRX_PLUGINHOST_VST3 0
#endif

#if DRX_PLUGINHOST_AU && (DRX_MAC || DRX_IOS)
 #include <AudioUnit/AudioUnit.h>
#endif

namespace drx
{

#if DRX_PLUGINHOST_VST || (DRX_PLUGINHOST_LADSPA && (DRX_LINUX || DRX_BSD))

static b8 arrayContainsPlugin (const OwnedArray<PluginDescription>& list,
                                 const PluginDescription& desc)
{
    for (auto* p : list)
        if (p->isDuplicateOf (desc))
            return true;

    return false;
}

#endif

#if DRX_MAC

//==============================================================================
/*  This is an NSViewComponent which holds a i64-lived NSView which acts
    as the parent view for plugin editors.

    Note that this component does not auto-resize depending on the bounds
    of the owned view. VST2 and VST3 plugins have dedicated interfaces to
    request that the editor bounds are updated. We can call `setSize` on this
    component from inside those dedicated callbacks.
*/
struct NSViewComponentWithParent : public NSViewComponent,
                                   private AsyncUpdater
{
    enum class WantsNudge { no, yes };

    explicit NSViewComponentWithParent (WantsNudge shouldNudge)
        : wantsNudge (shouldNudge)
    {
        auto* view = [[getViewClass().createInstance() init] autorelease];
        object_setInstanceVariable (view, "owner", this);
        setView (view);
    }

    explicit NSViewComponentWithParent (AudioPluginInstance& instance)
        : NSViewComponentWithParent (getWantsNudge (instance)) {}

    ~NSViewComponentWithParent() override
    {
        if (auto* view = static_cast<NSView*> (getView()))
            object_setInstanceVariable (view, "owner", nullptr);

        cancelPendingUpdate();
    }

    DRX_DECLARE_NON_COPYABLE (NSViewComponentWithParent)
    DRX_DECLARE_NON_MOVEABLE (NSViewComponentWithParent)

private:
    WantsNudge wantsNudge = WantsNudge::no;

    static WantsNudge getWantsNudge (AudioPluginInstance& instance)
    {
        PluginDescription pd;
        instance.fillInPluginDescription (pd);
        return pd.manufacturerName == "FabFilter" ? WantsNudge::yes : WantsNudge::no;
    }

    z0 handleAsyncUpdate() override
    {
        if (auto* peer = getTopLevelComponent()->getPeer())
        {
            auto* view = static_cast<NSView*> (getView());
            const auto newArea = peer->getAreaCoveredBy (*this);
            [view setFrame: makeCGRect (newArea.withHeight (newArea.getHeight() + 1))];
            [view setFrame: makeCGRect (newArea)];
        }
    }

    struct InnerNSView final : public ObjCClass<NSView>
    {
        InnerNSView()
            : ObjCClass ("DrxInnerNSView_")
        {
            addIvar<NSViewComponentWithParent*> ("owner");

            addMethod (@selector (isOpaque), [] (id, SEL) { return YES; });

            addMethod (@selector (didAddSubview:), [] (id self, SEL, NSView*)
            {
                if (auto* owner = getIvar<NSViewComponentWithParent*> (self, "owner"))
                    if (owner->wantsNudge == WantsNudge::yes)
                        owner->triggerAsyncUpdate();
            });

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (clipsToBounds), [] (id, SEL) { return YES; });
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            registerClass();
        }
    };

    static InnerNSView& getViewClass()
    {
        static InnerNSView result;
        return result;
    }
};

#endif

} // namespace drx

#include "utilities/drx_FlagCache.h"
#include "format/drx_AudioPluginFormat.cpp"
#include "format/drx_AudioPluginFormatManager.cpp"
#include "format_types/drx_LegacyAudioParameter.cpp"
#include "processors/drx_AudioProcessor.cpp"
#include "processors/drx_AudioPluginInstance.cpp"
#include "processors/drx_AudioProcessorEditor.cpp"
#include "processors/drx_AudioProcessorGraph.cpp"
#include "processors/drx_GenericAudioProcessorEditor.cpp"
#include "processors/drx_PluginDescription.cpp"
#include "format_types/drx_ARACommon.cpp"
#include "format_types/drx_LADSPAPluginFormat.cpp"
#include "format_types/drx_VSTPluginFormat.cpp"
#include "format_types/drx_VST3PluginFormat.cpp"
#include "format_types/drx_AudioUnitPluginFormat.mm"
#include "format_types/drx_ARAHosting.cpp"
#include "scanning/drx_KnownPluginList.cpp"
#include "scanning/drx_PluginDirectoryScanner.cpp"
#include "scanning/drx_PluginListComponent.cpp"
#include "processors/drx_AudioProcessorParameterGroup.cpp"
#include "utilities/drx_AudioProcessorParameterWithID.cpp"
#include "utilities/drx_RangedAudioParameter.cpp"
#include "utilities/drx_AudioParameterFloat.cpp"
#include "utilities/drx_AudioParameterInt.cpp"
#include "utilities/drx_AudioParameterBool.cpp"
#include "utilities/drx_AudioParameterChoice.cpp"
#include "utilities/drx_ParameterAttachments.cpp"
#include "utilities/drx_AudioProcessorValueTreeState.cpp"
#include "utilities/drx_PluginHostType.cpp"
#include "utilities/drx_AAXClientExtensions.cpp"
#include "utilities/drx_VST2ClientExtensions.cpp"
#include "utilities/drx_VST3ClientExtensions.cpp"
#include "utilities/ARA/drx_ARA_utils.cpp"

#include "format_types/drx_LV2PluginFormat.cpp"

#if DRX_UNIT_TESTS
 #if DRX_PLUGINHOST_VST3
  #include "format_types/drx_VST3PluginFormat_test.cpp"
 #endif

 #if DRX_PLUGINHOST_LV2 && (! (DRX_ANDROID || DRX_IOS))
  #include "format_types/drx_LV2PluginFormat_test.cpp"
 #endif
#endif
