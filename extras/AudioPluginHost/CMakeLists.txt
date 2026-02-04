# ==============================================================================
#
#  This file is part of the DRX framework.
#  Copyright (c) DinrusPro
#
#  DRX is an open source framework subject to commercial or open source
#  licensing.
#
#  By downloading, installing, or using the DRX framework, or combining the
#  DRX framework with any other source code, object code, content or any other
#  copyrightable work, you agree to the terms of the DRX End User Licence
#  Agreement, and all incorporated terms including the DRX Privacy Policy and
#  the DRX Website Terms of Service, as applicable, which will bind you. If you
#  do not agree to the terms of these agreements, we will not license the DRX
#  framework to you, and you must discontinue the installation or download
#  process and cease use of the DRX framework.
#
#  DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
#  DRX Privacy Policy: https://juce.com/juce-privacy-policy
#  DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/
#
#  Or:
#
#  You may also use this code under the terms of the AGPLv3:
#  https://www.gnu.org/licenses/agpl-3.0.en.html
#
#  THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
#  WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.
#
# ==============================================================================

drx_add_gui_app(AudioPluginHost
    BUNDLE_ID                       com.juce.pluginhost
    ICON_BIG                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCEAppIcon.png"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    PLUGINHOST_AU                   TRUE)

drx_generate_drx_header(AudioPluginHost)

target_sources(AudioPluginHost PRIVATE
    Source/HostStartup.cpp
    Source/Plugins/ARAPlugin.cpp
    Source/Plugins/IOConfigurationWindow.cpp
    Source/Plugins/InternalPlugins.cpp
    Source/Plugins/PluginGraph.cpp
    Source/UI/GraphEditorPanel.cpp
    Source/UI/MainHostWindow.cpp)

drx_add_binary_data(AudioPluginHostData SOURCES
    ../../examples/Assets/cassette_recorder.wav
    ../../examples/Assets/cello.wav
    ../../examples/Assets/guitar_amp.wav
    ../../examples/Assets/proaudio.path
    ../../examples/Assets/reverb_ir.wav
    ../../examples/Assets/singing.ogg)

target_compile_definitions(AudioPluginHost PRIVATE
    JUCE_ALSA=1
    JUCE_CONTENT_SHARING=1
    JUCE_DIRECTSOUND=1
    JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING=1
    JUCE_PLUGINHOST_LADSPA=1
    JUCE_PLUGINHOST_LV2=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_ARA=0
    JUCE_USE_CAMERA=0
    JUCE_USE_CDBURNER=0
    JUCE_USE_CDREADER=0
    JUCE_USE_CURL=0
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=1
    JUCE_VST3_HOST_CROSS_PLATFORM_UID=1
    JUCE_WASAPI=1
    JUCE_WEB_BROWSER=0
    PIP_JUCE_EXAMPLES_DIRECTORY_STRING="${JUCE_SOURCE_DIR}/examples"
    # This is a temporary workaround to allow builds to complete on Xcode 15.
    # Add -Wl,-ld_classic to the OTHER_LDFLAGS build setting if you need to
    # deploy to older versions of macOS/iOS.
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)

target_link_libraries(AudioPluginHost PRIVATE
    AudioPluginHostData
    juce::drx_audio_utils
    juce::drx_cryptography
    juce::drx_dsp
    juce::drx_opengl
    juce::drx_recommended_config_flags
    juce::drx_recommended_lto_flags
    juce::drx_recommended_warning_flags)

drx_add_bundle_resources_directory(AudioPluginHost ../../examples/Assets)
