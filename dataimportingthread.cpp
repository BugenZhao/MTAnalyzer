#include "dataimportingthread.h"

#include <utility>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QtCore>

DataImportingThread::DataImportingThread(QString csv, const QDir& dataSetDir, QMap<QString, int> fileId)
        : csv(std::move(csv)),
          dataSetDir(dataSetDir),
          fileId(std::move(fileId)) {
}

void DataImportingThread::run() {
    auto threadDb = QSqlDatabase::addDatabase("QSQLITE",
                                              QString::number(quintptr(QThread::currentThreadId())));
    threadDb.setDatabaseName("file::memory:");
    threadDb.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
    threadDb.open();
    qInfo() << threadDb.tables();

    threadDb.transaction();

    QSqlQuery query(threadDb);


    auto filePath = dataSetDir.absolutePath() + QDir::separator() + csv;
    QFile csvFile(filePath);
    qInfo() << filePath;
    if (!csvFile.open(QFile::ReadOnly | QFile::Text)) {
        return;
        //TODO
    }

    QTextStream stream(&csvFile);
    stream.readLine();

    query.prepare("INSERT INTO bz VALUES (:file,:1,:2,:3,:4,:5,:6)");

    while (!stream.atEnd()) {
        auto line = stream.readLine();
        auto pieces = line.split(',');

        query.bindValue(":file", fileId[csv]);
        for (int i = 0; i < 7; ++i) {
            if (i == 2 || i == 3 || i == 4)
                query.bindValue(i + 1, pieces[i].toInt());
            else if (i == 6)
                query.bindValue(6, pieces[i].toInt());
            else if (i == 5)
                continue;
            else
                query.bindValue(i + 1, pieces[i]);
        }
        if (!query.exec())
            qInfo() << query.lastError().text();
    }

    threadDb.commit();
    threadDb.close();
}
