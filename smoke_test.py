import json
from pathlib import Path

import torch
import torch_stub_gpu


TRACE_PATH = Path("stub_gpu_trace.json")
EXPECTED_EVENTS = {
    "stubGpuLaunchKernel",
    "stub_gpu_hello_kernel",
    "stubGpuMemcpyHtoD",
}

with torch.profiler.profile(
    activities=[
        torch.profiler.ProfilerActivity.CPU,
        torch.profiler.ProfilerActivity.PrivateUse1,
    ],
) as prof:
    pass

prof.export_chrome_trace(str(TRACE_PATH))
print(f"trace exported: {TRACE_PATH}")

with TRACE_PATH.open() as trace_file:
    trace = json.load(trace_file)

trace_events = trace.get("traceEvents") if isinstance(trace, dict) else None
if not isinstance(trace_events, list):
    raise RuntimeError("invalid trace: 'traceEvents' must be a JSON array")

found = {
    event.get("name")
    for event in trace_events
    if isinstance(event, dict) and event.get("name") in EXPECTED_EVENTS
}
missing = EXPECTED_EVENTS - found

print("found:", sorted(found))
print("missing:", sorted(missing))

if missing:
    raise RuntimeError(
        "missing synthetic profiler events: " + ", ".join(sorted(missing))
    )

print("trace validation passed")
