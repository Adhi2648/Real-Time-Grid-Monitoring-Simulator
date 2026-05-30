#include "DataProcessor.h"
#include <vector>

DataProcessor::DataProcessor() 
    : m_running(true), m_processedCount(0), m_hasLatestPoint(false)
{
    m_processorThread = std::thread(&DataProcessor::workerThread, this);
}

DataProcessor::~DataProcessor() {
    m_running = false;
    m_cv.notify_all();
    
    if (m_processorThread.joinable()) {
        m_processorThread.join();
    }
}

void DataProcessor::pushData(const TelemetryPoint& data) {
    if (!m_running) return;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_dataQueue.push(data);
    }
    m_cv.notify_one();
}

size_t DataProcessor::getProcessedCount() const {
    return m_processedCount.load();
}

ProcessorSnapshot DataProcessor::snapshot() const {
    ProcessorSnapshot snapshot;
    snapshot.processedCount = m_processedCount.load();

    std::lock_guard<std::mutex> lock(m_stateMutex);
    snapshot.hasLatestPoint = m_hasLatestPoint;
    snapshot.latestPoint = m_latestPoint;
    return snapshot;
}

void DataProcessor::workerThread() {
    while (m_running) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { 
            return !m_dataQueue.empty() || !m_running; 
        });

        if (!m_running && m_dataQueue.empty()) {
            break;
        }

        std::vector<TelemetryPoint> batch;
        while (!m_dataQueue.empty() && batch.size() < 1000) {
            batch.push_back(m_dataQueue.front());
            m_dataQueue.pop();
        }
        lock.unlock();

        for (const auto& point : batch) {
            if (point.voltageMagnitude > 0) {
                m_processedCount.fetch_add(1, std::memory_order_relaxed);
                std::lock_guard<std::mutex> stateLock(m_stateMutex);
                m_latestPoint = point;
                m_hasLatestPoint = true;
            }
        }
    }
}
