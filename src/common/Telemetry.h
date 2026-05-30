#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

struct TelemetryPoint {
    std::int64_t timestampMs = 0;
    std::string nodeId;
    double voltageMagnitude = 0.0;
    double voltageAngle = 0.0;
    double currentMagnitude = 0.0;
    double currentAngle = 0.0;
    double frequencyHz = 0.0;
};

struct ProcessorSnapshot {
    std::size_t processedCount = 0;
    bool hasLatestPoint = false;
    TelemetryPoint latestPoint;
};