#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QTreeWidgetItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "querywidget.h"
//#include "dataimportingthread.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    using Record=QString;

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void importDataSet();

signals:

    void percentageComplete(int);

private slots:

    void on_tabs_currentChanged(int index);

    void onImportStarted();

    void onImportFinished();

private:
    Ui::MainWindow *ui;
    QDir mainDir = QDir();
    QDir dataSetDir = QDir();
    QDir adjacencyDir = QDir();

    QTreeWidgetItem *dataSetItem;
    QueryWidget *queryWidget;

    QVector2D *adj = nullptr;

    QSqlDatabase db;
    QHash<int, QVector<Record>> data;
    QMap<QString, int> fileId;
    QSet<QString> curCsvs;

    const QString timeFormat = "yyyy-MM-dd hh:mm:ss";

    void setupUi();

    void updateFilterWidgetAdjacency();

    void updateFilterWidgetDataSet();

    void updateFilterWidgetOthers(const QStringList &lines, const QStringList &payTypes);

    void test();

    friend class DataImportingThread;
};

#endif // MAINWINDOW_H
