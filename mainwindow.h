#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QTreeWidgetItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "querywidget.h"
#include "pathsearchwidget.h"
#include "utilities/base.hpp"
//#include "dataimportingthread.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void preloadDataSets();

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
    QString adjacencyFilePath;

    QTreeWidgetItem *importItem;
    QueryWidget *queryWidget;
    PathSearchWidget *pathSearchWidget;

    Adj adj;

    QSqlDatabase db;
    QHash<int, QVector<Record>> data;
    QMap<QString, int> fileId;
    QSet<QString> curCsvs;
    bool curUserIdChecked = false;

    void setupUi();

    void updateFilterWidgetImportAdjacency(QTreeWidgetItem *parent);

    void updateFilterWidgetImportDataSet(QTreeWidgetItem *parent);

    void updateFilterWidgetImportFields(QTreeWidgetItem *parent);

    void updateFilterWidgetFiltersFields(const QStringList &payTypes, const QStringList &lines);

    void importFilteredData();

    void importFilteredDataMt();

    void importAdjacency();

    void importFilteredAll();
};

#endif // MAINWINDOW_H
