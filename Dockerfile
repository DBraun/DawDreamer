FROM quay.io/pypa/manylinux2014_x86_64

ARG DEBIAN_FRONTEND=noninteractive

# get pip
RUN curl "https://bootstrap.pypa.io/get-pip.py" -o "get-pip.py" && python3.9 get-pip.py

# clone repo by copying in
COPY . /DawDreamer

WORKDIR /DawDreamer/thirdparty/libfaust
RUN sh download_libfaust.sh

WORKDIR /DawDreamer
ENV PYTHONLIBPATH=/opt/python/cp39-cp39/lib
ENV PYTHONINCLUDEPATH=/opt/python/cp39-cp39/include/python3.9
RUN sh -v build_linux.sh

# Setup Python Requirements
WORKDIR /DawDreamer
RUN python3.9 -m pip install librosa scipy numpy pytest build wheel

# Build wheel
WORKDIR /DawDreamer
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/DawDreamer/dawdreamer:/DawDreamer/thirdparty/libfaust/ubuntu-x86_64/Release/lib
RUN python3.9 -m build --wheel

# Install wheel
WORKDIR /DawDreamer
RUN python3.9 -m pip install dist/dawdreamer*.whl

# Run all Tests
WORKDIR /DawDreamer/tests
RUN python3.9 -m pytest -v .