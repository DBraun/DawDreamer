#!/bin/sh

if [ "$(uname)" = "Darwin" ]; then
    echo "You are running macOS"
    curl -L https://github.com/grame-cncm/faust/releases/download/2.59.5/Faust-2.59.5-arm64.dmg -o Faust-2.59.5-arm64.dmg
    hdiutil attach Faust-2.59.5-arm64.dmg
    mkdir -p darwin-arm64/Release
    cp -R /Volumes/Faust-2.59.5/Faust-2.59.5/* darwin-arm64/Release/
    hdiutil detach /Volumes/Faust-2.59.5/

    curl -L https://github.com/grame-cncm/faust/releases/download/2.59.5/Faust-2.59.5-x64.dmg -o Faust-2.59.5-x64.dmg
    hdiutil attach Faust-2.59.5-x64.dmg
    mkdir -p darwin-x64/Release
    cp -R /Volumes/Faust-2.59.5/Faust-2.59.5/* darwin-x64/Release/
    hdiutil detach /Volumes/Faust-2.59.5/
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    echo "You are running Linux"
    curl -L https://github.com/grame-cncm/faust/releases/download/2.59.5/libfaust-ubuntu-x86_64.zip -o libfaust-ubuntu-x86_64.zip
    mkdir -p ubuntu-x86_64/lib
    unzip libfaust-ubuntu-x86_64.zip -d ubuntu-x86_64/lib
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" = "MINGW64_NT" ]; then
    echo "You are running Windows"
    # curl -L https://github.com/grame-cncm/faust/releases/download/2.59.5/Faust-2.59.5-win64.exe -o Faust-2.59.5-win64.exe
    # ./Faust-2.59.5-win64.exe /S /D=%cd%\win64
else
    echo "Unknown operating system"
fi