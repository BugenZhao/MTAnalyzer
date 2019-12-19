#ifndef BDATABASEMANAGER_H
#define BDATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class BDatabaseManager : public QObject {
Q_OBJECT
public:
    explicit BDatabaseManager(QObject *parent = nullptr);
    static QSqlDatabase connection(const QString& connectionName = QLatin1String(QSqlDatabase::defaultConnection));
    static QSqlDatabase readOnlyConnection(const QString &connectionName = QLatin1String(QSqlDatabase::defaultConnection));
};

#endif // BDATABASEMANAGER_H
