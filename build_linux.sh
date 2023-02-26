echo "PYTHONLIBPATH: $PYTHONLIBPATH"
echo "PYTHONINCLUDEPATH: $PYTHONINCLUDEPATH"

if [[ $(uname -m) == 'x86_64' ]]; then
    echo "building for x86_64"
	cp thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so.2
else
    echo "building for $(uname -m)" 
	cp thirdparty/libfaust/ubuntu-aarch64/lib/libfaust.so thirdparty/libfaust/ubuntu-aarch64/lib/libfaust.so.2
fi

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
make CONFIG=Release
cd ../../..

cd Builds/LinuxMakefile
ldconfig

# faust/architecture/faust/midi/RtMidi.cpp has #include <alsa/asoundlib.h>
make VERBOSE=1 CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L/__w/DawDreamer/DawDreamer/alsa-lib/src -L$PYTHONLIBPATH" CXXFLAGS="-I../../alsa-lib/include -I$PYTHONINCLUDEPATH"
mv build/libdawdreamer.so ../../dawdreamer/dawdreamer.so
cd ../..

echo "build_linux.sh is done!"