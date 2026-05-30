#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager() {
    m_db = QSqlDatabase::addDatabase("QODBC");
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

bool DatabaseManager::connect(const QString& connectionString) {
    m_db.setDatabaseName(connectionString);
    
    if (m_db.open()) {
        std::cout << "Successfully connected to SQL Server Database." << std::endl;
        return true;
    } else {
        std::cerr << "Database Connection Failed: " 
                  << m_db.lastError().text().toStdString() << std::endl;
        return false;
    }
}

void DatabaseManager::disconnect() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::logGridEvent(const QString& nodeId, double frequency, const QString& eventDesc) {
    if (!m_db.isOpen()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO GridEvents (NodeId, Frequency, EventDescription) VALUES (:node, :freq, :desc)");
    query.bindValue(":node", nodeId);
    query.bindValue(":freq", frequency);
    query.bindValue(":desc", eventDesc);

    return query.exec();
}
