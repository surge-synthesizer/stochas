#
# Run from git bash shell
# CMAKE_INSTALL_PREFIX should reflect where JUCE was installed using JUCE's cmake
# install target
#
# Inno setup path
#
#export INNO="/c/Program\ Files\ \(x86\)/Inno\ Setup\ 5/Compil32.exe"
export INNO="/c/Program Files (x86)/Inno Setup 5/iscc.exe"
export VST2_PATH=/Z/dev/VST_SDK/VST2_SDK
export STOCHAS_VERSION=`cat VERSION`
cmake -B build32 -DCMAKE_INSTALL_PREFIX=/c/data/juce6_install
cmake --build build32 --config Release
cmake -B build -A x64 -DCMAKE_INSTALL_PREFIX=/c/data/juce6_install
cmake --build build --config Release
"$INNO" //Obuild.install\\win "install\\win\\install.iss"
