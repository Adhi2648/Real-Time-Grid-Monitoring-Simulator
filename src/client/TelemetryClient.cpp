#include "TelemetryClient.h"

#include <utility>

TelemetryClient::TelemetryClient(std::string serverAddress)
    : m_serverAddress(std::move(serverAddress)), m_running(false)
{
}

TelemetryClient::~TelemetryClient()
{
    stop();
}

void TelemetryClient::start(TelemetryCallback onTelemetry, StatusCallback onStatus)
{
    if (m_running.exchange(true)) {
        return;
    }

    m_onTelemetry = std::move(onTelemetry);
    m_onStatus = std::move(onStatus);
    m_thread = std::thread(&TelemetryClient::run, this);
}

void TelemetryClient::stop()
{
    if (!m_running.exchange(false)) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_context) {
            m_context->TryCancel();
        }
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool TelemetryClient::isRunning() const
{
    return m_running.load();
}

void TelemetryClient::run()
{
    auto channel = grpc::CreateChannel(m_serverAddress, grpc::InsecureChannelCredentials());
    auto stub = grid::GridMonitoring::NewStub(channel);

    auto context = std::make_unique<grpc::ClientContext>();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_context = std::move(context);
    }

    grid::SubscribeRequest request;
    std::unique_ptr<grpc::ClientReader<grid::PMUData>> reader;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        reader = stub->SubscribeTelemetry(m_context.get(), request);
    }

    if (!reader) {
        if (m_onStatus) {
            m_onStatus("Failed to open telemetry subscription.");
        }
        m_running = false;
        return;
    }

    if (m_onStatus) {
        m_onStatus("Telemetry subscription opened.");
    }

    grid::PMUData message;
    while (m_running.load() && reader->Read(&message)) {
        if (m_onTelemetry) {
            m_onTelemetry(fromProto(message));
        }
    }

    const grpc::Status status = reader->Finish();
    if (m_onStatus) {
        m_onStatus(status.ok() ? "Telemetry subscription closed." : status.error_message());
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_context.reset();
    }

    m_running = false;
}

TelemetryPoint TelemetryClient::fromProto(const grid::PMUData& message)
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