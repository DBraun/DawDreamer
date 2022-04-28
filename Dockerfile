FROM quay.io/pypa/manylinux2014_x86_64

ARG DEBIAN_FRONTEND=noninteractive

# get pip
RUN curl "https://bootstrap.pypa.io/get-pip.py" -o "get-pip.py" && python3.9 get-pip.py

# clone repo by copying in
COPY . /DawDreamer

RUN git clone --recursive https://github.com/grame-cncm/faustlibraries.git /DawDreamer/dawdreamer/faustlibraries

WORKDIR /DawDreamer
ENV PYTHONLIBPATH=/opt/python/cp39-cp39/lib
ENV PYTHONINCLUDEPATH=/opt/python/cp39-cp39/include/python3.9
RUN sh -v build_linux.sh

# Setup Python Requirements
WORKDIR /DawDreamer
RUN python3.9 -m pip install librosa scipy numpy pytest build wheel

# Build and install wheel
WORKDIR /DawDreamer
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/DawDreamer/dawdreamer
RUN DISTUTILS_DEBUG=1 python3.9 /DawDreamer/setup.py install

# todo: ideally we could remove the files here
# RUN rm -rf /DawDreamer/dawdreamer/*.so*

# Run all Tests
WORKDIR /DawDreamer/tests
RUN python3.9 -m pytest -v .