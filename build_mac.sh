#
# This will build the mac install and disk image.
# you will need to have "Packages 1.2.9" installed on your system for this to work correctly
# Be sure to run this from the project's root directory
STOCHAS_VERSION=`cat VERSION`
cmake -B build -DSTOCHAS_VERSION=${STOCHAS_VERSION} -DSTOCHAS_IS_SYNTH=TRUE -DSTOCHAS_IS_MIDI_EFFECT=TRUE
cmake --build build --config Release
retVal=$?
if [ $retVal -eq 0 ]; then
  echo building package...
  packagesbuild "install/mac/Stochas.pkgproj"
  echo creating disk image...
  hdiutil create -volname "Stochas ${STOCHAS_VERSION}" -srcfolder build.install/mac/stochas -ov -format UDRO build.install/mac/stochas.${STOCHAS_VERSION}.dmg
fi
