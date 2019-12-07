#ifndef DATAIMPORTINGTHREAD_H
#define DATAIMPORTINGTHREAD_H


#include <QThread>
#include <QDir>
#include <QMap>
#include <QRunnable>


class [[deprecated]] DataImportingThread : public QRunnable {
private:
    QString csv;
    QDir dataSetDir;
    QMap<QString, int> fileId;

public:
    DataImportingThread(QString, const QDir&, QMap<QString, int>);

    void run() override;

signals:

    void percentageComplete(int);
};

#endif // DATAIMPORTINGTHREAD_H
