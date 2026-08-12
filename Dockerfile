FROM quay.io/pypa/manylinux_2_34_x86_64

ARG DEBIAN_FRONTEND=noninteractive

# Use the manylinux-provided CPython (it ships with pip)
ENV PYTHON=/opt/python/cp312-cp312/bin/python
ENV PYTHONLIBPATH=/opt/python/cp312-cp312/lib
ENV PYTHONINCLUDEPATH=/opt/python/cp312-cp312/include/python3.12

# clone repo by copying in
COPY . /DawDreamer

WORKDIR /DawDreamer/thirdparty/libfaust
RUN $PYTHON download_libfaust.py

WORKDIR /DawDreamer

# Install build dependencies
RUN yum install -y \
    ncurses-devel \
    libX11-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXrender-devel \
    libXcomposite-devel \
    libXcursor-devel \
    freetype-devel \
    alsa-lib-devel

# Build libsamplerate
RUN cd thirdparty/libsamplerate && \
    cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release -DCMAKE_POSITION_INDEPENDENT_CODE=ON && \
    cmake --build build_release --config Release && \
    cd ../..

# Build DawDreamer
RUN cd Builds/LinuxMakefile && \
    make CONFIG=Release LIBS="-lstdc++fs" LDFLAGS="-L$PYTHONLIBPATH" CXXFLAGS="-I$PYTHONINCLUDEPATH" && \
    strip --strip-unneeded build/libdawdreamer.so && \
    cp build/libdawdreamer.so ../../dawdreamer/dawdreamer.so && \
    cd ../..

# Setup python virtual environment and requirements
WORKDIR /DawDreamer
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/DawDreamer/dawdreamer:/DawDreamer/thirdparty/libfaust/ubuntu-x86_64/Release/lib
RUN $PYTHON -m venv test-env
#RUN /bin/bash -c "source test-env/bin/activate && pip install -r test-requirements.txt && python -m build --wheel && pip install dist/dawdreamer*.whl && cd tests && pytest -v ."
