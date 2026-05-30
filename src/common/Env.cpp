#include "Env.h"

#include <QFile>
#include <QTextStream>

namespace {

QString trimQuotes(QString value)
{
    value = value.trimmed();
    if (value.size() >= 2) {
        const QChar first = value.front();
        const QChar last = value.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            value.remove(0, 1);
            value.chop(1);
        }
    }
    return value;
}

}

namespace Env {

bool loadDotEnv(const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const int equalsIndex = line.indexOf('=');
        if (equalsIndex <= 0) {
            continue;
        }

        const QString key = line.left(equalsIndex).trimmed();
        QString value = line.mid(equalsIndex + 1).trimmed();
        value = trimQuotes(value);
        qputenv(key.toUtf8(), value.toUtf8());
    }

    return true;
}

QString value(const QString& key, const QString& defaultValue)
{
    const QByteArray raw = qgetenv(key.toUtf8().constData());
    if (raw.isEmpty()) {
        return defaultValue;
    }
    return QString::fromUtf8(raw);
}

QString connectionStringFromEnv()
{
    const QString server = value(QStringLiteral("SQLSERVER_HOST"), QStringLiteral("localhost"));
    const QString database = value(QStringLiteral("SQLSERVER_DATABASE"), QStringLiteral("GridDB"));
    const QString username = value(QStringLiteral("SQLSERVER_USERNAME"));
    const QString password = value(QStringLiteral("SQLSERVER_PASSWORD"));
    const QString trustServerCertificate = value(QStringLiteral("SQLSERVER_TRUST_SERVER_CERTIFICATE"), QStringLiteral("yes"));

    QString connectionString = QStringLiteral("Driver={ODBC Driver 18 for SQL Server};Server=%1;Database=%2;").arg(server, database);

    if (!username.isEmpty()) {
        connectionString += QStringLiteral("Uid=%1;Pwd=%2;").arg(username, password);
    } else {
        connectionString += QStringLiteral("Trusted_Connection=yes;");
    }

    connectionString += QStringLiteral("TrustServerCertificate=%1;").arg(trustServerCertificate);
    return connectionString;
}

}