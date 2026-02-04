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

drx_add_gui_app(AudioPerformanceTest BUNDLE_ID com.juce.AudioPerformanceTest)

drx_generate_drx_header(AudioPerformanceTest)

target_sources(AudioPerformanceTest PRIVATE
    Source/Main.cpp)

target_compile_definitions(AudioPerformanceTest PRIVATE
    JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)

target_link_libraries(AudioPerformanceTest PRIVATE
    juce::drx_audio_utils
    juce::drx_recommended_config_flags
    juce::drx_recommended_lto_flags
    juce::drx_recommended_warning_flags)
