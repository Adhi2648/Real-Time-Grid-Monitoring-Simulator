#pragma once

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool connect(const QString& connectionString);
    void disconnect();

    bool logGridEvent(const QString& nodeId, double frequency, const QString& eventDesc);

private:
    QSqlDatabase m_db;
};
