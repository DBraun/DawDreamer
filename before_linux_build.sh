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
freetype-devel \
libsndfile-devel \
libvorbis-devel \
opus-devel \
flac-devel \
flac-libs
# alsa-lib-devel \
# alsa-utils

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

# make VERBOSE=1 CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L/__w/DawDreamer/DawDreamer/alsa-lib/src -L$pythonLibPath" CXXFLAGS="-I../../alsa-lib/include -I$pythonInclude"
make VERBOSE=1 CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L$pythonLibPath" CXXFLAGS="-I$pythonInclude"
mv build/libdawdreamer.so ../../dawdreamer/dawdreamer.so
cd ../..
cp thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so dawdreamer/libfaust.so
cp thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so dawdreamer/libfaust.so.2

echo "before_linux_build.sh is done!"