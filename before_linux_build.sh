PYTHONMAJOR="$1"
PYTHONVERSION="$2"

# PYTHONMAJOR is "37", "38", "39", "310" etc.
# PYTHONVERSION is "3.7", "3.8", "3.9", "3.10" etc.

echo "PYTHONMAJOR: $PYTHONMAJOR" 
echo "PYTHONVERSION: $PYTHONVERSION"

if [[ "$PYTHONMAJOR" == "37" ]]; then
	pythonLibPath=/opt/python/cp37-cp37m/lib
	pythonInclude=/opt/python/cp37-cp37m/include/python3.7m
else
	pythonLibPath=/opt/python/cp$PYTHONMAJOR-cp$PYTHONMAJOR/lib
	pythonInclude=/opt/python/cp$PYTHONMAJOR-cp$PYTHONMAJOR/include/python$PYTHONVERSION
fi

echo "pythonLibPath: $pythonLibPath"
echo "pythonInclude: $pythonInclude"

cp -v thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so.2 dawdreamer/
ln dawdreamer/libfaust.so.2 dawdreamer/libfaust.so
ln thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so.2 thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so

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
TARGET_ARCH=x86_64 make CONFIG=Release
cd ../../..

CPLUS_INCLUDE_PATH=/usr/include/python$PYTHONVERSION
cd Builds/LinuxMakefile
ldconfig

# faust/architecture/faust/midi/RtMidi.cpp has #include <alsa/asoundlib.h>
make VERBOSE=1 CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L/__w/DawDreamer/DawDreamer/alsa-lib/src -L$pythonLibPath" CXXFLAGS="-I../../alsa-lib/include -I$pythonInclude"
mv build/libdawdreamer.so ../../dawdreamer/dawdreamer.so
cd ../..

echo "before_linux_build.sh is done!"