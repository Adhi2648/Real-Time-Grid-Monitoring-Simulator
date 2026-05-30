#pragma once

#include <cstddef>
#include <memory>

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

class DataProcessor;
class DatabaseManager;
class TelemetryClient;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateUI();

private:
    void initUI();

    DataProcessor* m_processor;
    DatabaseManager* m_dbManager;
    std::unique_ptr<TelemetryClient> m_telemetryClient;

    QLabel* m_lblStatus;
    QLabel* m_lblThroughput;
    QLabel* m_lblLatest;
    
    QTimer* m_timer;
    size_t m_lastProcessedCount;
};
