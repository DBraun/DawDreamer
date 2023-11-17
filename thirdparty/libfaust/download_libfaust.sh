#!/bin/sh

VERSION=2.69.3

if [ "$(uname)" = "Darwin" ]; then
    echo "You are running macOS"
    curl -L https://github.com/grame-cncm/faust/releases/download/$VERSION/Faust-$VERSION-arm64.dmg -o Faust-$VERSION-arm64.dmg
    hdiutil attach Faust-$VERSION-arm64.dmg
    mkdir -p "darwin-arm64/Release"
    cp -R /Volumes/Faust-$VERSION/Faust-$VERSION/* darwin-arm64/Release/
    hdiutil detach /Volumes/Faust-$VERSION/

    curl -L https://github.com/grame-cncm/faust/releases/download/$VERSION/Faust-$VERSION-x64.dmg -o Faust-$VERSION-x64.dmg
    hdiutil attach Faust-$VERSION-x64.dmg
    mkdir -p "darwin-x64/Release"
    cp -R /Volumes/Faust-$VERSION/Faust-$VERSION/* darwin-x64/Release/
    hdiutil detach /Volumes/Faust-$VERSION/
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    echo "You are running Linux"
    curl -L https://github.com/grame-cncm/faust/releases/download/$VERSION/libfaust-ubuntu-x86_64.zip -o libfaust-ubuntu-x86_64.zip
    mkdir -p "ubuntu-x86_64/Release"
    unzip libfaust-ubuntu-x86_64.zip -d ubuntu-x86_64/Release
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" = "MINGW64_NT" ]; then
    echo "You are running Windows. You should run download_libfaust.bat"
    exit
else
    echo "Unknown operating system"
fi