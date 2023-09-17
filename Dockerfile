FROM quay.io/pypa/manylinux2014_x86_64

ARG DEBIAN_FRONTEND=noninteractive

# get pip
RUN curl "https://bootstrap.pypa.io/get-pip.py" -o "get-pip.py" && python3.10 get-pip.py

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
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/DawDreamer/dawdreamer:/DawDreamer/thirdparty/libfaust/ubuntu-x86_64/Release/lib
RUN python3.10 -m venv test-env
RUN /bin/bash -c "source test-env/bin/activate && pip install librosa scipy numpy pytest build wheel && python -m build --wheel && pip install dist/dawdreamer*.whl"

# Don't test because of possible issue with glibc on GitHub Actions
# RUN /bin/bash -c "source test-env/bin/activate && cd tests && pytest -v ."