FROM quay.io/pypa/manylinux2014_x86_64

ARG DEBIAN_FRONTEND=noninteractive

# clone repo by copying in
COPY . /DawDreamer

WORKDIR /DawDreamer/thirdparty/libfaust
RUN sh download_libfaust.sh

WORKDIR /DawDreamer
ENV PYTHONLIBPATH=/opt/python/cp310-cp310/lib
ENV PYTHONINCLUDEPATH=/opt/python/cp310-cp310/include/python3.10
RUN sh -v build_linux.sh

# Setup python virtual environment and requirements
WORKDIR /DawDreamer
RUN python3.10 -m venv test-env
RUN source test-env/bin/activate
RUN python -m pip install librosa scipy numpy pytest build wheel

# Build wheel
WORKDIR /DawDreamer
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/DawDreamer/dawdreamer:/DawDreamer/thirdparty/libfaust/ubuntu-x86_64/Release/lib
RUN python -m build --wheel

# Install wheel
WORKDIR /DawDreamer
RUN python -m pip install dist/dawdreamer*.whl

# Run all Tests
WORKDIR /DawDreamer/tests
RUN python -m pytest -v .