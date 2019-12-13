#ifndef BDATABASEMANAGER_H
#define BDATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class BDatabaseManager : public QObject {
Q_OBJECT
public:
    explicit BDatabaseManager(QObject *parent = nullptr);
    static QSqlDatabase connection(const QString& connectionName = QLatin1String(QSqlDatabase::defaultConnection));
signals:

public slots:
};

#endif // BDATABASEMANAGER_H
