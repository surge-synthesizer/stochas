# This script has been adapted from the example script that comes with JUCE in the cmake example dir.
# Original closed-source Stochas code used the projucer 
#
# To build stochas:
#   git submodule update --init --recursive
#   cmake -B build -DSTOCHAS_VERSION=xxx, etc.
#   cmake --build build  --config Release
#
cmake_minimum_required(VERSION 3.16)

# always statically link c runtime on windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# default to 10.11 (earlier versions will fail)
if (NOT DEFINED STOCHAS_MAC_SDK_VER)
  set(STOCHAS_MAC_SDK_VER 10.11)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET ${STOCHAS_MAC_SDK_VER} CACHE STRING "Build for ${STOCHAS_MAC_SDK_VER}")

if (NOT DEFINED STOCHAS_VERSION)
  set (STOCHAS_VERSION 1.3.0)
endif()  

project(Stochas VERSION ${STOCHAS_VERSION})
message( STATUS "Building Stochas with version ${STOCHAS_VERSION}" )
if( APPLE )
  message( STATUS "Building using ${CMAKE_OSX_DEPLOYMENT_TARGET}" )
endif()

# synth defaults to true
if (NOT DEFINED STOCHAS_IS_SYNTH)  
  set(STOCHAS_IS_SYNTH "TRUE")
endif()

# midi effect defaults to true
# TRUE is needed for AU and AUv3 in order for proper functioning in Logic
# but may have issues with other daw's. Specifically:
# I had some problems in cubase when this was turned on.
# Intermittent. It seems that when there are no output buffers it
# can sometimes get garbage in its buffer. Turning this off, while it
# causes the plugin to have a stereo out bus, seems to alleviate the problem.
# to rep the problem start cubase, hit F11 add stochas as an instrument. right away
# it starts producing a buzz (although not always)

if (NOT DEFINED STOCHAS_IS_MIDI_EFFECT)  
  set(STOCHAS_IS_MIDI_EFFECT "TRUE")
endif()
message( STATUS "Is Synth: ${STOCHAS_IS_SYNTH}, Is Midi Effect: ${STOCHAS_IS_MIDI_EFFECT}")
# I'm not sure at this point whether this has any effect. I know etting it to "Instrument" causes it to show as
# VSTi in reaper whereas setting it to "Effect" causes it to show as VST
set(STOCHAS_CATEGORY "Effect") # original was "Instrument"
add_subdirectory(lib/JUCE)

add_custom_target( git-info BYPRODUCTS ${CMAKE_BINARY_DIR}/geninclude/version.cpp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -D PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}  -D PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
                           -D STOCHASSRC=${CMAKE_SOURCE_DIR} -D STOCHASBLD=${CMAKE_BINARY_DIR}
                           -D AZURE_PIPELINE=${AZURE_PIPELINE}
                           -P ${CMAKE_SOURCE_DIR}/cmake/versiontools.cmake
                           )

# if you want vst2, you need to specify VST2_PATH in your environment
if (DEFINED VST2_PATH)
  message( STATUS "Building VST2. SDK at ${VST2_PATH}")
  juce_set_vst2_sdk_path(${VST2_PATH})
  set(VST2 "VST")
endif()

# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (AudioPluginExample here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(stochas
    COMPANY_NAME "Surge Synth Team"
    BUNDLE_ID "org.surge-synth-team.stochas"
    DESCRIPTION "Stochas Randomization Enabled Step Sequencer"
    ICON_BIG   "${CMAKE_CURRENT_SOURCE_DIR}/image/app_logo_512.png"
    IS_SYNTH ${STOCHAS_IS_SYNTH}
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT ${STOCHAS_IS_MIDI_EFFECT}  
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE AuVi               # allows compatibility with older patches from AV days
    PLUGIN_CODE Stoc
    FORMATS VST3 ${VST2} AU Standalone     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    VST3_CATEGORIES ${STOCHAS_CATEGORY} 
    PRODUCT_NAME "Stochas")        # The name of the final executable, which can differ from the target name

add_dependencies( stochas git-info )
include_directories( src )

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

    ${CMAKE_BINARY_DIR}/geninclude/version.cpp
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

    # GPL3 Plugs can disable splash screen
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0

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
    image/logo_no_bg.png
    image/x-mark-4-64.png
    text/infobox.txt
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

if( APPLE )
  add_custom_target( install-au-local )
  add_dependencies( install-au-local stochas_AU )
  add_custom_command(
    TARGET install-au-local
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND echo "Installing audio unit locally"
    COMMAND rsync -r --delete "${ZIP_FROM_DIR}/AU/Stochas.component/" "\${HOME}/Library/Audio/Plug-Ins/Components/Stochas.component/"
    COMMAND auval -vt aumi AuVi
    )

endif()
