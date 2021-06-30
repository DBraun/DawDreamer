FROM ubuntu:18.04 as ubuntu_base

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
    python3.8 \
    python3.8-dev \
&& update-ca-certificates \
&& apt-get clean -y

# additional install requirements for Faust
RUN apt-get install -yq --no-install-recommends libmicrohttpd-dev llvm-10 llvm-10-dev libssl-dev ncurses-dev libsndfile-dev

# Copy DawDreamer repo
#RUN git clone --recursive https://github.com/DBraun/DawDreamer.git
#COPY . /DawDreamer/
#WORKDIR /DawDreamer
COPY . .
#RUN mkdir /DawDreamer/
#ADD ./* /DawDreamer/
#RUN ls

# Make libsamplerate
WORKDIR /thirdparty/libsamplerate/build
RUN cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release ..
RUN make
RUN cp /thirdparty/libsamplerate/build/src/libsamplerate.so.0 /usr/local/lib/libsamplerate.so.0
RUN cp /thirdparty/libsamplerate/build/src/libsamplerate.so /usr/local/lib/libsamplerate.so

# Make symlink for Faust to use during building
RUN ln -s /usr/bin/llvm-config-10 /usr/bin/llvm-config

# Build Faust
WORKDIR /thirdparty/faust
RUN LLVM_DIR=/usr/lib/llvm-10/lib/cmake/llvm
RUN USE_LLVM_CONFIG=off
RUN make world
RUN make install

WORKDIR /Builds/LinuxMakefile
ENV CPLUS_INCLUDE_PATH=/usr/include/python3.6m/
RUN ldconfig
RUN make CONFIG=Release
RUN cp /Builds/LinuxMakefile/build/libdawdreamer.so /tests/dawdreamer.so

# Basic Import Test
#WORKDIR /tests
#RUN python3.8 -c "import dawdreamer; print('DawDreamer was successfully imported in python3.')"

# Pytest Full Test
#RUN apt install python3-pip
#RUN python3.8 -m pip install librosa scipy numpy pytest
#WORKDIR /tests
#RUN python3.8 -m pytest .