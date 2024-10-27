#!/bin/bash

set -ex
# 64 bit
cmake -Bignore/build -A x64 -DSTOCHAS_VERSION=${STOCHAS_VERSION}
cmake --build ignore/build --config Release
# 32 bit
cmake -Bignore/build32 -A Win32 -DSTOCHAS_VERSION=${STOCHAS_VERSION}
cmake --build ignore/build32 --config Release
mkdir -p ignore/build/product
mkdir -p zip/x86
mkdir -p zip/x64
cp -r ignore/build/stochas_artefacts/Release/VST3/* zip/x64
cp -r ignore/build/stochas_artefacts/Release/CLAP/* zip/x64
cp -r ignore/build/stochas_artefacts/Release/Standalone/* zip/x64
cp -r ignore/build32/stochas_artefacts/Release/VST3/* zip/x86
cp -r ignore/build32/stochas_artefacts/Release/CLAP/* zip/x86
cp -r ignore/build32/stochas_artefacts/Release/Standalone/* zip/x86
NM=stochas-${STOCHAS_VERSION}.${GH}-win.zip
# see what we have
find ignore/build
powershell Compress-Archive -DestinationPath "ignore/build/product/${NM}" -Force -Path "zip"
