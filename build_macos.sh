# Build Libsamplerate
cd thirdparty/libsamplerate
mkdir build_release
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DLIBSAMPLERATE_EXAMPLES=off -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
make --directory=build_release
cd ../..

# build macOS release
export PYTHONMAJOR=3.9
export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.9
xcodebuild -configuration Release -project Builds/MacOSX/DawDreamer.xcodeproj/
mv Builds/MacOSX/build/Release/dawdreamer.so.dylib Builds/MacOSX/build/Release/dawdreamer.so
# otool -L Builds/MacOSX/build/Release/dawdreamer.so
install_name_tool -change @rpath/libfaust.2.dylib @loader_path/libfaust.2.dylib Builds/MacOSX/build/Release/dawdreamer.so
# otool -L Builds/MacOSX/build/Release/dawdreamer.so

# codesigning
# Open Keychain Access. Go to "login". Look for "Apple Development".
# run `export CODESIGN_IDENTITY="Apple Development: example@example.com (ABCDE12345)"` with your own info substituted.
codesign --force --deep --sign "$CODESIGN_IDENTITY" Builds/MacOSX/build/Release/dawdreamer.so

# # Confirm the codesigning
codesign -vvvv Builds/MacOSX/build/Release/dawdreamer.so

rm tests/dawdreamer.so
cp Builds/MacOSX/build/Release/dawdreamer.so tests/dawdreamer.so

# # To make a wheel locally:
# export MACOSX_DEPLOYMENT_TARGET=10.15
# pip install setuptools wheel build
# python3 -m build --wheel
