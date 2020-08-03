# Stochas 

## History
Stochas was originally developed as closed-source software back in 2016 and was made available for sale by Audio Vitamins.
The plugin was designed by Andrew Shakinovsky and Dave Clissold, and all coding was done by Andrew Shakinovsky. The original idea sprang from a JSFX plugin (Reaper plugin) created by Andrew called Stochasticizer which allowed semi-random sequencing of melodic and drum lines. Andrew was contacted by Dave who suggested creating a plugin that would work on any DAW. The two worked together to design the product which was released a few months later. Due to lack of time to devote to marketing a
commercial product, in 2020, it was decided to release the software as open-source in the hope that it would be useful to music producers around the world.

## Features
- Cross platform MIDI sequencer surfaced as a VST/AU/etc. plugin
- Allows random selection of designated notes in a sequence (random or semi-random melodic lines)
- Allows random triggering of notes (more dynamic and varied drum patterns)
- Adjust randomness of note start times for more humanized playback
- Adjust randomness of note velocity and length
- Fully MIDI-programmable interface for live performance. Mute/unmute layers, change patterns, time signature, etc. on the fly via MIDI.
- Chain mode allows procedural programming (eg "if this note plays/doesn't play then always/never play this other note")
- Adjust note start time to move sequence "off the grid", as well as note length.
- Record incoming midi to grid
- Use Chord mode to quickly add chords to the sequence
- Select from preset scales, or customize notes in grid along with note names. Save/load note names from file
- Add swing or groove. Import groove from MIDI files
- Create up to four layers with differing time signature, playback speed and/or number of steps per layer
- Create up to eight patterns

## Download
To download pre-built binaries go [here](https://github.com/rudeog/stochas_open/releases/) which is on the main github project page.

## Platforms and DAWs
The original product has been tested on Windows and Mac using Reaper, Logic, Cubase, Ableton, FLStudio, Studio One, Cakewalk, ProTools. 
The open-source version has been so far tested on Windows and Mac using Reaper, but should have no problem with other targets.
By default AAX is not built, but if you have the AAX sdk you will be able to enable this.

## Building
### Pre-requisites
- Windows or Mac OSX based system
- C++ based developer toolchain such as Clang, VC++, etc.
- CMake
- JUCE version 6
- VSCode (optional but recommended, see below)
- VST2 sdk (only if you need vst2 plugin)
- AAX (ProTools) SDK (only if you need it. disabled by default)

### Building
- Install developer tool chain on your system. Windows has been tested with MS C++, Mac has been tested with Clang.
- Install CMake on your system. Go to cmake.org/download
- Install JUCE using Cmake as follows
  - cmake -B cmake-build-install -DCMAKE_INSTALL_PREFIX=/desired/juce/install/dir
  - cmake --build cmake-build-install --target install
- VST2 - if you need it you need to define VST2_PATH in your environment before following steps.
- AAX - if you need it you need to install the sdk and edit the CMakefile
- Build Stochas (note that the -DCMAKE_INSTALL_PREFIX may or may not be needed here):
  - cmake -B build -DCMAKE_INSTALL_PREFIX=/desired/juce/install/dir
  - cmake --build build

### VSCode
Development is a lot easier with VSCode using the CMake extension. Simply point vscode at the root directory of the repo. It pretty much detects a cmake project and handles building without any issues.
- Install C++ extensions for vscode
- Install CMake Tools extensions for vscode
- In the settings for CMake (settings/extensions/Cmake tools configuration) under Cmake: Install Prefix enter the path where you installed JUCE using Cmake (ie /desired/juce/install/dir if you followed above directions). Note that this seemed to be needed on Windows but not Mac.


## Projucer
Prior to JUCE v6, Stochas was managed via Projucer. With the port to v6 and also the release of Stochas as OSS, Projucer is no longer in use. There were some settings in Projucer which had to be adjusted depending on the target. These notes may not be applicable anymore. In particular channel config should be looked at... from juce cmake doc " It is recommended to avoid this option altogether, and to use the newer buses API to specify the desired plugin inputs and outputs." Following are the old notes from
Projucer days:

### VST and AAX
- Channel Config - {1, 1}, {2, 2}
- Plugin is a Synth
- Plugin Midi Input 
- Plugin Midi Output

On an additional note for AAX there are linking errors on building in XCode, so the following settings have to be set. 

- C++ Language - C++11
- C++ Library - GNU libstdc++

I have read somewhere it’s associated with how Xcode built the SDK at my end, but rather than go fiddling and trying to recompile the SDK in other ways, I just stick to the above settings as they work.

### AU 
- Channel Config - {1, 1}, {2, 2}
- Plugin Midi input
- Plugin Midi output
- Midi Effect Plugin
