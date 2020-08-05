#
# This will build the mac install and disk image.
# you will need to have "Packages 1.2.9" installed on your system for this to work correctly
# Be sure to run this from the project's root directory
STOCHAS_VERSION=`cat VERSION`
cmake -B build -DSTOCHAS_VERSION=${STOCHAS_VERSION}
cmake --build build --config Release
retVal=$?
if [ $retVal -eq 0 ]; then
  echo building package...
  packagesbuild "install/mac/Stochas Open.pkgproj"
  echo creating disk image...
  hdiutil create -volname "Stochas ${STOCHAS_VERSION}" -srcfolder build.install/mac/stochas_open -ov -format UDRO build.install/mac/stochas.${STOCHAS_VERSION}.dmg
fi
