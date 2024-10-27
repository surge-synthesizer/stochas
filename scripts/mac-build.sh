#!/bin/bash
# This is the build steps used in the releae pipeline

if [ ! -f lib/sst-plugininfra/scripts/installer_mac/make_installer.sh ]; then
   echo "Cant find installer mac. Please run from source dir and update subomdules"
   die
fi

cmake -Bignore/build -GNinja -DCMAKE_BUILD_TYPE=Release -DSTOCHAS_VERSION=${STOCHAS_VERSION} -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build ignore/build --target stochas_VST3 --config Release
cmake --build ignore/build --target stochas_CLAP --config Release
cmake --build ignore/build --target stochas_Standalone --config Release

mkdir -p ignore/mac_assets
cp -r ignore/build/stochas_artefacts/VST3/* ignore/mac_assets
cp -r ignore/build/stochas_artefacts/CLAP/* ignore/mac_assets
cp -r ignore/build/stochas_artefacts/Standalone/* ignore/mac_assets

cmake -GNinja -Bignore/build_au -DCMAKE_BUILD_TYPE=Release -DSTOCHAS_IS_SYNTH=FALSE -DSTOCHAS_VERSION=${STOCHAS_VERSION} -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build ignore/build_au --target stochas_AU --config Release
cp -r ignore/build_au/stochas_artefacts/AU/* ignore/mac_assets

mkdir -p ignore/mac_installer
lib/sst-plugininfra/scripts/installer_mac/make_installer.sh Stochas ignore/mac_assets resources ignore/mac_installer ${STOCHAS_VERSION}

mkdir product
mv ignore/mac_installer/*dmg product
      

