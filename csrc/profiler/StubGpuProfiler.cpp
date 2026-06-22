#include <torch/csrc/profiler/standalone/privateuse1_profiler.h>

#include <kineto/IActivityProfiler.h>
#include <kineto/GenericTraceActivity.h>
#include <kineto/libkineto.h>

#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <Python.h>
#include <iostream>

using namespace std::chrono;

struct FakeEvent
{
    std::string name;
    libkineto::ActivityType type;
    int64_t startUs;
    int64_t endUs;
    int device;
    int stream;
    uint64_t correlationId;
};

class StubGpuProfilerSession final : public libkineto::IActivityProfilerSession
{
public:
    explicit StubGpuProfilerSession(
        const std::set<libkineto::ActivityType> &activities)
        : activities_(activities) {}

    void start() override
    {
        startUs_ = nowUs();

        std::cerr << "[stub_gpu] profiler start\n";
    }

    void stop() override
    {
        auto base = startUs_;

        events_.push_back({
            "stubGpuLaunchKernel",
            libkineto::ActivityType::PRIVATEUSE1_RUNTIME,
            base + 100,
            base + 180,
            0,
            0,
            currentCorrelationId_,
        });

        events_.push_back({
            "stub_gpu_hello_kernel",
            libkineto::ActivityType::CONCURRENT_KERNEL,
            base + 200,
            base + 800,
            0,
            7,
            currentCorrelationId_,
        });

        events_.push_back({
            "stubGpuMemcpyHtoD",
            libkineto::ActivityType::GPU_MEMCPY,
            base + 850,
            base + 1000,
            0,
            7,
            currentCorrelationId_,
        });

        std::cerr << "[stub_gpu] profiler stop\n";
    }

    void processTrace(libkineto::ActivityLogger &logger) override
    {
        std::cerr << "[stub_gpu] processTrace\n";

        libkineto::TraceSpan span(
            startUs_ * 1'000,
            (startUs_ + 10'000) * 1'000,
            "stub_gpu_trace");

        for (const auto &e : events_)
        {
            libkineto::GenericTraceActivity act(span, e.type, e.name);

            act.startTime = e.startUs * 1'000;
            act.endTime = e.endUs * 1'000;
            act.device = e.device;
            act.resource = e.stream;
            act.id = static_cast<int32_t>(e.correlationId);

            act.addMetadata("device", e.device);
            act.addMetadata("stream", e.stream);
            act.addMetadataQuoted("backend", "stub_gpu");

            act.log(logger);
        }
    }

    std::vector<std::string> errors() override
    {
        return {};
    }

    std::unique_ptr<libkineto::DeviceInfo> getDeviceInfo() override
    {
        return std::make_unique<libkineto::DeviceInfo>(
            0,           // device id
            0,           // sort index
            "StubGPU",   // name
            "stub_gpu:0" // label
        );
    }

    std::vector<libkineto::ResourceInfo> getResourceInfos() override
    {
        return {
            libkineto::ResourceInfo{
                7,          // resource id
                7,          // sort index
                0,          // device id
                "stream 7", // name
            },
        };
    }

    std::unique_ptr<libkineto::CpuTraceBuffer> getTraceBuffer() override
    {
        return nullptr;
    }

    void pushCorrelationId(uint64_t id) override
    {
        currentCorrelationId_ = id;
    }

    void popCorrelationId() override
    {
        currentCorrelationId_ = 0;
    }

private:
    int64_t nowUs() const
    {
        return duration_cast<microseconds>(
                   system_clock::now().time_since_epoch())
            .count();
    }

private:
    std::set<libkineto::ActivityType> activities_;
    std::vector<FakeEvent> events_;
    uint64_t currentCorrelationId_{0};
    int64_t startUs_{0};
};

class StubGpuActivityProfiler final : public libkineto::IActivityProfiler
{
public:
    const std::string &name() const override
    {
        return name_;
    }

    const std::set<libkineto::ActivityType> &availableActivities() const override
    {
        return activities_;
    }

    std::unique_ptr<libkineto::IActivityProfilerSession> configure(
        const std::set<libkineto::ActivityType> &activityTypes,
        const libkineto::Config &config) override
    {
        return std::make_unique<StubGpuProfilerSession>(activityTypes);
    }

    std::unique_ptr<libkineto::IActivityProfilerSession> configure(
        int64_t tsMs,
        int64_t durationMs,
        const std::set<libkineto::ActivityType> &activityTypes,
        const libkineto::Config &config) override
    {
        return std::make_unique<StubGpuProfilerSession>(activityTypes);
    }

private:
    std::string name_{"mygpu"};

    std::set<libkineto::ActivityType> activities_{
        libkineto::ActivityType::CONCURRENT_KERNEL,
        libkineto::ActivityType::GPU_MEMCPY,
        libkineto::ActivityType::GPU_MEMSET,
        libkineto::ActivityType::PRIVATEUSE1_RUNTIME,
        libkineto::ActivityType::PRIVATEUSE1_DRIVER,
    };
};

REGISTER_PRIVATEUSE1_PROFILER(StubGpuActivityProfiler);

PyMODINIT_FUNC PyInit__C(void)
{
    static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT,
        "_C",
        nullptr,
        -1,
        nullptr,
    };

    return PyModule_Create(&module);
}