if [ -z "$PYTHONMAJOR" ]; then
  echo "Build failed. You must set the environment variable PYTHONMAJOR to a value such as 3.11"
  exit 1
fi

if [ -z "$pythonLocation" ]; then
  echo "Build failed. You must set the environment variable pythonLocation to a value such as /Library/Frameworks/Python.framework/Versions/3.11"
  exit 1
fi

if [[ $(uname -m) == 'arm64' ]]; then
    export ARCHS="arm64"
    export CFLAGS="-arch arm64"
    export ARCHFLAGS="-arch arm64"
else
    export ARCHS="x86_64"
    export CFLAGS="-arch x86_64"
    export ARCHFLAGS="-arch x86_64"
fi

# Below this you shouldn't need to change anything.
CONFIGURATION=Release-$ARCHS

# Build Libsamplerate
cd thirdparty/libsamplerate
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release -DCMAKE_OSX_ARCHITECTURES="$ARCHS" -DLIBSAMPLERATE_EXAMPLES=off -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
make --directory=build_release
cd ../..

# build macOS release
xcodebuild ARCHS="$ARCHS" -configuration Release-$ARCHS -project Builds/MacOSX/DawDreamer.xcodeproj/ CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED="NO" CODE_SIGN_ENTITLEMENTS="" CODE_SIGNING_ALLOWED="NO"
mv Builds/MacOSX/build/$CONFIGURATION/dawdreamer.so.dylib Builds/MacOSX/build/$CONFIGURATION/dawdreamer.so

rm tests/dawdreamer.so
cp Builds/MacOSX/build/$CONFIGURATION/dawdreamer.so tests/dawdreamer.so

# # To make a wheel locally:
# pip install setuptools wheel build delocate
# python3 -m build --wheel
# delocate-listdeps dist/dawdreamer-0.5.9-cp39-cp39-macosx_10_15_universal2.whl 
# delocate-wheel --require-archs x86_64 -w repaired_wheel dist/dawdreamer-0.5.9-cp39-cp39-macosx_10_15_universal2.whl
# pip install repaired_wheel/dawdreamer-0.5.9-cp39-cp39-macosx_10_15_universal2.whl
# cd tests
# python -m pytest -s .