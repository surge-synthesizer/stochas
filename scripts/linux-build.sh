#!/bin/bash

cmake -Bignore/build -DSTOCHAS_VERSION=${STOCHAS_VERSION}
cmake --build ignore/build --config Release
LINARCH=`uname -m`
GH=`git log -1 --format=%h`
NM=stochas-${STOCHAS_VERSION}.${GH}.linux-${LINARCH}.tgz
mkdir -p ignore/build/product/
mkdir -p ignore/build/Stochas/Standalone
cp -r ignore/build/stochas_artefacts/VST3/* build/Stochas
cp -r ignore/build/stochas_artefacts/CLAP/* build/Stochas
cp -r ignore/build/stochas_artefacts/Standalone/* build/Stochas/Standalone
tar cvzf "build/product/${NM}" -C build Stochas