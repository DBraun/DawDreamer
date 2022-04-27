echo "pythonLocation: " $pythonLocation
yum install -y libsndfile \
libX11-devel \
libXrandr-devel \
libXinerama-devel \
libXrender-devel \
libXcomposite-devel \
libXinerama-devel \
libXcursor-devel \
freetype-devel \
libsndfile-devel \
libvorbis-devel \
opus-devel \
flac-devel \
flac-libs \
alsa-lib-devel \
alsa-utils

echo "Build libsamplerate"
cd thirdparty/libsamplerate
mkdir build_release
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cd build_release
TARGET_ARCH=x86_64 make CONFIG=Release VERBOSE=1
cd ../../..

cd Builds/LinuxMakefile
ldconfig
PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/lib64/pkgconfig:/usr/lib/pkgconfig/:/usr/local/include/freetype2/freetype/config:../../alsa-lib/utils:/__w/DawDreamer/DawDreamer/alsa-lib/utils
make CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L/__w/DawDreamer/DawDreamer/alsa-lib/src" CXXFLAGS="-I../../alsa-lib/include -I../../curl/include -I../../freetype/include"
mv build/libdawdreamer.so ../../dawdreamer/dawdreamer.so
cd ../..
cp thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so dawdreamer/libfaust.so