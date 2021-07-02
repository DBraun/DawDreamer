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
    faust \
    libsamplerate0 \
    llvm-11 \
    llvm-11-dev \
&& update-ca-certificates \
&& apt-get clean -y

# clone repo
RUN git clone --recursive https://github.com/DBraun/DawDreamer.git
## or copy:
# WORKDIR /DawDreamer
# COPY . .

# Make symlinks to use during building DawDreamer
RUN ln -s /usr/bin/llvm-config-11 /usr/bin/llvm-config
RUN ln -s /usr/lib/x86_64-linux-gnu/libsamplerate.so.0 /usr/local/lib/libsamplerate.so

# Build DawDreamer
WORKDIR /DawDreamer/Builds/LinuxMakefile
ENV CPLUS_INCLUDE_PATH=/usr/include/python3.9/
RUN ldconfig
RUN make CONFIG=Release
RUN cp /DawDreamer/Builds/LinuxMakefile/build/libdawdreamer.so /DawDreamer/tests/dawdreamer.so

# Basic Import Test
WORKDIR /DawDreamer/tests
RUN python3.9 -c "import dawdreamer; print('DawDreamer was successfully imported in python3.')"

# Pytest Full Test
#RUN apt install python3-pip
#RUN python3.9 -m pip install librosa scipy numpy pytest
#WORKDIR /DawDreamer/tests
#RUN python3.9 -m pytest .