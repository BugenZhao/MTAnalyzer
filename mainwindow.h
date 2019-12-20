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
#include "utilities/BDateTime.h"
#include "flowplotwidget.h"
#include "preferencesdialog.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    void preloadDataSets();

signals:

    void percentageComplete(int);

    void statusBarMessage(QString message, int timeout = 0);

    void enabledChanged(bool enabled);

    void filterDataListUpdated(FilterDataList _filterDataList);

private slots:

    void on_tabs_currentChanged(int index);

    void onImportStarted();

    void onImportFinished();

private:
    Ui::MainWindow *ui;
    QString TITLE;

    bool enabled = false;

    QString adjacencySubdirName = "adjacency_adjacency";
    QString dataSetSubdirName = "dataset";
    int speedLevel = FASTER;

    QDir mainDir = QDir();
    QDir dataSetDir = QDir();
    QDir adjacencyDir = QDir();

    QTreeWidgetItem *importerItem = nullptr;
    QTreeWidgetItem *filterItem = nullptr;

    QueryWidget *queryWidget = nullptr;
    PathSearchWidget *pathSearchWidget = nullptr;
    QVector<BasePlotWidget *> plotWidgets{};

    Adj adj;

    QSqlDatabase db;
    QMap<QString, int> fileId;
    QMap<QString, int> dateId;
    QSet<QString> curCsvs;

    FilterDataList filterDataList;

    bool curUserIdChecked = false;

    void setupBzUi();

    void updateFilterWidgetImportAdjacency(QTreeWidgetItem *parent);

    void updateFilterWidgetImportDataSet(QTreeWidgetItem *parent);

    void updateFilterWidgetImportFields(QTreeWidgetItem *parent);

    void updateFilterWidgetFiltersFields(const QStringList &payTypes, const QStringList &lines);

    [[deprecated]] void importFilteredData();

    void importFilteredDataMt();

    void importAdjacency();

    void doImportAndFilterAll();

    void onPreloadFinished();

    void testPlot();

    BasePlotWidget *initPlotTab(const QString &type, QWidget *plotTab);

    void doAddPlotTab(const QString &type);

    void closePlotTab(int index);

    void preferences();

    void setMainEnabled(bool _enabled);

    FilterDataList getFilterDataList();

    void updateFilterDataList();

    void addFlowPlotTab();

    void addTotalFlowPlotTab();

    void addWithLineFlowPlotTab();

    void addStationFlowPlotTab();
};

#endif // MAINWINDOW_H
