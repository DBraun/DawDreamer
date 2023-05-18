# check that these are right for your system:
export PYTHONMAJOR=3.9
export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.9

export ARCHS="arm64"
export CFLAGS="-arch arm64"
export ARCHFLAGS="-arch arm64"
# export ARCHS="x86_64"
# export CFLAGS="-arch x86_64"
# export ARCHFLAGS="-arch x86_64"

# Below this you shouldn't need to change anything.
CONFIGURATION=Release-$ARCHS

# Build Libsamplerate
cd thirdparty/libsamplerate
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release -DCMAKE_OSX_ARCHITECTURES="$ARCHS" -DLIBSAMPLERATE_EXAMPLES=off -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
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