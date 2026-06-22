FROM python:3.11-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    ninja-build \
    git \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace/stub_gpu

COPY . .

RUN python -m pip install -U pip
RUN python -m pip install torch "setuptools<82" wheel ninja numpy
    
CMD ["/bin/bash"]