# This script has been adapted from the example script that comes with JUCE in the cmake example dir.
# Original close-source Stochas code used the projucer 
#
# To intall JUCE, from the downloaded JUCE directory: 
#   cmake -B cmake-build-install -DCMAKE_INSTALL_PREFIX=/path/to/desired/install/dir
#   cmake --build cmake-build-install --target install
#
# To build stochas:
#   cmake -B build
#   cmake --build build
#
cmake_minimum_required(VERSION 3.15)
if (DEFINED ENV{STOCHAS_VERSION} )
  set (STOCHAS_VERSION $ENV{STOCHAS_VERSION})
else()
  set (STOCHAS_VERSION 0.9.0)
endif()  

project(Stochas VERSION ${STOCHAS_VERSION})
message( STATUS "Building Stochas with version ${STOCHAS_VERSION}" )
add_subdirectory(lib/JUCE)

# create version.h and allow it to be referenced
configure_file(src/version.h.in version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# if you want vst2, you need to specify VST2_PATH in your environment
if (DEFINED ENV{VST2_PATH})
  juce_set_vst2_sdk_path($ENV{VST2_PATH})
  set(VST2 "VST")
endif()


# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (AudioPluginExample here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(stochas
    COMPANY_NAME "Audio Vitamins"
    BUNDLE_ID "com.audiovitamins.stochas"
    DESCRIPTION "Stochas Randomization Enabled Step Sequencer"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE AuVi
    PLUGIN_CODE Stoc
    FORMATS VST3 ${VST2} AU AUv3 Standalone                 # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    VST3_CATEGORIES "Instrument"
    PRODUCT_NAME "Stochas")        # The name of the final executable, which can differ from the target name


juce_generate_juce_header(stochas)  

target_sources(stochas PRIVATE
    src/ChainDialog.cpp
    src/CommonComponents.cpp
    src/EditDialog.cpp
    src/EditorState.cpp
    src/FileDialog.cpp
    src/HelpBanner.cpp
    src/InfoDialog.cpp
    src/MidiDialog.cpp
    src/NotePanel.cpp
    src/OptionsPanel.cpp
    src/pcg_basic.c
    src/Persist.cpp
    src/PlayPanel.cpp
    src/PluginEditor.cpp
    src/PluginProcessor.cpp
    src/Scale.cpp
    src/SeqRandom.cpp
    src/SequenceData.cpp
    src/SettingsTab.cpp
    src/StepPanel.cpp
    src/StochaEngine.cpp
    
    )

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(stochas
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0
 
    # Some linux options
    JUCE_JACK=1
    JUCE_ALSA=1
)

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

juce_add_binary_data(assets NAMESPACE SeqImageX SOURCES 
    image/exclamation-64.png
    image/not.png
    image/play.png
    image/play2.png
    image/Stochas-logo.png
    image/x-mark-4-64.png
    )

set_target_properties(assets PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)


# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_audio_utils` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.

target_link_libraries(stochas PRIVATE
    assets                             # to include our images
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    )
