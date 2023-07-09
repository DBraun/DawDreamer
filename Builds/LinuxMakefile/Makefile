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

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DDEBUG=1" "-D_DEBUG=1" "-DPIP_JUCE_EXAMPLES_DIRECTORY=QzpcdG9vbHNcSlVDRVxleGFtcGxlcw==" "-DSAMPLER_SKIP_UI" "-DJUCE_MODAL_LOOPS_PERMITTED" "-DHAVE_LIBSAMPLERATE" "-DUSE_BUILTIN_FFT" "-DUSE_PTHREADS" "-DBUILD_DAWDREAMER_FAUST" "-DBUILD_DAWDREAMER_RUBBERBAND" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=0.7.1" "-DJUCE_APP_VERSION_HEX=0x701" $(shell $(PKG_CONFIG) --cflags alsa freetype2) -pthread -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/lilv/src -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/lilv -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/sratom -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/sord/src -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/sord -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/serd -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/lv2 -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK -I../../thirdparty/JUCE/modules/juce_audio_processors/format_types/VST3_SDK -I../../JuceLibraryCode -I../../JuceLibraryCode/modules -I../../thirdparty -I../../thirdparty/pybind11/include -I../../thirdparty/faust/architecture -I../../thirdparty/faust/compiler -I../../thirdparty/faust/compiler/boxes -I../../thirdparty/faust/compiler/documentator -I../../thirdparty/faust/compiler/draw -I../../thirdparty/faust/compiler/draw/device -I../../thirdparty/faust/compiler/draw/schema -I../../thirdparty/faust/compiler/errors -I../../thirdparty/faust/compiler/evaluate -I../../thirdparty/faust/compiler/extended -I../../thirdparty/faust/compiler/generator -I../../thirdparty/faust/compiler/generator/interpreter -I../../thirdparty/faust/compiler/normalize -I../../thirdparty/faust/compiler/parallelize -I../../thirdparty/faust/compiler/parser -I../../thirdparty/faust/compiler/patternmatcher -I../../thirdparty/faust/compiler/propagate -I../../thirdparty/faust/compiler/signals -I../../thirdparty/faust/compiler/tlib -I../../thirdparty/faust/compiler/transform -I../../thirdparty/faust/compiler/utils -I../../thirdparty/libsamplerate/src -I../../thirdparty/libsamplerate/include $(CPPFLAGS)
  JUCE_CPPFLAGS_DYNAMIC_LIBRARY :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0" "-DJucePlugin_Build_LV2=0"
  JUCE_CFLAGS_DYNAMIC_LIBRARY := -fPIC -fvisibility=hidden
  JUCE_LDFLAGS_DYNAMIC_LIBRARY := -shared
  JUCE_TARGET_DYNAMIC_LIBRARY := libdawdreamer.so

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -fPIC -g -ggdb -O0 -fPIC $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) -L/usr/local/lib -L../../thirdparty/libfaust/ubuntu-x86_64/Debug/lib -L../../thirdparty/libsamplerate/build_release/src $(shell $(PKG_CONFIG) --libs alsa freetype2) -fvisibility=hidden -lrt -ldl -lpthread -lGL -lsamplerate -lfaustwithllvm $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

ifeq ($(CONFIG),Release)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Release
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DNDEBUG=1" "-DPIP_JUCE_EXAMPLES_DIRECTORY=QzpcdG9vbHNcSlVDRVxleGFtcGxlcw==" "-DSAMPLER_SKIP_UI" "-DJUCE_MODAL_LOOPS_PERMITTED" "-DHAVE_LIBSAMPLERATE" "-DUSE_BUILTIN_FFT" "-DUSE_PTHREADS" "-DBUILD_DAWDREAMER_FAUST" "-DBUILD_DAWDREAMER_RUBBERBAND" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=0.7.1" "-DJUCE_APP_VERSION_HEX=0x701" $(shell $(PKG_CONFIG) --cflags alsa freetype2) -pthread -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/lilv/src -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/lilv -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/sratom -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/sord/src -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/sord -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/serd -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK/lv2 -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/LV2_SDK -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK -I../../thirdparty/JUCE/modules/juce_audio_processors/format_types/VST3_SDK -I../../JuceLibraryCode -I../../JuceLibraryCode/modules -I../../thirdparty -I../../thirdparty/pybind11/include -I../../thirdparty/faust/architecture -I../../thirdparty/faust/compiler -I../../thirdparty/faust/compiler/boxes -I../../thirdparty/faust/compiler/documentator -I../../thirdparty/faust/compiler/draw -I../../thirdparty/faust/compiler/draw/device -I../../thirdparty/faust/compiler/draw/schema -I../../thirdparty/faust/compiler/errors -I../../thirdparty/faust/compiler/evaluate -I../../thirdparty/faust/compiler/extended -I../../thirdparty/faust/compiler/generator -I../../thirdparty/faust/compiler/generator/interpreter -I../../thirdparty/faust/compiler/normalize -I../../thirdparty/faust/compiler/parallelize -I../../thirdparty/faust/compiler/parser -I../../thirdparty/faust/compiler/patternmatcher -I../../thirdparty/faust/compiler/propagate -I../../thirdparty/faust/compiler/signals -I../../thirdparty/faust/compiler/tlib -I../../thirdparty/faust/compiler/transform -I../../thirdparty/faust/compiler/utils -I../../thirdparty/libsamplerate/src -I../../thirdparty/libsamplerate/include $(CPPFLAGS)
  JUCE_CPPFLAGS_DYNAMIC_LIBRARY :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0" "-DJucePlugin_Build_LV2=0"
  JUCE_CFLAGS_DYNAMIC_LIBRARY := -fPIC -fvisibility=hidden
  JUCE_LDFLAGS_DYNAMIC_LIBRARY := -shared
  JUCE_TARGET_DYNAMIC_LIBRARY := libdawdreamer.so

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -fPIC -O3 -fPIC $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) -L/usr/local/lib -L../../thirdparty/libfaust/ubuntu-x86_64/Release/lib -L../../thirdparty/libsamplerate/build_release/src $(shell $(PKG_CONFIG) --libs alsa freetype2) -fvisibility=hidden -lrt -ldl -lpthread -lGL -lsamplerate -lfaustwithllvm $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

OBJECTS_DYNAMIC_LIBRARY := \
  $(JUCE_OBJDIR)/DataModel_e101b2.o \
  $(JUCE_OBJDIR)/MPESettingsDataModel_aeb1f3d1.o \
  $(JUCE_OBJDIR)/SamplerAudioProcessor_298eea40.o \
  $(JUCE_OBJDIR)/SamplerAudioProcessorEditor_39db550d.o \
  $(JUCE_OBJDIR)/FaustProcessor_2056a3e0.o \
  $(JUCE_OBJDIR)/FaustBoxAPI_5f675c8b.o \
  $(JUCE_OBJDIR)/FaustSignalAPI_ca49a5a0.o \
  $(JUCE_OBJDIR)/FilterProcessor_8755f5b.o \
  $(JUCE_OBJDIR)/PlaybackWarpProcessor_eedba470.o \
  $(JUCE_OBJDIR)/PluginProcessor_a059e380.o \
  $(JUCE_OBJDIR)/ProcessorBase_65bf9464.o \
  $(JUCE_OBJDIR)/CustomParameters_5970e144.o \
  $(JUCE_OBJDIR)/custom_pybind_wrappers_ac32b5b4.o \
  $(JUCE_OBJDIR)/RenderEngine_d1c4d401.o \
  $(JUCE_OBJDIR)/source_704c2604.o \
  $(JUCE_OBJDIR)/include_juce_audio_basics_8a4e984a.o \
  $(JUCE_OBJDIR)/include_juce_audio_devices_63111d02.o \
  $(JUCE_OBJDIR)/include_juce_audio_formats_15f82001.o \
  $(JUCE_OBJDIR)/include_juce_audio_processors_10c03666.o \
  $(JUCE_OBJDIR)/include_juce_audio_processors_ara_2a4c6ef7.o \
  $(JUCE_OBJDIR)/include_juce_audio_processors_lv2_libs_12bdca08.o \
  $(JUCE_OBJDIR)/include_juce_core_f26d17db.o \
  $(JUCE_OBJDIR)/include_juce_cryptography_8cb807a8.o \
  $(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o \
  $(JUCE_OBJDIR)/include_juce_dsp_aeb2060f.o \
  $(JUCE_OBJDIR)/include_juce_events_fd7d695.o \
  $(JUCE_OBJDIR)/include_juce_graphics_f817e147.o \
  $(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o \
  $(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o \
  $(JUCE_OBJDIR)/include_juce_opengl_a8a032b.o \
  $(JUCE_OBJDIR)/include_juce_video_be78589.o \

.PHONY: clean all strip

all : $(JUCE_OUTDIR)/$(JUCE_TARGET_DYNAMIC_LIBRARY)

$(JUCE_OUTDIR)/$(JUCE_TARGET_DYNAMIC_LIBRARY) : $(OBJECTS_DYNAMIC_LIBRARY) $(RESOURCES)
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@$(PKG_CONFIG) --print-errors alsa freetype2
	@echo Linking "DawDreamer - Dynamic Library"
	-$(V_AT)mkdir -p $(JUCE_BINDIR)
	-$(V_AT)mkdir -p $(JUCE_LIBDIR)
	-$(V_AT)mkdir -p $(JUCE_OUTDIR)
	$(V_AT)$(CXX) -o $(JUCE_OUTDIR)/$(JUCE_TARGET_DYNAMIC_LIBRARY) $(OBJECTS_DYNAMIC_LIBRARY) $(JUCE_LDFLAGS) $(JUCE_LDFLAGS_DYNAMIC_LIBRARY) $(RESOURCES) $(TARGET_ARCH)

$(JUCE_OBJDIR)/DataModel_e101b2.o: ../../Source/Sampler/Source/DataModels/DataModel.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling DataModel.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MPESettingsDataModel_aeb1f3d1.o: ../../Source/Sampler/Source/DataModels/MPESettingsDataModel.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling MPESettingsDataModel.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/SamplerAudioProcessor_298eea40.o: ../../Source/Sampler/Source/SamplerAudioProcessor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling SamplerAudioProcessor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/SamplerAudioProcessorEditor_39db550d.o: ../../Source/Sampler/Source/SamplerAudioProcessorEditor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling SamplerAudioProcessorEditor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/FaustProcessor_2056a3e0.o: ../../Source/FaustProcessor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling FaustProcessor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/FaustBoxAPI_5f675c8b.o: ../../Source/FaustBoxAPI.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling FaustBoxAPI.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/FaustSignalAPI_ca49a5a0.o: ../../Source/FaustSignalAPI.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling FaustSignalAPI.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/FilterProcessor_8755f5b.o: ../../Source/FilterProcessor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling FilterProcessor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PlaybackWarpProcessor_eedba470.o: ../../Source/PlaybackWarpProcessor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling PlaybackWarpProcessor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PluginProcessor_a059e380.o: ../../Source/PluginProcessor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling PluginProcessor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/ProcessorBase_65bf9464.o: ../../Source/ProcessorBase.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling ProcessorBase.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/CustomParameters_5970e144.o: ../../Source/CustomParameters.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling CustomParameters.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/custom_pybind_wrappers_ac32b5b4.o: ../../Source/custom_pybind_wrappers.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling custom_pybind_wrappers.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/RenderEngine_d1c4d401.o: ../../Source/RenderEngine.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling RenderEngine.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/source_704c2604.o: ../../Source/source.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling source.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_audio_basics_8a4e984a.o: ../../JuceLibraryCode/include_juce_audio_basics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_audio_basics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_audio_devices_63111d02.o: ../../JuceLibraryCode/include_juce_audio_devices.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_audio_devices.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_audio_formats_15f82001.o: ../../JuceLibraryCode/include_juce_audio_formats.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_audio_formats.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_audio_processors_10c03666.o: ../../JuceLibraryCode/include_juce_audio_processors.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_audio_processors.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_audio_processors_ara_2a4c6ef7.o: ../../JuceLibraryCode/include_juce_audio_processors_ara.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_audio_processors_ara.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_audio_processors_lv2_libs_12bdca08.o: ../../JuceLibraryCode/include_juce_audio_processors_lv2_libs.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_audio_processors_lv2_libs.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_core_f26d17db.o: ../../JuceLibraryCode/include_juce_core.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_core.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_cryptography_8cb807a8.o: ../../JuceLibraryCode/include_juce_cryptography.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_cryptography.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o: ../../JuceLibraryCode/include_juce_data_structures.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_data_structures.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_dsp_aeb2060f.o: ../../JuceLibraryCode/include_juce_dsp.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_dsp.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_events_fd7d695.o: ../../JuceLibraryCode/include_juce_events.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_events.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_graphics_f817e147.o: ../../JuceLibraryCode/include_juce_graphics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_graphics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o: ../../JuceLibraryCode/include_juce_gui_basics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_gui_basics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o: ../../JuceLibraryCode/include_juce_gui_extra.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_gui_extra.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_opengl_a8a032b.o: ../../JuceLibraryCode/include_juce_opengl.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_opengl.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_video_be78589.o: ../../JuceLibraryCode/include_juce_video.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_video.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

clean:
	@echo Cleaning DawDreamer
	$(V_AT)$(CLEANCMD)

strip:
	@echo Stripping DawDreamer
	-$(V_AT)$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(TARGET)

-include $(OBJECTS_DYNAMIC_LIBRARY:%.o=%.d)
