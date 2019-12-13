#include "bdatabasemanager.h"

BDatabaseManager::BDatabaseManager(QObject *parent) : QObject(parent) {
}

QSqlDatabase BDatabaseManager::connection(const QString &connectionName) {
    auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName("file::memory:");
    db.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
    db.open();
    return db;
}
