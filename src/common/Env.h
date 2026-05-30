#pragma once

#include <QString>

namespace Env {

bool loadDotEnv(const QString& filePath = QStringLiteral(".env"));
QString value(const QString& key, const QString& defaultValue = QString());
QString connectionStringFromEnv();

}