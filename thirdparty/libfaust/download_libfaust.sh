#!/bin/sh

if [ "$(uname)" = "Darwin" ]; then
    echo "You are running macOS"
    # curl -L https://github.com/DBraun/faust/suites/12075724181/artifacts/635656964 -o faust-2.58.13-arm64.dmg.zip
    # unzip -o faust-2.58.13-arm64.dmg.zip

    # hdiutil attach faust-2.58.13-arm64.dmg
    mkdir -p darwin-arm64/Release
    cp -R /Volumes/Faust-2.58.13/Faust-2.58.13/* darwin-arm64/Release/
    # hdiutil detach /Volumes/Faust-2.58.13/
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    echo "You are running Linux"
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" = "MINGW64_NT" ]; then
    echo "You are running Windows"
else
    echo "Unknown operating system"
fi