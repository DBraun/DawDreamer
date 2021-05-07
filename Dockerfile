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
    python3 \
    python3-dev \
&& update-ca-certificates \
&& apt-get clean -y

RUN git clone --recursive https://github.com/guillaumephd/DawDreamer.git

# Build
WORKDIR DawDreamer/Builds/LinuxMakefile
ENV CPLUS_INCLUDE_PATH=/usr/include/python3.6m/
RUN ldconfig
RUN make
RUN mv build/libdawdreamer.so build/dawdreamer.so

# Test
RUN cp build/dawdreamer.so /
WORKDIR /
RUN python3 -c "import dawdreamer; print('DawDreamer was successfully imported in python3.')"
