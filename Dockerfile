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
RUN python -m pip install "setuptools<82" wheel ninja numpy
RUN python -m pip install "torch==2.12.1+cpu" \
    --index-url https://download.pytorch.org/whl/cpu
RUN python -c "import torch; assert torch.version.cuda is None, f'expected CPU-only PyTorch, got CUDA {torch.version.cuda}'; print(f'installed CPU-only PyTorch {torch.__version__}')"
    
CMD ["/bin/bash"]