#!/bin/bash

cmake -Bbuild -GNinja -DSTOCHAS_VERSION=${STOCHAS_VERSION} -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
LINARCH=`uname -m`
GH=`git log -1 --format=%h`
NM=stochas-${STOCHAS_VERSION}.${GH}.linux-${LINARCH}.tgz
mkdir -p build/product/
mkdir -p build/Stochas/Standalone
cp -r build/stochas_artefacts/VST3/* build/Stochas
cp -r build/stochas_artefacts/CLAP/* build/Stochas
cp -r build/stochas_artefacts/Standalone/* build/Stochas/Standalone
tar cvzf "build/product/${NM}" -C build Stochas

mkdir product
mv build/product/* product