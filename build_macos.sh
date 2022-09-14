# check that these are right for your system:
export PYTHONMAJOR=3.9
export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.9

# Below this you shouldn't need to change anything.

# Build Libsamplerate
cd thirdparty/libsamplerate
mkdir build_release
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DLIBSAMPLERATE_EXAMPLES=off -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
make --directory=build_release
cd ../..

# build macOS release
# ARCHS="x86_64 arm64" 
xcodebuild ARCHS="arm64" ONLY_ACTIVE_ARCH=NO -configuration Release -project Builds/MacOSX/DawDreamer.xcodeproj/
mv Builds/MacOSX/build/Release/dawdreamer.so.dylib Builds/MacOSX/build/Release/dawdreamer.so

rm tests/dawdreamer.so
cp Builds/MacOSX/build/Release/dawdreamer.so tests/dawdreamer.so
cp thirdparty/libfaust/darwin-x64/Release/libfaust.a tests/libfaust.2.dylib

# # To make a wheel locally:
# pip install setuptools wheel build delocate
# python3 -m build --wheel
# delocate-listdeps dist/dawdreamer-0.5.9-cp39-cp39-macosx_10_15_universal2.whl 
# delocate-wheel --require-archs x86_64 -w repaired_wheel dist/dawdreamer-0.5.9-cp39-cp39-macosx_10_15_universal2.whl
# pip install repaired_wheel/dawdreamer-0.5.9-cp39-cp39-macosx_10_15_universal2.whl
# cd tests
# python -m pytest -s .