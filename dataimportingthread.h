#ifndef DATAIMPORTINGTHREAD_H
#define DATAIMPORTINGTHREAD_H


#include <QThread>
#include "mainwindow.h"

class DataImportingThread : public QThread {
private:
    QStringList csvs;
    MainWindow *mainWindow;
public:
    DataImportingThread(QStringList, MainWindow *);

    void run() override;

signals:

    void percentageComplete(int);
};

#endif // DATAIMPORTINGTHREAD_H
