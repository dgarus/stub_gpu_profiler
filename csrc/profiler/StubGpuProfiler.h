#pragma once

#include <kineto/IActivityProfiler.h>

#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <vector>

struct FakeEvent {
  std::string name;
  libkineto::ActivityType type;
  int64_t startUs;
  int64_t endUs;
  int device;
  int stream;
  uint64_t correlationId;
};

class MyGpuProfilerSession final : public libkineto::IActivityProfilerSession {
 public:
  MyGpuProfilerSession(
      const std::set<libkineto::ActivityType>& activities,
      const libkineto::Config& config);

  void start() override;
  void stop() override;
  void processTrace(libkineto::ActivityLogger& logger) override;

  std::vector<std::string> errors() override;

  std::unique_ptr<libkineto::DeviceInfo> getDeviceInfo() override;
  std::vector<libkineto::ResourceInfo> getResourceInfos() override;
  std::unique_ptr<libkineto::CpuTraceBuffer> getTraceBuffer() override;

  void pushCorrelationId(uint64_t id) override;
  void popCorrelationId() override;

 private:
  int64_t nowUs() const;

 private:
  std::set<libkineto::ActivityType> activities_;
  std::vector<FakeEvent> events_;
  std::vector<std::string> errors_;

  uint64_t currentCorrelationId_{0};
  int64_t startUs_{0};
};

class MyGpuActivityProfiler final : public libkineto::IActivityProfiler {
 public:
  const std::string& name() const override;
  const std::set<libkineto::ActivityType>& availableActivities() const override;

  std::unique_ptr<libkineto::IActivityProfilerSession> configure(
      const std::set<libkineto::ActivityType>& activityTypes,
      const libkineto::Config& config) override;

  std::unique_ptr<libkineto::IActivityProfilerSession> configure(
      int64_t tsMs,
      int64_t durationMs,
      const std::set<libkineto::ActivityType>& activityTypes,
      const libkineto::Config& config) override;

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