PYTHONMAJOR="$1"
PYTHONVERSION="$2"

echo "PYTHONMAJOR: $PYTHONMAJOR" 
echo "PYTHONVERSION: $PYTHONVERSION" 

yum install -y libsndfile \
libX11-devel \
libXrandr-devel \
libXinerama-devel \
libXrender-devel \
libXcomposite-devel \
libXinerama-devel \
libXcursor-devel \
libsndfile-devel \
libvorbis-devel \
opus-devel \
flac-devel \
flac-libs \
alsa-lib-devel \
alsa-utils
# freetype-devel

echo "Build libsamplerate"
cd thirdparty/libsamplerate
mkdir build_release
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cd build_release
TARGET_ARCH=x86_64 make CONFIG=Release VERBOSE=1
cd ../../..

CPLUS_INCLUDE_PATH=/usr/include/python$PYTHONVERSION
cd Builds/LinuxMakefile
ldconfig

pythonLibPath=/opt/python/cp$PYTHONMAJOR-cp$PYTHONMAJOR/lib
pythonInclude=/opt/python/cp$PYTHONMAJOR-cp$PYTHONMAJOR/include/python$PYTHONVERSION

echo "pythonLibPath: $pythonLibPath" 
echo "pythonInclude: $pythonInclude" 

# PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/lib64/pkgconfig:/usr/lib/pkgconfig/:/usr/local/include/freetype2/freetype/config:../../alsa-lib/utils:/__w/DawDreamer/DawDreamer/alsa-lib/utils
make VERBOSE=1 CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L/__w/DawDreamer/DawDreamer/alsa-lib/src -L$pythonLibPath" CXXFLAGS="-I../../alsa-lib/include -I$pythonInclude" CFLAGS=-DJUCE_USE_FREETYPE_AMALGAMATED=1
# make VERBOSE=1 CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L/__w/DawDreamer/DawDreamer/alsa-lib/src -L$pythonLibPath" CXXFLAGS="-I../../alsa-lib/include -I../../curl/include -I../../freetype/include -I$pythonInclude"
mv build/libdawdreamer.so ../../dawdreamer/dawdreamer.so
cd ../..
cp thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so dawdreamer/libfaust.so
cp thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so dawdreamer/libfaust.so.2

rm -f thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so
rm -f thirdparty/libfaust/ubuntu-aarch64/lib/libfaust.so
rm -f thirdparty/faust/architecture/android/app/lib/libsndfile/lib/armeabi-v7a/libsndfile.so
rm -f thirdparty/faust/architecture/android/app/lib/libsndfile/lib/arm64-v8a/libsndfile.so
echo "before_linux_build.sh is done!"