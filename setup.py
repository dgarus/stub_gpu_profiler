from setuptools import setup, find_packages
from torch.utils.cpp_extension import CppExtension, BuildExtension
import torch
from pathlib import Path

torch_include = Path(torch.__file__).parent / "include"
kineto_include = torch_include / "kineto"

setup(
    name="torch_stub_gpu",
    packages=find_packages(),
    ext_modules=[
        CppExtension(
            name="torch_stub_gpu._C",
            sources=["csrc/profiler/StubGpuProfiler.cpp"],
            include_dirs=[
                str(kineto_include),
            ],
            extra_compile_args=[
                "-std=c++17",
                "-O0",
                "-g",
                "-DUSE_KINETO",
                "-DFMT_HEADER_ONLY",
            ],
        )
    ],
    cmdclass={"build_ext": BuildExtension},
)