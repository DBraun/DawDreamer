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

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DDEBUG=1" "-D_DEBUG=1" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=1.0.0" "-DJUCE_APP_VERSION_HEX=0x10000" $(shell pkg-config --cflags alsa freetype2 libcurl) -pthread -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK -I../../thirdparty/JUCE/modules/juce_audio_processors/format_types/VST3_SDK -I../../JuceLibraryCode -I../../JuceLibraryCode/modules -I/usr/include/python3.6m -I../../thirdparty/pybind11/include $(CPPFLAGS)
  JUCE_CPPFLAGS_DYNAMIC_LIBRARY :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_RTAS=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0"
  JUCE_CFLAGS_DYNAMIC_LIBRARY := -fPIC -fvisibility=hidden
  JUCE_LDFLAGS_DYNAMIC_LIBRARY := -shared
  JUCE_TARGET_DYNAMIC_LIBRARY := libdawdreamer.so

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -fPIC -g -ggdb -O0 -fPIC $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++14 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell pkg-config --libs alsa freetype2 libcurl) -fvisibility=hidden -shared -lpython3.6m -lrt -ldl -lpthread -lGL $(LDFLAGS)

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

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DNDEBUG=1" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=1.0.0" "-DJUCE_APP_VERSION_HEX=0x10000" $(shell pkg-config --cflags alsa freetype2 libcurl) -pthread -I../../JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK -I../../thirdparty/JUCE/modules/juce_audio_processors/format_types/VST3_SDK -I../../JuceLibraryCode -I../../JuceLibraryCode/modules -I/usr/include/python3.6m -I../../thirdparty/pybind11/include $(CPPFLAGS)
  JUCE_CPPFLAGS_DYNAMIC_LIBRARY :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_RTAS=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0"
  JUCE_CFLAGS_DYNAMIC_LIBRARY := -fPIC -fvisibility=hidden
  JUCE_LDFLAGS_DYNAMIC_LIBRARY := -shared
  JUCE_TARGET_DYNAMIC_LIBRARY := libdawdreamer.so

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -fPIC -O3 -fPIC $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++14 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell pkg-config --libs alsa freetype2 libcurl) -fvisibility=hidden -shared -lpython3.6m -lrt -ldl -lpthread -lGL $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

OBJECTS_DYNAMIC_LIBRARY := \
  $(JUCE_OBJDIR)/FilterProcessor_8755f5b.o \
  $(JUCE_OBJDIR)/PluginProcessor_a059e380.o \
  $(JUCE_OBJDIR)/ProcessorBase_65bf9464.o \
  $(JUCE_OBJDIR)/fft_7b305358.o \
  $(JUCE_OBJDIR)/maxiFFT_413b4093.o \
  $(JUCE_OBJDIR)/maxiMFCC_a1624ee4.o \
  $(JUCE_OBJDIR)/maximilian_1e00e870.o \
  $(JUCE_OBJDIR)/custom_pybind_wrappers_ac32b5b4.o \
  $(JUCE_OBJDIR)/RenderEngine_d1c4d401.o \
  $(JUCE_OBJDIR)/RenderEngineWrapper_9b21dedc.o \
  $(JUCE_OBJDIR)/source_704c2604.o \
  $(JUCE_OBJDIR)/include_juce_audio_basics_8a4e984a.o \
  $(JUCE_OBJDIR)/include_juce_audio_devices_63111d02.o \
  $(JUCE_OBJDIR)/include_juce_audio_formats_15f82001.o \
  $(JUCE_OBJDIR)/include_juce_audio_processors_10c03666.o \
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
	@command -v pkg-config >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@pkg-config --print-errors alsa freetype2 libcurl
	@echo Linking "DawDreamer - Dynamic Library"
	-$(V_AT)mkdir -p $(JUCE_BINDIR)
	-$(V_AT)mkdir -p $(JUCE_LIBDIR)
	-$(V_AT)mkdir -p $(JUCE_OUTDIR)
	$(V_AT)$(CXX) -o $(JUCE_OUTDIR)/$(JUCE_TARGET_DYNAMIC_LIBRARY) $(OBJECTS_DYNAMIC_LIBRARY) $(JUCE_LDFLAGS) $(JUCE_LDFLAGS_DYNAMIC_LIBRARY) $(RESOURCES) $(TARGET_ARCH)

$(JUCE_OBJDIR)/FilterProcessor_8755f5b.o: ../../Source/FilterProcessor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling FilterProcessor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PluginProcessor_a059e380.o: ../../Source/PluginProcessor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling PluginProcessor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/ProcessorBase_65bf9464.o: ../../Source/ProcessorBase.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling ProcessorBase.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/fft_7b305358.o: ../../Source/Maximilian/libs/fft.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling fft.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/maxiFFT_413b4093.o: ../../Source/Maximilian/libs/maxiFFT.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling maxiFFT.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/maxiMFCC_a1624ee4.o: ../../Source/Maximilian/libs/maxiMFCC.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling maxiMFCC.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/maximilian_1e00e870.o: ../../Source/Maximilian/maximilian.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling maximilian.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/custom_pybind_wrappers_ac32b5b4.o: ../../Source/custom_pybind_wrappers.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling custom_pybind_wrappers.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/RenderEngine_d1c4d401.o: ../../Source/RenderEngine.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling RenderEngine.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/RenderEngineWrapper_9b21dedc.o: ../../Source/RenderEngineWrapper.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling RenderEngineWrapper.cpp"
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
