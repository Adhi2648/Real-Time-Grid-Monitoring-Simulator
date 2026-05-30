#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include <grid_telemetry.grpc.pb.h>
#include "../common/Telemetry.h"

class TelemetryClient {
public:
    using TelemetryCallback = std::function<void(const TelemetryPoint&)>;
    using StatusCallback = std::function<void(const std::string&)>;

    explicit TelemetryClient(std::string serverAddress);
    ~TelemetryClient();

    void start(TelemetryCallback onTelemetry, StatusCallback onStatus = {});
    void stop();
    bool isRunning() const;

private:
    void run();
    static TelemetryPoint fromProto(const grid::PMUData& message);

    std::string m_serverAddress;
    TelemetryCallback m_onTelemetry;
    StatusCallback m_onStatus;
    std::atomic<bool> m_running;
    std::thread m_thread;
    mutable std::mutex m_mutex;
    std::unique_ptr<grpc::ClientContext> m_context;
};