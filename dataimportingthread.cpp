#include "dataimportingthread.h"

#include <utility>
#include <QDir>
#include <QSqlQuery>
#include <QTextStream>
#include <QtCore>

DataImportingThread::DataImportingThread(QStringList csvs, MainWindow *mainWindow)
        : csvs(std::move(csvs)),
          mainWindow(mainWindow) {
}

void DataImportingThread::run() {

}
