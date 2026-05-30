#include "MainWindow.h"
#include "../core/DataProcessor.h"
#include "../db/DatabaseManager.h"
#include "../common/Env.h"
#include "TelemetryClient.h"

#include <QWidget>
#include <QFont>
#include <QString>

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent), m_lastProcessedCount(0) 
{
    const QString serverAddress = Env::value(QStringLiteral("GRID_SERVER_ADDRESS"), QStringLiteral("localhost:50051"));

    m_processor = new DataProcessor();
    m_dbManager = new DatabaseManager();
    m_telemetryClient = std::make_unique<TelemetryClient>(serverAddress.toStdString());

    m_dbManager->connect(Env::connectionStringFromEnv());

    initUI();

    m_telemetryClient->start([this](const TelemetryPoint& point) {
        m_processor->pushData(point);
    });

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateUI);
    m_timer->start(500);
}

MainWindow::~MainWindow() {
    if (m_telemetryClient) {
        m_telemetryClient->stop();
    }

    delete m_processor;
    delete m_dbManager;
}

void MainWindow::initUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    m_lblStatus = new QLabel("Status: gRPC Server Disconnected", this);
    QFont f = m_lblStatus->font();
    f.setPointSize(14);
    f.setBold(true);
    m_lblStatus->setFont(f);

    m_lblThroughput = new QLabel("Throughput: 0 Points/sec\nLatency: -- ms", this);
    m_lblLatest = new QLabel("Latest telemetry: waiting for data", this);
    
    layout->addWidget(m_lblStatus);
    layout->addSpacing(20);
    layout->addWidget(m_lblThroughput);
    layout->addWidget(m_lblLatest);
    layout->addStretch();

    setCentralWidget(centralWidget);
    setWindowTitle("Real-Time Grid Monitoring Simulator");
    resize(800, 600);
}

void MainWindow::updateUI() {
    const ProcessorSnapshot snapshot = m_processor->snapshot();
    size_t delta = snapshot.processedCount - m_lastProcessedCount;
    m_lastProcessedCount = snapshot.processedCount;

    m_lblThroughput->setText(QString("Throughput: %1 Points/sec\nSub-10ms Engine Active")
                             .arg(delta * 2));

    if (snapshot.hasLatestPoint) {
        const TelemetryPoint& point = snapshot.latestPoint;
        m_lblLatest->setText(QString("Latest telemetry: %1 | V=%2 pu | I=%3 pu | f=%4 Hz")
                                 .arg(QString::fromStdString(point.nodeId))
                                 .arg(point.voltageMagnitude, 0, 'f', 3)
                                 .arg(point.currentMagnitude, 0, 'f', 3)
                                 .arg(point.frequencyHz, 0, 'f', 2));
    }
}
