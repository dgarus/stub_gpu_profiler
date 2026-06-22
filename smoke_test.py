import torch
import torch_stub_gpu

with torch.profiler.profile(
    activities=[
        torch.profiler.ProfilerActivity.CPU,
        torch.profiler.ProfilerActivity.PrivateUse1,
    ],
) as prof:
    pass

prof.export_chrome_trace("stub_gpu_trace.json")
print("trace exported")