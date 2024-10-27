#!/bin/bash

set -ex
# 64 bit
cmake -Bbuild -A x64 -DSTOCHAS_VERSION=${STOCHAS_VERSION} -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# 32 bit
mkdir -p build/product
mkdir -p zip/x64
cp -r build/stochas_artefacts/Release/VST3/* zip/x64
cp -r build/stochas_artefacts/Release/CLAP/* zip/x64
cp -r build/stochas_artefacts/Release/Standalone/* zip/x64
NM=stochas-${STOCHAS_VERSION}.${GH}-win.zip
# see what we have
powershell Compress-Archive -DestinationPath "build/product/${NM}" -Force -Path "zip"
ls build/product

nuget install innosetup
mkdir -p build/product/
iscc //Obuild\\product //Fstochas_windows_installer //DSTOCHAS_VERSION=${STOCHAS_VERSION}.${GH} "install\\win\\install.iss"

mkdir -p product
mv build/product/* product
