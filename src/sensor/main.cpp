#include <atomic>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "../common/Env.h"
#include "../common/Telemetry.h"
#include <grid_telemetry.grpc.pb.h>

namespace {

std::atomic_bool g_running{true};

void handleSignal(int)
{
    g_running = false;
}

TelemetryPoint generateTelemetry(const std::string& nodeId, std::mt19937& rng, std::uint64_t tick)
{
    std::uniform_real_distribution<double> voltageNoise(-0.02, 0.02);
    std::uniform_real_distribution<double> currentNoise(-0.03, 0.03);
    std::uniform_real_distribution<double> angleNoise(-1.0, 1.0);

    const double phase = static_cast<double>(tick % 6000) / 6000.0;
    const double baseVoltage = 1.0 + 0.05 * std::sin(phase * 6.283185307179586);
    const double baseCurrent = 0.8 + 0.08 * std::cos(phase * 6.283185307179586);

    TelemetryPoint point;
    point.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    point.nodeId = nodeId;
    point.voltageMagnitude = baseVoltage + voltageNoise(rng);
    point.voltageAngle = angleNoise(rng);
    point.currentMagnitude = baseCurrent + currentNoise(rng);
    point.currentAngle = angleNoise(rng);
    point.frequencyHz = 60.0 + std::uniform_real_distribution<double>(-0.05, 0.05)(rng);
    return point;
}

grid::PMUData toProto(const TelemetryPoint& point)
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

}

int main()
{
    Env::loadDotEnv();
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    const std::string serverAddress = Env::value(QStringLiteral("GRID_SERVER_ADDRESS"), QStringLiteral("localhost:50051")).toStdString();
    const std::string nodeId = Env::value(QStringLiteral("SENSOR_NODE_ID"), QStringLiteral("PMU-001")).toStdString();
    const int pointsPerSecond = Env::value(QStringLiteral("SENSOR_POINTS_PER_SECOND"), QStringLiteral("2000")).toInt();

    auto channel = grpc::CreateChannel(serverAddress, grpc::InsecureChannelCredentials());
    auto stub = grid::GridMonitoring::NewStub(channel);

    grpc::ClientContext context;
    grid::Ack ack;
    std::unique_ptr<grpc::ClientWriter<grid::PMUData>> writer = stub->StreamTelemetry(&context, &ack);

    if (!writer) {
        std::cerr << "Unable to open telemetry stream to " << serverAddress << std::endl;
        return 1;
    }

    std::mt19937 rng(std::random_device{}());
    const auto delay = std::chrono::microseconds(1'000'000 / std::max(pointsPerSecond, 1));
    std::uint64_t tick = 0;

    std::cout << "Streaming telemetry to " << serverAddress << " as " << nodeId << std::endl;

    while (g_running.load()) {
        const TelemetryPoint point = generateTelemetry(nodeId, rng, tick++);
        if (!writer->Write(toProto(point))) {
            std::cerr << "Telemetry stream closed by server." << std::endl;
            break;
        }

        std::this_thread::sleep_for(delay);
    }

    writer->WritesDone();
    const grpc::Status status = writer->Finish();

    if (!status.ok()) {
        std::cerr << "gRPC stream error: " << status.error_message() << std::endl;
        return 1;
    }

    std::cout << ack.message() << std::endl;
    return 0;
}