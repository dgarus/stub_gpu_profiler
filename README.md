# stub_gpu profiler

A minimal out-of-tree profiler for PyTorch `PrivateUse1`. It creates three synthetic events and exports them to a Chrome/Perfetto trace through the standard Kineto/`ActivityLogger` path.

No NVIDIA GPU, CUDA Toolkit, or CUPTI access is required: all events are fake. The Docker image installs the CPU-only PyTorch wheel, so Kineto does not attempt to initialize CUDA or CUPTI.

## Installed dependencies

The Docker image is based on `python:3.11-slim` and installs:

- `build-essential` and `cmake` — the C/C++ toolchain;
- `ninja-build` and `ninja` — the C++ extension build system;
- `git`;
- `torch` — the CPU-only PyTorch wheel, including the required Kineto headers and libraries;
- `setuptools<82`, `wheel`, and `numpy`.

PyTorch `2.12.1+cpu` is installed from the official CPU wheel index:

```text
https://download.pytorch.org/whl/cpu
```

Pinning the version keeps the private Kineto API used by this example reproducible. The image build verifies that `torch.version.cuda is None` and fails immediately if a CUDA-enabled wheel is installed.

No separate system `fmt` library is installed. The extension is compiled with `-DFMT_HEADER_ONLY` and uses the headers bundled with PyTorch. The `kineto/libkineto.h` header provides the complete `CpuTraceBuffer` definition required by the current PyTorch wheel.

After starting the container, install and build the project with:

```bash
python -m pip install -e . --no-build-isolation
```

This builds the Linux `torch_stub_gpu._C` extension and installs the package in editable mode. The `--no-build-isolation` option allows the build to use the PyTorch installation and headers already available in the image.

## Build the image

Run these commands from the `stub_gpu_profiler` project directory:

```bash
cd stub_gpu_profiler
docker build -t stub-gpu-profiler .
```

## Start the container

Linux/macOS:

```bash
docker run -it \
  --name stub_gpu_profiler \
  -v "$PWD":/workspace/stub_gpu \
  -w /workspace/stub_gpu \
  stub-gpu-profiler
```

The project is attached through a bind mount, so the compiled extension and `stub_gpu_trace.json` are also available on the host machine.

## Build the extension

Inside the container:

```bash
python -m pip install -e . --no-build-isolation
```

A successful build ends with output similar to:

```text
Successfully built torch_stub_gpu
Successfully installed torch_stub_gpu-0.0.0
```

Run the same command to rebuild the extension after changing the C++ code.

## Smoke test

Inside the container:

```bash
python smoke_test.py
```

The script automatically:

1. runs the CPU + `PrivateUse1` profiler;
2. exports `stub_gpu_trace.json`;
3. parses the file with the standard JSON parser;
4. verifies that `traceEvents` is an array containing all three synthetic events.

Expected output:

```text
[stub_gpu] profiler start
[stub_gpu] profiler stop
[stub_gpu] processTrace
trace exported: stub_gpu_trace.json
found: ['stubGpuLaunchKernel', 'stubGpuMemcpyHtoD', 'stub_gpu_hello_kernel']
missing: []
trace validation passed
```

If the JSON is malformed, `traceEvents` has the wrong type, or at least one event is missing, the script exits with a non-zero status. Therefore, `python smoke_test.py` is sufficient both to run the profiler and validate its output.

The CPU-only PyTorch build does not initialize CUDA or CUPTI, so CUDA/CUPTI warnings are not expected. The generated `stub_gpu_trace.json` can be opened in the Perfetto UI or Chrome Trace Viewer.

## Complete workflow

On the host machine:

```bash
cd stub_gpu
docker build -t stub-gpu-profiler .
docker run --rm -it \
  --name stub_gpu_profiler \
  -v "$PWD":/workspace/stub_gpu \
  -w /workspace/stub_gpu \
  stub-gpu-profiler
```

Then, inside the container:

```bash
python -m pip install -e . --no-build-isolation
python smoke_test.py
```
