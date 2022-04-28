FROM ubuntu:21.04 as ubuntu_base

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update -yq \
&& apt-get install -yq --no-install-recommends \
    ca-certificates \
    build-essential \
    clang \
    pkg-config \
    libboost-all-dev \
    libboost-python-dev \
    libfreetype6-dev \
    libx11-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxcursor-dev \
    mesa-common-dev \
    libasound2-dev \
    freeglut3-dev \
    libxcomposite-dev \
    libcurl4-gnutls-dev \
    git \
    cmake \
    python3 \
    python3.9-dev \
    libsamplerate0 \
    libsndfile1 \
    libncurses5 \
    libncursesw5 \
    libncurses5-dev \
    libncursesw5-dev \
&& update-ca-certificates \
&& apt-get clean -y

# clone repo by copying in
COPY . /DawDreamer

RUN git clone --recursive https://github.com/grame-cncm/faustlibraries.git /DawDreamer/dawdreamer/faustlibraries

# Make symlinks to use during building DawDreamer
RUN ln -s /usr/lib/x86_64-linux-gnu/libsamplerate.so.0 /usr/local/lib/libsamplerate.so
RUN ln -s /usr/lib/x86_64-linux-gnu/libncurses.so.6 /usr/lib/x86_64-linux-gnu/libncurses.so.5
RUN ln -s /usr/lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5
RUN ln -s /usr/lib/x86_64-linux-gnu/libform.so.6 /usr/lib/x86_64-linux-gnu/libform.so.5

RUN ldd /DawDreamer/thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so
RUN cp /DawDreamer/thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so /DawDreamer/dawdreamer/libfaust.so
RUN cp /DawDreamer/thirdparty/libfaust/ubuntu-x86_64/lib/libfaust.so /DawDreamer/dawdreamer/libfaust.so.2

# Build DawDreamer
WORKDIR /DawDreamer/Builds/LinuxMakefile
ENV CPLUS_INCLUDE_PATH=/usr/include/python3.9/
RUN ldconfig
RUN make VERBOSE=1 CONFIG=Release
RUN cp /DawDreamer/Builds/LinuxMakefile/build/libdawdreamer.so /DawDreamer/dawdreamer/dawdreamer.so
RUN ldd /DawDreamer/dawdreamer/dawdreamer.so

# Setup Python Requirements
WORKDIR /DawDreamer
RUN apt install -y python3-pip
RUN python3.9 -m pip install librosa scipy numpy pytest build wheel

# Build and install wheel
WORKDIR /DawDreamer
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/x86_64-linux-gnu/:/DawDreamer/dawdreamer
RUN echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
RUN ldconfig -v
RUN DISTUTILS_DEBUG=1 python3.9 /DawDreamer/setup.py install

# Run all Tests
WORKDIR /DawDreamer/tests
RUN ldconfig -v
RUN python3.9 -m pytest -v .