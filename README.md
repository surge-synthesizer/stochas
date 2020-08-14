# Stochas 
[Main web site](https://surge-synth-team.org/stochas/)

## Download
To download pre-built binaries and installations go [here](https://github.com/surge-synthesizer/stochas/releases)

## Platforms and DAWs
The original product has been tested on Windows and Mac using Reaper, Logic, Cubase, Ableton, FLStudio, Studio One, Cakewalk, ProTools. 
The open-source version has been so far tested on Windows, Mac and Linux using Reaper, Logic, Bitwig and others, but should have no problem with other targets.
By default AAX is not built, but if you have the AAX sdk you will be able to enable this.

## Building
### Pre-requisites
- Windows, Linux, Mac OSX based system
- C++ based developer toolchain such as Clang, VC++, etc.
- CMake
- VSCode (optional but recommended, see below)
- VST2 sdk (only if you need vst2 plugin)
- AAX (ProTools) SDK (only if you need it. disabled by default)

### Building
- Install developer tool chain on your system. Windows has been tested with MS C++, Mac has been tested with Clang, Linux with GCC on Ubuntu and Buster
- Install CMake on your system. Go to cmake.org/download
- If you want to set a particular version add -DSTOCHAS_VERSION=9.9.9 in options below otherwise the version will be 0.9.0 
- VST2 - if you need it you need to add -DVST2_PATH=path-to-vst2-sdk-here to options below
- AAX - if you need it you need to install the sdk and edit the CMakefile
- AU - if you are building the AudioUnit add the option -DSTOCHAS_IS_SYNTH=FALSE
- Build Stochas:
  - git submodule update --init --recursive
  - cmake -B build [options]
  - cmake --build build --config Release


### VSCode
Development is a lot easier with VSCode using the CMake extension. Simply point vscode at the root directory of the repo. It pretty much detects a cmake project and handles building without any issues.
- Install C++ extensions for vscode
- Install CMake Tools extensions for vscode

### XCode

If you want to use XCode on macintosh, adjust the first cmake command to `cmake -B build -GXcode` and your build directory will contain xcode assets.

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
