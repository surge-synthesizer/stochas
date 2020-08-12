#
# This will build the mac install and disk image.
# you will need to have "Packages 1.2.9" installed on your system for this to work correctly
# Be sure to run this from the project's root directory
set -e
STOCHAS_VERSION=`cat VERSION`
cmake -Bbuild -GXcode -DSTOCHAS_VERSION=${STOCHAS_VERSION} 
cmake --build build --target stochas_VST3 --config Release
cmake --build build --target stochas_Standalone --config Release
cmake -Bbuild -DSTOCHAS_IS_SYNTH=FALSE -DSTOCHAS_VERSION=${STOCHAS_VERSION} 
cmake --build build --target stochas_AU --config Release
if [ "$1" == "install" ]; then
   echo building package...
   packagesbuild "install/mac/Stochas.pkgproj"
   echo creating disk image...
   hdiutil create -volname "Stochas ${STOCHAS_VERSION}" -srcfolder build.install/mac/stochas -ov -format UDRO build.install/mac/stochas-mac-install.dmg
fi
