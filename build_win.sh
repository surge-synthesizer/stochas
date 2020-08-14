#
# Run from git bash shell
# install target
#
# Inno setup path
#
#INNO="/c/Program\ Files\ \(x86\)/Inno\ Setup\ 5/Compil32.exe"
INNO="/c/Program Files (x86)/Inno Setup 5/iscc.exe"
VST2_PATH=/Z/dev/VST_SDK/VST2_SDK
STOCHAS_VERSION=`cat VERSION`
cmake -B build -A x64 -DSTOCHAS_VERSION=${STOCHAS_VERSION} -DVST2_PATH=${VST2_PATH} -DSTOCHAS_IS_SYNTH=TRUE -DSTOCHAS_IS_MIDI_EFFECT=TRUE
cmake --build build --config Release
retVal=$?
if [ $retVal -eq 0 ]; then
   cmake -B build32 -A Win32 -DSTOCHAS_VERSION=${STOCHAS_VERSION} -DVST2_PATH=${VST2_PATH} -DSTOCHAS_IS_SYNTH=FALSE -DSTOCHAS_IS_MIDI_EFFECT=TRUE
   cmake --build build32 --config Release

  retVal=$?
  if [ $retVal -eq 0 ] && [ "$1" == "install" ]; then
    "$INNO" //Obuild.install\\win //Fstochas_windows_install //DSTOCHAS_VERSION=${STOCHAS_VERSION} "install\\win\\install.iss"
  fi
fi