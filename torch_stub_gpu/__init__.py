import torch

# Регистрируем красивое имя для PrivateUse1 backend.
torch.utils.rename_privateuse1_backend("stub_gpu")
torch.utils.generate_methods_for_privateuse1_backend()

# Важно: импорт .so должен произойти, чтобы сработал
# REGISTER_PRIVATEUSE1_PROFILER(...)
from . import _C  # noqa: F401