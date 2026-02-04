# Automatically generated makefile, created by the Projucer
# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!

# build with "V=1" for verbose builds
ifeq ($(V), 1)
V_AT =
else
V_AT = @
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifndef PKG_CONFIG
  PKG_CONFIG=pkg-config
endif

ifndef STRIP
  STRIP=strip
endif

ifndef AR
  AR=ar
endif

ifndef CONFIG
  CONFIG=Debug
endif

JUCE_ARCH_LABEL := $(shell uname -m)

ifeq ($(CONFIG),Debug)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Debug
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DDEBUG=1" "-D_DEBUG=1" "-DJUCE_PROJUCER_VERSION=0x80007" "-DJUCE_MODULE_AVAILABLE_drx_audio_basics=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_devices=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_formats=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_processors=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_utils=1" "-DJUCE_MODULE_AVAILABLE_drx_core=1" "-DJUCE_MODULE_AVAILABLE_drx_cryptography=1" "-DJUCE_MODULE_AVAILABLE_drx_data_structures=1" "-DJUCE_MODULE_AVAILABLE_drx_dsp=1" "-DJUCE_MODULE_AVAILABLE_drx_events=1" "-DJUCE_MODULE_AVAILABLE_drx_graphics=1" "-DJUCE_MODULE_AVAILABLE_drx_gui_basics=1" "-DJUCE_MODULE_AVAILABLE_drx_gui_extra=1" "-DJUCE_MODULE_AVAILABLE_drx_opengl=1" "-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1" "-DJUCE_WASAPI=1" "-DJUCE_DIRECTSOUND=1" "-DJUCE_ALSA=1" "-DJUCE_USE_FLAC=0" "-DJUCE_USE_OGGVORBIS=1" "-DJUCE_PLUGINHOST_VST3=1" "-DJUCE_PLUGINHOST_AU=1" "-DJUCE_PLUGINHOST_LADSPA=1" "-DJUCE_PLUGINHOST_LV2=1" "-DJUCE_USE_CDREADER=0" "-DJUCE_USE_CDBURNER=0" "-DJUCE_WEB_BROWSER=0" "-DJUCE_STANDALONE_APPLICATION=1" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=1.0.0" "-DJUCE_APP_VERSION_HEX=0x10000" $(shell $(PKG_CONFIG) --cflags alsa freetype2 fontconfig gl libcurl) -pthread -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/lilv/src -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/lilv -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/sratom -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/sord/src -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/sord -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/serd -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/lv2 -I../../../../modules/drx_audio_processors/format_types/LV2_SDK -I../../../../modules/drx_audio_processors/format_types/VST3_SDK -I../../DrxLibraryCode -I../../../../modules $(CPPFLAGS)
  JUCE_CPPFLAGS_APP :=  "-DDrxPlugin_Build_VST=0" "-DDrxPlugin_Build_VST3=0" "-DDrxPlugin_Build_AU=0" "-DDrxPlugin_Build_AUv3=0" "-DDrxPlugin_Build_AAX=0" "-DDrxPlugin_Build_Standalone=0" "-DDrxPlugin_Build_Unity=0" "-DDrxPlugin_Build_LV2=0"
  JUCE_TARGET_APP := AudioPluginHost

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0 $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell $(PKG_CONFIG) --libs alsa freetype2 fontconfig gl libcurl) -fvisibility=hidden -lrt -ldl -lpthread $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(JUCE_TARGET_APP) $(JUCE_OBJDIR)
endif

ifeq ($(CONFIG),Release)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Release
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DNDEBUG=1" "-DJUCE_PROJUCER_VERSION=0x80007" "-DJUCE_MODULE_AVAILABLE_drx_audio_basics=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_devices=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_formats=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_processors=1" "-DJUCE_MODULE_AVAILABLE_drx_audio_utils=1" "-DJUCE_MODULE_AVAILABLE_drx_core=1" "-DJUCE_MODULE_AVAILABLE_drx_cryptography=1" "-DJUCE_MODULE_AVAILABLE_drx_data_structures=1" "-DJUCE_MODULE_AVAILABLE_drx_dsp=1" "-DJUCE_MODULE_AVAILABLE_drx_events=1" "-DJUCE_MODULE_AVAILABLE_drx_graphics=1" "-DJUCE_MODULE_AVAILABLE_drx_gui_basics=1" "-DJUCE_MODULE_AVAILABLE_drx_gui_extra=1" "-DJUCE_MODULE_AVAILABLE_drx_opengl=1" "-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1" "-DJUCE_WASAPI=1" "-DJUCE_DIRECTSOUND=1" "-DJUCE_ALSA=1" "-DJUCE_USE_FLAC=0" "-DJUCE_USE_OGGVORBIS=1" "-DJUCE_PLUGINHOST_VST3=1" "-DJUCE_PLUGINHOST_AU=1" "-DJUCE_PLUGINHOST_LADSPA=1" "-DJUCE_PLUGINHOST_LV2=1" "-DJUCE_USE_CDREADER=0" "-DJUCE_USE_CDBURNER=0" "-DJUCE_WEB_BROWSER=0" "-DJUCE_STANDALONE_APPLICATION=1" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=1.0.0" "-DJUCE_APP_VERSION_HEX=0x10000" $(shell $(PKG_CONFIG) --cflags alsa freetype2 fontconfig gl libcurl) -pthread -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/lilv/src -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/lilv -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/sratom -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/sord/src -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/sord -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/serd -I../../../../modules/drx_audio_processors/format_types/LV2_SDK/lv2 -I../../../../modules/drx_audio_processors/format_types/LV2_SDK -I../../../../modules/drx_audio_processors/format_types/VST3_SDK -I../../DrxLibraryCode -I../../../../modules $(CPPFLAGS)
  JUCE_CPPFLAGS_APP :=  "-DDrxPlugin_Build_VST=0" "-DDrxPlugin_Build_VST3=0" "-DDrxPlugin_Build_AU=0" "-DDrxPlugin_Build_AUv3=0" "-DDrxPlugin_Build_AAX=0" "-DDrxPlugin_Build_Standalone=0" "-DDrxPlugin_Build_Unity=0" "-DDrxPlugin_Build_LV2=0"
  JUCE_TARGET_APP := AudioPluginHost

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -Os $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell $(PKG_CONFIG) --libs alsa freetype2 fontconfig gl libcurl) -fvisibility=hidden -lrt -ldl -lpthread $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(JUCE_TARGET_APP) $(JUCE_OBJDIR)
endif

OBJECTS_APP := \
  $(JUCE_OBJDIR)/ARAPlugin_e9864935.o \
  $(JUCE_OBJDIR)/InternalPlugins_8278e3f5.o \
  $(JUCE_OBJDIR)/IOConfigurationWindow_d71a5732.o \
  $(JUCE_OBJDIR)/PluginGraph_6bd15e2d.o \
  $(JUCE_OBJDIR)/GraphEditorPanel_2223d925.o \
  $(JUCE_OBJDIR)/MainHostWindow_b3494acd.o \
  $(JUCE_OBJDIR)/HostStartup_5ce96f96.o \
  $(JUCE_OBJDIR)/BinaryData_ce4232d4.o \
  $(JUCE_OBJDIR)/include_drx_audio_basics_8a4e984a.o \
  $(JUCE_OBJDIR)/include_drx_audio_devices_63111d02.o \
  $(JUCE_OBJDIR)/include_drx_audio_formats_15f82001.o \
  $(JUCE_OBJDIR)/include_drx_audio_processors_10c03666.o \
  $(JUCE_OBJDIR)/include_drx_audio_processors_ara_2a4c6ef7.o \
  $(JUCE_OBJDIR)/include_drx_audio_processors_lv2_libs_12bdca08.o \
  $(JUCE_OBJDIR)/include_drx_audio_utils_9f9fb2d6.o \
  $(JUCE_OBJDIR)/include_drx_core_f26d17db.o \
  $(JUCE_OBJDIR)/include_drx_core_CompilationTime_9257742c.o \
  $(JUCE_OBJDIR)/include_drx_cryptography_8cb807a8.o \
  $(JUCE_OBJDIR)/include_drx_data_structures_7471b1e3.o \
  $(JUCE_OBJDIR)/include_drx_dsp_aeb2060f.o \
  $(JUCE_OBJDIR)/include_drx_events_fd7d695.o \
  $(JUCE_OBJDIR)/include_drx_graphics_f817e147.o \
  $(JUCE_OBJDIR)/include_drx_graphics_Harfbuzz_60c52ba2.o \
  $(JUCE_OBJDIR)/include_drx_graphics_Sheenbidi_c310974d.o \
  $(JUCE_OBJDIR)/include_drx_gui_basics_e3f79785.o \
  $(JUCE_OBJDIR)/include_drx_gui_extra_6dee1c1a.o \
  $(JUCE_OBJDIR)/include_drx_opengl_a8a032b.o \

.PHONY: clean all strip

all : $(JUCE_OUTDIR)/$(JUCE_TARGET_APP)

$(JUCE_OUTDIR)/$(JUCE_TARGET_APP) : $(OBJECTS_APP) $(JUCE_OBJDIR)/execinfo.cmd $(RESOURCES)
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@$(PKG_CONFIG) --print-errors alsa freetype2 fontconfig gl libcurl
	@echo Linking "AudioPluginHost - App"
	-$(V_AT)mkdir -p $(JUCE_BINDIR)
	-$(V_AT)mkdir -p $(JUCE_LIBDIR)
	-$(V_AT)mkdir -p $(JUCE_OUTDIR)
	$(V_AT)$(CXX) -o $(JUCE_OUTDIR)/$(JUCE_TARGET_APP) $(OBJECTS_APP) $(JUCE_LDFLAGS) $(shell cat $(JUCE_OBJDIR)/execinfo.cmd) $(JUCE_LDFLAGS_APP) $(RESOURCES) $(TARGET_ARCH)

$(JUCE_OBJDIR)/ARAPlugin_e9864935.o: ../../Source/Plugins/ARAPlugin.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling ARAPlugin.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/InternalPlugins_8278e3f5.o: ../../Source/Plugins/InternalPlugins.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling InternalPlugins.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/IOConfigurationWindow_d71a5732.o: ../../Source/Plugins/IOConfigurationWindow.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling IOConfigurationWindow.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PluginGraph_6bd15e2d.o: ../../Source/Plugins/PluginGraph.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling PluginGraph.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/GraphEditorPanel_2223d925.o: ../../Source/UI/GraphEditorPanel.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling GraphEditorPanel.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MainHostWindow_b3494acd.o: ../../Source/UI/MainHostWindow.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling MainHostWindow.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/HostStartup_5ce96f96.o: ../../Source/HostStartup.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling HostStartup.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/BinaryData_ce4232d4.o: ../../DrxLibraryCode/BinaryData.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling BinaryData.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_audio_basics_8a4e984a.o: ../../DrxLibraryCode/include_drx_audio_basics.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_audio_basics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_audio_devices_63111d02.o: ../../DrxLibraryCode/include_drx_audio_devices.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_audio_devices.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_audio_formats_15f82001.o: ../../DrxLibraryCode/include_drx_audio_formats.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_audio_formats.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_audio_processors_10c03666.o: ../../DrxLibraryCode/include_drx_audio_processors.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_audio_processors.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_audio_processors_ara_2a4c6ef7.o: ../../DrxLibraryCode/include_drx_audio_processors_ara.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_audio_processors_ara.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_audio_processors_lv2_libs_12bdca08.o: ../../DrxLibraryCode/include_drx_audio_processors_lv2_libs.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_audio_processors_lv2_libs.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_audio_utils_9f9fb2d6.o: ../../DrxLibraryCode/include_drx_audio_utils.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_audio_utils.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_core_f26d17db.o: ../../DrxLibraryCode/include_drx_core.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_core.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_core_CompilationTime_9257742c.o: ../../DrxLibraryCode/include_drx_core_CompilationTime.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_core_CompilationTime.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_cryptography_8cb807a8.o: ../../DrxLibraryCode/include_drx_cryptography.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_cryptography.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_data_structures_7471b1e3.o: ../../DrxLibraryCode/include_drx_data_structures.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_data_structures.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_dsp_aeb2060f.o: ../../DrxLibraryCode/include_drx_dsp.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_dsp.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_events_fd7d695.o: ../../DrxLibraryCode/include_drx_events.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_events.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_graphics_f817e147.o: ../../DrxLibraryCode/include_drx_graphics.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_graphics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_graphics_Harfbuzz_60c52ba2.o: ../../DrxLibraryCode/include_drx_graphics_Harfbuzz.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_graphics_Harfbuzz.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_graphics_Sheenbidi_c310974d.o: ../../DrxLibraryCode/include_drx_graphics_Sheenbidi.c
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_graphics_Sheenbidi.c"
	$(V_AT)$(CC) $(JUCE_CFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_gui_basics_e3f79785.o: ../../DrxLibraryCode/include_drx_gui_basics.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_gui_basics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_gui_extra_6dee1c1a.o: ../../DrxLibraryCode/include_drx_gui_extra.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_gui_extra.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_drx_opengl_a8a032b.o: ../../DrxLibraryCode/include_drx_opengl.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_drx_opengl.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/execinfo.cmd:
	-$(V_AT)mkdir -p $(@D)
	-@if [ -z "$(V_AT)" ]; then echo "Checking if we need to link libexecinfo"; fi
	$(V_AT)printf "i32 main() { return 0; }" | $(CXX) -x c++ -o $(@D)/execinfo.x -lexecinfo - >/dev/null 2>&1 && printf -- "-lexecinfo" > "$@" || touch "$@"

$(JUCE_OBJDIR)/cxxfs.cmd:
	-$(V_AT)mkdir -p $(@D)
	-@if [ -z "$(V_AT)" ]; then echo "Checking if we need to link stdc++fs"; fi
	$(V_AT)printf "i32 main() { return 0; }" | $(CXX) -x c++ -o $(@D)/cxxfs.x -lstdc++fs - >/dev/null 2>&1 && printf -- "-lstdc++fs" > "$@" || touch "$@"

clean:
	@echo Cleaning AudioPluginHost
	$(V_AT)$(CLEANCMD)

strip:
	@echo Stripping AudioPluginHost
	-$(V_AT)$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(JUCE_TARGET_APP)

-include $(OBJECTS_APP:%.o=%.d)
