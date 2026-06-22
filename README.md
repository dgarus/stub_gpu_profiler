# stub_gpu profiler

Минимальный out-of-tree profiler для PyTorch `PrivateUse1`. Он создаёт три synthetic-события и экспортирует их штатным путём Kineto/`ActivityLogger` в Chrome/Perfetto trace.

Для запуска не нужны NVIDIA GPU, CUDA Toolkit или доступ к CUPTI: все события fake. Если установленный PyTorch пытается инициализировать CUPTI, предупреждения об отсутствии CUDA можно игнорировать.

## Что устанавливается

Docker-образ основан на `python:3.11-slim` и устанавливает:

- `build-essential`, `cmake` — C/C++ toolchain;
- `ninja-build`, `ninja` — сборщик C++ extension;
- `git`;
- `torch` — PyTorch вместе с используемыми Kineto headers и libraries;
- `setuptools<82`, `wheel`, `numpy`.

Отдельная системная библиотека `fmt` не устанавливается. Extension собирается с `-DFMT_HEADER_ONLY`, используя headers, поставляемые вместе с PyTorch. Заголовок `kineto/libkineto.h` предоставляет полное определение `CpuTraceBuffer` для текущего PyTorch wheel.

После запуска контейнера сам проект устанавливается командой:

```bash
python -m pip install -e . --no-build-isolation
```

Это собирает Linux extension `torch_stub_gpu._C` и устанавливает пакет в editable-режиме. `--no-build-isolation` позволяет сборке использовать уже установленный в образе PyTorch и его headers.

## Сборка образа

Выполняйте команды из каталога проекта `stub_gpu`:

```bash
cd stub_gpu
docker build -t stub-gpu-profiler .
```

## Запуск контейнера

Linux/macOS:

```bash
docker run -it \
  --name stub_gpu_profiler \
  -v "$PWD":/workspace/stub_gpu \
  -w /workspace/stub_gpu \
  stub-gpu-profiler
```

Проект подключается bind mount-ом, поэтому собранный extension и `stub_gpu_trace.json` будут доступны и на host-машине.

## Сборка extension

Внутри контейнера:

```bash
python -m pip install -e . --no-build-isolation
```

При успешной сборке вывод заканчивается примерно так:

```text
Successfully built torch_stub_gpu
Successfully installed torch_stub_gpu-0.0.0
```

Для повторной сборки после изменения C++-кода выполните ту же команду.

## Smoke test

Внутри контейнера:

```bash
python smoke_test.py
```

Скрипт самостоятельно:

1. запускает CPU + `PrivateUse1` profiler;
2. экспортирует `stub_gpu_trace.json`;
3. парсит файл стандартным JSON parser;
4. проверяет, что `traceEvents` является массивом и содержит все три synthetic-события.

Ожидаемый вывод:

```text
[stub_gpu] profiler start
[stub_gpu] profiler stop
[stub_gpu] processTrace
trace exported: stub_gpu_trace.json
found: ['stubGpuLaunchKernel', 'stubGpuMemcpyHtoD', 'stub_gpu_hello_kernel']
missing: []
trace validation passed
```

Если JSON повреждён, поле `traceEvents` имеет неверный тип или хотя бы одно событие отсутствует, скрипт завершается с ненулевым кодом. Поэтому одной команды `python smoke_test.py` достаточно и для запуска profiler, и для проверки результата.

Предупреждения `CUPTI initialization failed` или `gpuGetDeviceCount failed` допустимы: fake `PrivateUse1` profiler от CUDA не зависит. Созданный `stub_gpu_trace.json` можно открыть в Perfetto UI или Chrome Trace Viewer.

## Полный сценарий

На host-машине:

```bash
cd stub_gpu
docker build -t stub-gpu-profiler .
docker run --rm -it \
  --name stub_gpu_profiler \
  -v "$PWD":/workspace/stub_gpu \
  -w /workspace/stub_gpu \
  stub-gpu-profiler
```

Затем внутри контейнера:

```bash
python -m pip install -e . --no-build-isolation
python smoke_test.py
```
