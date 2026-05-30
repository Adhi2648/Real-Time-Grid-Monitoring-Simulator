#include "GridMonitoringService.h"

#include <chrono>

grpc::Status GridMonitoringService::StreamTelemetry(grpc::ServerContext* context,
                                                    grpc::ServerReader<grid::PMUData>* reader,
                                                    grid::Ack* reply)
{
    grid::PMUData message;
    std::size_t received = 0;

    while (!context->IsCancelled() && reader->Read(&message)) {
        const TelemetryPoint point = fromProto(message);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_latestPoint = point;
            m_hasLatestPoint = true;
            ++m_revision;
        }

        m_cv.notify_all();
        ++received;
    }

    reply->set_success(true);
    reply->set_message("Accepted " + std::to_string(received) + " telemetry points.");
    return grpc::Status::OK;
}

grpc::Status GridMonitoringService::SubscribeTelemetry(grpc::ServerContext* context,
                                                        const grid::SubscribeRequest* request,
                                                        grpc::ServerWriter<grid::PMUData>* writer)
{
    (void)request;

    std::uint64_t lastRevision = 0;

    while (!context->IsCancelled()) {
        TelemetryPoint point;
        bool hasPoint = false;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait_for(lock, std::chrono::milliseconds(250), [&] {
                return context->IsCancelled() || m_revision != lastRevision;
            });

            if (context->IsCancelled()) {
                break;
            }

            if (m_hasLatestPoint && m_revision != lastRevision) {
                point = m_latestPoint;
                hasPoint = true;
                lastRevision = m_revision;
            }
        }

        if (hasPoint && !writer->Write(toProto(point))) {
            break;
        }
    }

    return grpc::Status::OK;
}

ProcessorSnapshot GridMonitoringService::snapshot() const
{
    ProcessorSnapshot snapshot;
    std::lock_guard<std::mutex> lock(m_mutex);
    snapshot.hasLatestPoint = m_hasLatestPoint;
    snapshot.latestPoint = m_latestPoint;
    return snapshot;
}

TelemetryPoint GridMonitoringService::fromProto(const grid::PMUData& message)
{
    TelemetryPoint point;
    point.timestampMs = message.timestamp_ms();
    point.nodeId = message.node_id();
    point.voltageMagnitude = message.voltage_magnitude();
    point.voltageAngle = message.voltage_angle();
    point.currentMagnitude = message.current_magnitude();
    point.currentAngle = message.current_angle();
    point.frequencyHz = message.frequency_hz();
    return point;
}

grid::PMUData GridMonitoringService::toProto(const TelemetryPoint& point)
{
    grid::PMUData message;
    message.set_timestamp_ms(point.timestampMs);
    message.set_node_id(point.nodeId);
    message.set_voltage_magnitude(point.voltageMagnitude);
    message.set_voltage_angle(point.voltageAngle);
    message.set_current_magnitude(point.currentMagnitude);
    message.set_current_angle(point.currentAngle);
    message.set_frequency_hz(point.frequencyHz);
    return message;
}