# Stochas 

## History
Stochas was originally developed as closed-source software back in 2016 and was made available for sale by Audio Vitamins.
The plugin was designed by Andrew Shakinovsky and Dave Clissold, and all coding was done by Andrew Shakinovsky. The original idea sprang from a JSFX plugin (Reaper plugin) created by Andrew called Stochasticizer which allowed semi-random sequencing of melodic and drum lines. Andrew was contacted by Dave who suggested turning it into a full featured sequencer with randomization built in. The two worked together to design the product which was released a few months later. In 2020, it was decided to release the software as open-source in the hope that it would be useful to music producers around the world.

## Download
To download pre-built binaries go here ...

## Platforms and DAWs
The original product has been tested on Windows and Mac using Reaper, Logic, Cubase, Ableton, FLStudio, Studio One, Cakewalk, ProTools. 
The open-source version has been so far tested on Windows and Mac using Reaper, but should have no problem with other targets.

## Building
### Pre-requisites
- Windows or Mac OSX based system
- C++ based developer toolchain such as Clang, VC++, etc.
- CMake
- JUCE version 6
- VSCode (optional, see below)
- VST2 sdk (if you need vst2 plugin)
### Building
- Install developer tools on your system.
- Install CMake on your system. Go to cmake.org/download
- Install JUCE using Cmake as follows
  - cmake -B cmake-build-install -DCMAKE_INSTALL_PREFIX=/desired/juce/install/dir
  - cmake --build cmake-build-install --target install
- VST2 - if you need it you need to define VST2_PATH in your environment before following steps.
- AAX - if you need it you need to define AAX_PATH in your environment
- Build Stochas (note that the -DCMAKE_INSTALL_PREFIX may or may not be needed here):
  - cmake -B build -DCMAKE_INSTALL_PREFIX=/desired/juce/install/dir
  - cmake --build build

### VSCode
Development is a lot easier with VSCode using the CMake extension. Simply point vscode at the root directory of the repo. It pretty much detects a cmake project and handles building without any issues.
- Install C++ extensions for vscode
- Install CMake Tools extensions for vscode
- In the settings for CMake (settings/extensions/Cmake tools configuration) under Cmake: Install Prefix enter the path where you installed JUCE using Cmake (ie /desired/juce/install/dir if you followed above directions). Note that this seemed to be needed on Windows but not Mac.


## Projucer
Prior to JUCE v6, Stochas was managed via Projucer. With the port to v6 and also the release of Stochas as OSS, Projucer is no longer in use. There were some settings in Projucer which had to be adjusted depending on the target. These notes may not be applicable anymore. In particular channel config should be looked at... from juce cmake doc " It is recommended to avoid this option altogether, and to use the newer buses API to specify the desired plugin inputs and outputs."

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
