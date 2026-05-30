#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include <grpcpp/grpcpp.h>

#include <grid_telemetry.grpc.pb.h>
#include "../common/Telemetry.h"

class GridMonitoringService final : public grid::GridMonitoring::Service {
public:
    grpc::Status StreamTelemetry(grpc::ServerContext* context,
                                 grpc::ServerReader<grid::PMUData>* reader,
                                 grid::Ack* reply) override;

    grpc::Status SubscribeTelemetry(grpc::ServerContext* context,
                                    const grid::SubscribeRequest* request,
                                    grpc::ServerWriter<grid::PMUData>* writer) override;

    ProcessorSnapshot snapshot() const;

private:
    static TelemetryPoint fromProto(const grid::PMUData& message);
    static grid::PMUData toProto(const TelemetryPoint& point);

    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    TelemetryPoint m_latestPoint;
    bool m_hasLatestPoint = false;
    std::uint64_t m_revision = 0;
};