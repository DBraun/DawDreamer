FROM quay.io/pypa/manylinux2014_x86_64

ARG DEBIAN_FRONTEND=noninteractive

# get pip
RUN curl "https://bootstrap.pypa.io/get-pip.py" -o "get-pip.py" && python3.9 get-pip.py

# clone repo by copying in
COPY . /DawDreamer

RUN git clone --recursive https://github.com/grame-cncm/faustlibraries.git /DawDreamer/dawdreamer/faustlibraries

WORKDIR /DawDreamer
RUN sh -v before_linux_build.sh "39" "3.9"

# Setup Python Requirements
WORKDIR /DawDreamer
RUN python3.9 -m pip install librosa scipy numpy pytest build wheel

# Build and install wheel
WORKDIR /DawDreamer
RUN DISTUTILS_DEBUG=1 python3.9 /DawDreamer/setup.py install

# Run all Tests
WORKDIR /DawDreamer/tests
# ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/DawDreamer/dawdreamer
RUN python3.9 -m pytest -v .