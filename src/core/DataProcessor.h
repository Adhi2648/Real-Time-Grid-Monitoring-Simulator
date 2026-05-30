#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <thread>

#include "../common/Telemetry.h"

class DataProcessor {
public:
    DataProcessor();
    ~DataProcessor();

    void pushData(const TelemetryPoint& data);

    size_t getProcessedCount() const;
    ProcessorSnapshot snapshot() const;

private:
    void workerThread();

    std::queue<TelemetryPoint> m_dataQueue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    
    std::thread m_processorThread;
    std::atomic<bool> m_running;

    std::atomic<size_t> m_processedCount;
    mutable std::mutex m_stateMutex;
    TelemetryPoint m_latestPoint;
    bool m_hasLatestPoint;
};
