#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utilities/hint.hpp"
#include "utilities/bdatabasemanager.h"
#include "totalflowplotwidget.h"
#include "withlineflowplotwidget.h"
#include "stationflowplotwidget.h"
#include "previewwidget.h"
#include <QtWidgets>
#include <string>
#include <QtConcurrent>
#include <QFuture>

using std::string;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setupBzUi();
    TITLE = windowTitle();

    auto fileMenu = menuBar()->addMenu(tr("&File"));
    auto windowMenu = menuBar()->addMenu(tr("&Window"));

    auto importAction = new QAction(tr("&Open Data Set Folder..."), this);
    importAction->setShortcut(QKeySequence::Open);
    connect(importAction, &QAction::triggered, this, &MainWindow::preloadDataSets);
    fileMenu->addAction(importAction);

    auto applyAction = new QAction(tr("&Apply Importer && Filters"), this);
    applyAction->setShortcut(QKeySequence("Ctrl+Return"));
    applyAction->setEnabled(false);
    connect(applyAction, &QAction::triggered, ui->filterButton, &QPushButton::click);
    connect(this, &MainWindow::enabledChanged, applyAction, &QAction::setEnabled);
    fileMenu->addAction(applyAction);

    auto preferencesAction = new QAction(tr("&Preferences"), this);
    preferencesAction->setShortcut(QKeySequence::Preferences);
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::preferences);
    fileMenu->addAction(preferencesAction);

    fileMenu->addSeparator();
    auto addTabMenu = fileMenu->addMenu("&Add Tab...");

    auto addFlowPlotTabAction = new QAction(tr("Inflow and Outflow Plot"), this);
    addFlowPlotTabAction->setShortcut(QKeySequence("Ctrl+1"));
    connect(addFlowPlotTabAction, &QAction::triggered, this, &MainWindow::addFlowPlotTab);
    connect(this, &MainWindow::enabledChanged, addFlowPlotTabAction, &QAction::setEnabled);
    addTabMenu->addAction(addFlowPlotTabAction);

    auto addTotalFlowPlotTabAction = new QAction(tr("Total Flow Plot"), this);
    addTotalFlowPlotTabAction->setShortcut(QKeySequence("Ctrl+2"));
    connect(addTotalFlowPlotTabAction, &QAction::triggered, this, &MainWindow::addTotalFlowPlotTab);
    connect(this, &MainWindow::enabledChanged, addTotalFlowPlotTabAction, &QAction::setEnabled);
    addTabMenu->addAction(addTotalFlowPlotTabAction);

    auto addWithLineFlowPlotTabAction = new QAction(tr("Inflow Plot with Lines"), this);
    addWithLineFlowPlotTabAction->setShortcut(QKeySequence("Ctrl+3"));
    connect(addWithLineFlowPlotTabAction, &QAction::triggered, this, &MainWindow::addWithLineFlowPlotTab);
    connect(this, &MainWindow::enabledChanged, addWithLineFlowPlotTabAction, &QAction::setEnabled);
    addTabMenu->addAction(addWithLineFlowPlotTabAction);

    auto addStationFlowPlotTabAction = new QAction(tr("Station Flow Plot"), this);
    addStationFlowPlotTabAction->setShortcut(QKeySequence("Ctrl+4"));
    connect(addStationFlowPlotTabAction, &QAction::triggered, this, &MainWindow::addStationFlowPlotTab);
    connect(this, &MainWindow::enabledChanged, addStationFlowPlotTabAction, &QAction::setEnabled);
    addTabMenu->addAction(addStationFlowPlotTabAction);

    auto showDockAction = new QAction(tr("&Importer && Filters"), this);
    showDockAction->setShortcut(QKeySequence("Ctrl+I"));
    showDockAction->setCheckable(true);
    connect(ui->dockWidget, &QDockWidget::visibilityChanged, showDockAction, &QAction::setChecked);
    connect(showDockAction, &QAction::toggled, ui->dockWidget, &QDockWidget::setVisible);
    windowMenu->addAction(showDockAction);


    connect(ui->filterButton, &QPushButton::clicked, this, &MainWindow::doImportAndFilterAll);

    db = BDatabaseManager::connection();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupBzUi() {
    ui->filterTree->setHeaderLabels({tr("Filters")});
    ui->filterTree->setAnimated(true);

    auto tab1Layout = new QVBoxLayout(ui->tab1);
    tab1Layout->addWidget(ui->hintLabel);
    tab1Layout->setContentsMargins(0, 0, 0, 0);
    ui->tab1->setLayout(tab1Layout);


    plotWidgets.clear();
//    for (int i = 0; i < ui->plotTabs->count(); ++i) {
//        auto plotTab = ui->plotTabs->widget(i);
//        initPlotTab(plotTab);
//    }
    ui->plotTabs->clear();
    connect(ui->plotTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closePlotTab);

    auto tab2Layout = new QVBoxLayout(ui->tab2);
    pathSearchWidget = new PathSearchWidget(&adj, this);
    tab2Layout->addWidget(pathSearchWidget);
    ui->tab2->setLayout(tab2Layout);

    auto tab3Layout = new QVBoxLayout(ui->tab3);
    queryWidget = new QueryWidget(this);
    tab3Layout->addWidget(queryWidget);
    ui->tab3->setLayout(tab3Layout);

    auto tabPreviewLayout = new QVBoxLayout(ui->tabPreview);
    previewWidget = new PreviewWidget(this);
    tabPreviewLayout->addWidget(previewWidget);
    ui->tabPreview->setLayout(tabPreviewLayout);

    ui->progressBar->setMaximum(100);
    connect(this, &MainWindow::percentageComplete, ui->progressBar, &QProgressBar::setValue);

    ui->hintLabel->setText(BugenZhao::hintRichText);

    connect(queryWidget, &QueryWidget::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);
    connect(pathSearchWidget, &PathSearchWidget::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);
    connect(this, &MainWindow::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);

    connect(this->statusBar(), &QStatusBar::messageChanged, this, &MainWindow::statusBarReady);

    if (BZ_DEBUG) {
        ui->filterTree->setHeaderHidden(false);
        ui->filterTree->setColumnCount(2);
    }
}

BasePlotWidget *MainWindow::initPlotTab(const QString &type, QWidget *plotTab) {
    BasePlotWidget *plotWidget = nullptr;
    if (type == "FLOW") {
        plotWidget = new FlowPlotWidget(plotTab);
    } else if (type == "TOTAL") {
        plotWidget = new TotalFlowPlotWidget(plotTab);
    } else if (type == "LINE_FLOW") {
        plotWidget = new WithLineFlowPlotWidget(plotTab);
    } else if (type == "STATION") {
        plotWidget = new StationFlowPlotWidget(plotTab);
    }
    plotWidgets.push_back(plotWidget);
    auto tabLayout = new QVBoxLayout(plotTab);
    tabLayout->addWidget(plotWidget);
    tabLayout->setContentsMargins(12, 0, 12, 0);
    plotTab->setLayout(tabLayout);

    connect(plotWidget, &BasePlotWidget::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);
    return plotWidget;
}

void MainWindow::doAddPlotTab(const QString &type) {
    if (ui->plotTabs->count() == 0) {
        ui->tab1->layout()->removeWidget(ui->hintLabel);
        ui->tab1->layout()->addWidget(ui->plotTabs);
        ui->hintLabel->hide();
        if (speedLevel != FASTEST)
                emit statusBarMessage("To get real(best) performance of the SQL-based plot analysis, "
                                      "please go to Preferences and set the speed to 'Fastest', "
                                      "which will turn off all animations",
                                      15000);
    }

    auto plotTab = new QWidget(ui->plotTabs);
    auto tabName = QString("Plot %1 - %2").arg(plotWidgets.size() + 1);
    if (type == "FLOW") {
        tabName = tabName.arg("In/Outflow");
    } else if (type == "TOTAL") {
        tabName = tabName.arg("Total Flow");
    } else if (type == "LINE_FLOW") {
        tabName = tabName.arg("Flow with Lines");
    } else if (type == "STATION") {
        tabName = tabName.arg("Station Flow");
    }
    auto index = ui->plotTabs->addTab(plotTab, tabName);

    auto plotWidget = initPlotTab(type, plotTab);
    plotWidget->setBzEnabled(enabled);
    plotWidget->setSpeed(speedLevel);

    plotWidget->setFilterDataList(filterDataList);
    connect(this, &MainWindow::filterDataListUpdated, plotWidget, &BasePlotWidget::setFilterDataList);

    try {
        if (enabled)
            plotWidget->setDate(importerItem->child(1)->child(0)->text(0));
    } catch (...) {}

    ui->plotTabs->setCurrentIndex(index);
}

void MainWindow::addFlowPlotTab() {
    doAddPlotTab("FLOW");
}

void MainWindow::addTotalFlowPlotTab() {
    doAddPlotTab("TOTAL");
}

void MainWindow::addWithLineFlowPlotTab() {
    doAddPlotTab("LINE_FLOW");
}

void MainWindow::addStationFlowPlotTab() {
    doAddPlotTab("STATION");
}

void MainWindow::closePlotTab(int index) {
    ui->plotTabs->removeTab(index);

    if (ui->plotTabs->count() == 0) {
        ui->tab1->layout()->removeWidget(ui->plotTabs);
        ui->tab1->layout()->addWidget(ui->hintLabel);
        ui->hintLabel->show();
    }
}

void MainWindow::preloadDataSets() {
    auto dir = QFileDialog::getExistingDirectory(this, tr("Select data set directory"));
//    auto dir = QString("/Users/bugenzhao/Codes/CLionProjects/FinalProject/dataset_CS241/");
    if (dir.isEmpty()) { return; }


    ui->filterTree->clear();
    curCsvs.clear();
    setMainEnabled(false);


    QSqlQuery query;
    qInfo() << query.exec("DROP TABLE bz");
    qInfo() << query.exec("CREATE TABLE \"bz\" (\n"
                          "  \"time\" text(20),\n"
                          "  \"lineID\" text(3),\n"
                          "  \"stationID\" integer,\n"
                          "  \"deviceID\" integer,\n"
                          "  \"status\" integer,\n"
                          "  \"userID\" text(40),\n"
                          "  \"payType\" integer,\n"
                          "  \"file\" integer,\n"
                          "  \"dateID\" integer,\n"
                          "  \"timestamp\" integer\n"
                          ");");
//    qInfo() << query.exec("\n"
//                          "CREATE INDEX \"userID\"\n"
//                          "ON \"bz\" (\n"
//                          "  \"userID\"\n"
//                          ");");
//    qInfo() << query.exec("\n"
//                          "CREATE INDEX \"file\"\n"
//                          "ON \"bz\" (\n"
//                          "  \"file\"\n"
//                          ");");
//    qInfo() << query.exec("\n"
//                          "CREATE INDEX \"timestamp\"\n"
//                          "ON \"bz\" (\n"
//                          "  \"timestamp\"\n"
//                          ");");

    mainDir = QDir(dir);
    adjacencyDir = mainDir;
    bool ok1 = adjacencyDir.cd(adjacencySubdirName);
    dataSetDir = mainDir;
    bool ok2 = dataSetDir.cd(dataSetSubdirName);

    if (!ok1 || !ok2) {
        emit statusBarMessage(QString("Failed to open data set. "
                                      "Make sure that subdirectories '%1' and '%2' exist. "
                                      "You can custom them in Preferences.")
                                      .arg(adjacencySubdirName).arg(dataSetSubdirName));
        return;
    }


    importerItem = new QTreeWidgetItem(ui->filterTree);
    importerItem->setText(0, tr("Importer"));
    importerItem->setText(1, tr("IMPORTER"));
    importerItem->setCheckState(0, Qt::Checked);
    importerItem->setFlags(importerItem->flags() & (~Qt::ItemIsUserCheckable) | Qt::ItemIsAutoTristate);
    importerItem->setExpanded(true);

    updateFilterWidgetImportAdjacency(importerItem);
    updateFilterWidgetImportDataSet(importerItem);
    updateFilterWidgetImportFields(importerItem);


    updateFilterWidgetFiltersFields({}, {});

    importAdjacency();

    for (const auto &plotWidget:plotWidgets) {
        plotWidget->setDate(importerItem->child(1)->child(0)->text(0));
    }

    onPreloadFinished();
}

void MainWindow::updateFilterWidgetImportAdjacency(QTreeWidgetItem *parent) {
    auto adjacencyCsv = adjacencyDir.entryList(QStringList() << "*.csv", QDir::Files)[0];

    auto adjacencyItem = new QTreeWidgetItem(parent);
    adjacencyItem->setText(0, tr("Adjacency table"));
    adjacencyItem->setText(1, tr("ADJ_TABLE"));
    adjacencyItem->setCheckState(0, Qt::Checked);
    adjacencyItem->setFlags(adjacencyItem->flags());
    adjacencyItem->setDisabled(true);
    adjacencyItem->setExpanded(true);

    auto csvItem = new QTreeWidgetItem(adjacencyItem);
    csvItem->setText(0, adjacencyCsv);
    csvItem->setText(1, "ADJ_DATA");
    csvItem->setCheckState(0, Qt::Checked);
    csvItem->setFlags(csvItem->flags() | Qt::ItemIsAutoTristate);
}

void MainWindow::updateFilterWidgetImportDataSet(QTreeWidgetItem *parent) {
    auto dataSets = dataSetDir.entryList(QStringList() << "*.csv", QDir::Files);
    auto datesMap = QMap<QString, QVector<QString>>();

    auto pattern = QRegExp(R"(\d{4}-\d{2}-\d{2})");

    fileId.clear();
    dateId.clear();
    for (const auto &csv:dataSets) {
//        qInfo() << csv;
        fileId.insert(csv, fileId.size());
        auto date = csv.split("_")[1];
        if (pattern.exactMatch(date)) {
            if (!datesMap.contains(date))
                datesMap.insert(date, QVector<QString>());
            datesMap[date].push_back(csv);

            if (!dateId.contains(date))
                dateId.insert(date, dateId.size());
        }
    }

    auto dataSetItem = new QTreeWidgetItem(parent);
    dataSetItem->setText(0, tr("Data sets"));
    dataSetItem->setText(1, "DATA_SETS");
    dataSetItem->setCheckState(0, Qt::Checked);
    dataSetItem->setFlags(dataSetItem->flags() | Qt::ItemIsAutoTristate);
    dataSetItem->setExpanded(true);

    for (const auto &date:datesMap.keys()) {
        std::sort(datesMap[date].begin(), datesMap[date].end());
        auto dateItem = new QTreeWidgetItem(dataSetItem);
        dateItem->setText(0, date);
        dateItem->setText(1, "DATE");
        dateItem->setCheckState(0, Qt::Unchecked);
        dateItem->setFlags(dateItem->flags() | Qt::ItemIsAutoTristate);

        for (const auto &csv:datesMap[date]) {
            auto csvItem = new QTreeWidgetItem(dateItem);
            csvItem->setText(0, csv);
            csvItem->setText(1, "DATA");
            csvItem->setCheckState(0, Qt::Unchecked);
            csvItem->setFlags(csvItem->flags() | Qt::ItemIsAutoTristate);
        }
    }

    if (dataSetItem->childCount() != 0)
        dataSetItem->child(0)->setCheckState(0, Qt::Checked);
}

void MainWindow::updateFilterWidgetImportFields(QTreeWidgetItem *parent) {
    auto fieldsItem = new QTreeWidgetItem(parent);
    fieldsItem->setText(0, tr("Fields"));
    fieldsItem->setText(1, tr("FIELDS"));
    fieldsItem->setCheckState(0, Qt::Unchecked);
    fieldsItem->setExpanded(true);
    fieldsItem->setFlags(fieldsItem->flags() & (~Qt::ItemIsUserCheckable) | Qt::ItemIsAutoTristate);


    auto timeItem = new QTreeWidgetItem(fieldsItem);
    timeItem->setText(0, tr("Time"));
    timeItem->setText(1, tr("TIME"));
    timeItem->setText(2, "time");
    timeItem->setCheckState(0, Qt::Checked);
    timeItem->setFlags(timeItem->flags() | Qt::ItemIsAutoTristate);
    timeItem->setDisabled(true);

    auto lineIdItem = new QTreeWidgetItem(fieldsItem);
    lineIdItem->setText(0, tr("Line ID"));
    lineIdItem->setText(1, tr("LINE_ID"));
    lineIdItem->setText(2, "lineID");
    lineIdItem->setCheckState(0, Qt::Checked);
    lineIdItem->setFlags(lineIdItem->flags() | Qt::ItemIsAutoTristate);

    auto stationIdItem = new QTreeWidgetItem(fieldsItem);
    stationIdItem->setText(0, tr("Station ID"));
    stationIdItem->setText(1, tr("STATION_ID"));
    stationIdItem->setText(2, "stationID");
    stationIdItem->setCheckState(0, Qt::Checked);
    stationIdItem->setFlags(stationIdItem->flags() | Qt::ItemIsAutoTristate);
    stationIdItem->setDisabled(true);

    auto deviceIdItem = new QTreeWidgetItem(fieldsItem);
    deviceIdItem->setText(0, tr("Device ID"));
    deviceIdItem->setText(1, tr("DEVICE_ID"));
    deviceIdItem->setText(2, "deviceID");
    deviceIdItem->setCheckState(0, Qt::Unchecked);
    deviceIdItem->setFlags(deviceIdItem->flags() | Qt::ItemIsAutoTristate);

    auto statusItem = new QTreeWidgetItem(fieldsItem);
    statusItem->setText(0, tr("Status"));
    statusItem->setText(1, tr("STATUS"));
    statusItem->setText(2, "status");
    statusItem->setCheckState(0, Qt::Checked);
    statusItem->setFlags(statusItem->flags() | Qt::ItemIsAutoTristate);
    statusItem->setDisabled(true);

    auto userIdItem = new QTreeWidgetItem(fieldsItem);
    userIdItem->setText(0, tr("User ID"));
    userIdItem->setText(1, tr("USER_ID"));
    userIdItem->setText(2, "userID");
    userIdItem->setCheckState(0, Qt::Checked);
    userIdItem->setFlags(userIdItem->flags() | Qt::ItemIsAutoTristate);

    auto payTypeItem = new QTreeWidgetItem(fieldsItem);
    payTypeItem->setText(0, tr("Payment Types"));
    payTypeItem->setText(1, tr("PAY_TYPE"));
    payTypeItem->setText(2, "payType");
    payTypeItem->setCheckState(0, Qt::Unchecked);
    payTypeItem->setFlags(payTypeItem->flags() | Qt::ItemIsAutoTristate);

    curFields.clear();
    for (int i = 0; i < fieldsItem->childCount(); ++i) {
        auto field = fieldsItem->child(i);
        if (!field->isDisabled() && field->checkState(0) == Qt::Checked)
            curFields.insert(field->text(1));
    }
}

void MainWindow::updateFilterWidgetFiltersFields(const QStringList &payTypes, const QStringList &lines) {
    filterItem = new QTreeWidgetItem(ui->filterTree);
    filterItem->setText(0, tr("Filters for Plots"));
    filterItem->setText(1, "FILTERS");
    filterItem->setCheckState(0, Qt::Checked);
    filterItem->setFlags(filterItem->flags() | Qt::ItemIsAutoTristate);
    filterItem->setExpanded(true);

    auto linesItem = new QTreeWidgetItem(filterItem);
    linesItem->setText(0, tr("Lines"));
    linesItem->setText(1, "LINES");
    linesItem->setCheckState(0, Qt::Checked);
    linesItem->setFlags(linesItem->flags() | Qt::ItemIsAutoTristate);

    for (const auto &s:QStringList{"A", "B", "C"}) {
        auto item = new QTreeWidgetItem(linesItem);
        item->setText(0, s);
        item->setText(1, "LINE");
        item->setCheckState(0, Qt::Checked);
        item->setFlags(item->flags() | Qt::ItemIsAutoTristate);
    }

    auto payTypesItem = new QTreeWidgetItem(filterItem);
    payTypesItem->setText(0, tr("Payment Types"));
    payTypesItem->setText(1, "PAY_TYPES");
    payTypesItem->setCheckState(0, Qt::Checked);
    payTypesItem->setFlags(payTypesItem->flags() | Qt::ItemIsAutoTristate);

    for (const auto &s:QStringList{"0", "1", "2", "3"}) {
        auto item = new QTreeWidgetItem(payTypesItem);
        item->setText(0, s);
        item->setText(1, "PAY_TYPE");
        item->setCheckState(0, Qt::Checked);
        item->setFlags(item->flags() | Qt::ItemIsAutoTristate);
    }
}

void MainWindow::doImportAndFilterAll() {
    importFilteredDataMt();
//  updateFilterDataList();
//  Now in onImportFinished()
}

void MainWindow::importFilteredDataMt() {
    auto thread = QThread::create([this] {
        auto startTime = QTime::currentTime();

        auto dataSetItem = importerItem->child(1);
        QMap<QString, QTreeWidgetItem *> filterItems;
        QSet<QString> newFields;
        QStringList sqlFields;
        for (int i = 0; i < importerItem->child(2)->childCount(); ++i) {
            auto field = importerItem->child(2)->child(i);
            if (field->checkState(0) == Qt::Checked) {
                newFields.insert(field->text(1));
                sqlFields.append(field->text(2));
            }
            if (!field->isDisabled()) {
                filterItems.insert(field->text(1), field);
            }
        }
        qInfo() << newFields;

        previewSqlText = QString("SELECT %1 FROM bz LIMIT 1000").arg(sqlFields.join(", "));
        qInfo() << previewSqlText;

        QSet<QString> newCsvs;
        for (int i = 0; i < dataSetItem->childCount(); ++i) {
            auto date = dataSetItem->child(i);
            for (int j = 0; j < date->childCount(); ++j) {
                auto child = date->child(j);
                if (child->checkState(0) == Qt::Checked && child->text(1) == "DATA")
                    newCsvs.insert(child->text(0));
            }
        }

        bool needToReload = false;
        qInfo() << (curFields == newFields);
        if (curFields != newFields) needToReload = true;

        auto threadDb = BDatabaseManager::connection("thread");
        QSqlQuery query(threadDb);

//        qInfo() << threadDb.tables();

        threadDb.transaction();

        if (newCsvs.isEmpty()) {
            emit statusBarMessage("Removing all data...");
            if (!query.exec("DELETE FROM bz"))
                qInfo() << query.lastError().text();
            if (!query.exec("DROP INDEX main_index"))
                qInfo() << query.lastError().text();
        } else {
            QList<QString> toInsert;
            QList<QString> toDelete;


            if (needToReload) {
                emit statusBarMessage("Removing all data...");
                if (!query.exec("DELETE FROM bz"))
                    qInfo() << query.lastError().text();
                if (!query.exec("DROP INDEX main_index"))
                    qInfo() << query.lastError().text();
                toInsert = newCsvs.toList();
            } else {
                for (const auto &csv:curCsvs) {
                    if (!newCsvs.contains(csv)) toDelete.append(csv);
                }
                for (const auto &csv:newCsvs) {
                    if (!curCsvs.contains(csv)) toInsert.append(csv);
                }
            }

            qInfo() << toInsert;
            qInfo() << toDelete;

            emit statusBarMessage("Loading data sets concurrently...");
            const QString QUERY("INSERT INTO bz VALUES (%1)");

            auto worker = [this, QUERY, newFields](const QString &csv) -> QVector<QString> {
                auto filePath = dataSetDir.absolutePath() + QDir::separator() + csv;
                QFile csvFile(filePath);
                qInfo() << filePath;

                if (!csvFile.open(QFile::ReadOnly | QFile::Text)) {
                    return {};
                    //TODO
                }

                QTextStream stream(&csvFile);
                stream.readLine();

                const QString STRFTIME("STRFTIME('%s', '%1')");
                QVector<QString> queries;
                queries.reserve(80000);

                bool lineIdChecked = newFields.contains("LINE_ID");
                bool deviceIdChecked = newFields.contains("DEVICE_ID");
                bool userIdChecked = newFields.contains("USER_ID");
                bool payTypeChecked = newFields.contains("PAY_TYPE");

                QString lineId = "NULL";
                QString deviceId = "NULL";
                QString userId = "NULL";
                QString payType = "NULL";

                while (!stream.atEnd()) {
                    auto line = stream.readLine();
                    auto pieces = line.split(',');

                    auto _dateId = dateId[pieces[0].split(' ')[0]];

                    if (lineIdChecked) lineId = QString("'%1'").arg(pieces[1]);
                    if (deviceIdChecked) deviceId = pieces[3];
                    if (userIdChecked) userId = QString("\"%1\"").arg(pieces[5]);
                    if (payTypeChecked) payType = pieces[6];

                    QStringList queryPieces;
                    queryPieces << QString("'%1'").arg(pieces[0])
                                << lineId
                                << pieces[2]
                                << deviceId
                                << pieces[4]
                                << userId
                                << payType
                                << QString::number(fileId[csv])
                                << QString::number(_dateId)
                                << QString::number(BDateTime::bToLocalTimestamp(pieces[0]));


                    queries.append(QUERY.arg(queryPieces.join(", ")));
                }
                return queries;
            };
            auto insertQueries = QtConcurrent::blockingMapped(toInsert,
                                                              std::function<QVector<QString>(const QString &)>(worker));

            // Delete
            int total = toDelete.size();
            int cur = 0;

            emit statusBarMessage("Removing...");
            for (const auto &csv:toDelete) {
                query.prepare("DELETE FROM bz WHERE file = :file");
                query.bindValue(":file", fileId[csv]);
                if (!query.exec())
                    qInfo() << query.lastError().text();
                ++cur;
                emit percentageComplete(int((cur + 0.0) / total * 100));
            }

            // Insert
            emit statusBarMessage("Please wait while data sets are processing...");
            total = insertQueries.size();
            cur = 0;
            QVector<int> perTimesMs;

            for (auto &result:insertQueries) {
                auto t0 = QTime::currentTime();

                for (const auto &queryText:result) {
                    query.exec(queryText);
                }
                result.clear();

                perTimesMs.push_back(t0.msecsTo(QTime::currentTime()));
                emit statusBarMessage(
                        QString("Please wait while data sets are processing. Remaining time: %1s")
                                .arg((total - cur) * BugenZhao::bAverage(perTimesMs) / 1000));
                emit percentageComplete((++cur) * 100 / total);
            }

            // Attempt to create index, if exists then ignored
            emit statusBarMessage("Creating indices...");
            emit percentageComplete(0);
            qInfo() << query.exec("\n"
                                  "CREATE INDEX \"main_index\"\n"
                                  "ON \"bz\" (\n"
                                  "  \"timestamp\",\n"
                                  "  \"status\",\n"
                                  "  \"stationID\",\n"
                                  "  \"lineID\"\n"
                                  ");");
            emit percentageComplete(80);
//            qInfo() << query.exec("\n"
//                                  "CREATE INDEX \"file\"\n"
//                                  "ON \"bz\" (\n"
//                                  "  \"file\"\n"
//                                  ");");
//            emit percentageComplete(90);
//            qInfo() << query.exec("\n"
//                                  "CREATE INDEX \"stationID\"\n"
//                                  "ON \"bz\" (\n"
//                                  "  \"stationID\"\n"
//                                  ");");
            emit percentageComplete(100);
        }

        threadDb.commit();
        threadDb.close();

        curCsvs = newCsvs;
        curFields = newFields;

        emit statusBarMessage(
            QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0), 10000);
    });

    connect(thread, &QThread::started, this, &MainWindow::onImportStarted);
    connect(thread, &QThread::finished, this, &MainWindow::onImportFinished);

    thread->start();
}

void MainWindow::importAdjacency() {
    auto thread = QThread::create([this]() {
        auto filePath = adjacencyDir.entryInfoList(QStringList() << "*.csv", QDir::Files)[0]
                .absoluteFilePath();
        qInfo() << filePath;
        QFile adjFile(filePath);
        if (!adjFile.open(QFile::ReadOnly | QFile::Text)) {
            return;
            //TODO
        }
        adj = QVector<QVector<bool>>();
        QTextStream stream(&adjFile);
        stream.readLine();
        while (!stream.atEnd()) {
            QVector<bool> row;
            auto line = stream.readLine();
            auto pieces = line.split(',');
            for (int i = 1; i < pieces.size(); ++i) {
                row.push_back(pieces[i] == "1");
            }
            adj.push_back(row);
        }
        qInfo() << adj.size();
    });

    thread->start();
}

void MainWindow::on_tabs_currentChanged(int index) {
//    if (index == 2) resize((width() > 1120) ? width() : 1120, height());
//    else resize((width() > 1120) ? width() : 960, height());
}

void MainWindow::onPreloadFinished() {
    ui->filterButton->setEnabled(true);
    ui->progressBar->setValue(0);
    queryWidget->setBzEnabled(false);
    pathSearchWidget->setBzEnabled(true);
    for (auto plotWidget:plotWidgets) {
        plotWidget->setBzEnabled(false);
    }
    emit statusBarMessage("Data set folder is open, click 'Apply' to load and process them.", 8000);
    setWindowTitle(QString("%1 - %2").arg(TITLE).arg(mainDir.absolutePath()));
    setMainEnabled(true);
    ui->filterButton->setFocus();
}

void MainWindow::onImportStarted() {
    setMainEnabled(false);
    ui->filterButton->setEnabled(false);
    ui->progressBar->setValue(0);
    queryWidget->setBzEnabled(false);
    pathSearchWidget->setBzEnabled(false);
    for (auto plotWidget:plotWidgets) {
        plotWidget->setBzEnabled(false);
    }
}

void MainWindow::onImportFinished() {
    if (curFields.contains("LINE_ID")) {
        filterItem->child(0)->setDisabled(false);
    } else {
        filterItem->child(0)->setDisabled(true);
        filterItem->child(0)->setCheckState(0, Qt::Checked);
    }
    if (curFields.contains("PAY_TYPE")) {
        filterItem->child(1)->setDisabled(false);
    } else {
        filterItem->child(1)->setDisabled(true);
        filterItem->child(1)->setCheckState(0, Qt::Checked);
    }

    updateFilterDataList();

    ui->filterButton->setEnabled(true);
    ui->progressBar->setValue(100);
    queryWidget->setBzEnabled(true);
    pathSearchWidget->setBzEnabled(true);
    previewWidget->updateBz(previewSqlText);
    for (auto plotWidget:plotWidgets) {
        plotWidget->setBzEnabled(true);
    }
    setMainEnabled(true);
    ui->tabs->setCurrentIndex(1);
}

void MainWindow::testPlot() {
    FlowPlotWidget::BzChartData chartData;
    chartData.title = "Test";
    chartData.seriesNames = QStringList() << "Line 1";
    BDataList dataList;
    for (int i = 0; i < 10; ++i) {
        dataList.append(BData{QPointF{static_cast<qreal>(i), static_cast<qreal>(i * i)}, "Test"});
    }
    chartData.dataTable = QList<BDataList>() << dataList;
//    plotWidgets[0]->setDateTimeSplineChart(chartData);
}

void MainWindow::preferences() {
    auto newPreferences = PreferencesDialog::getPreferences(
            {adjacencySubdirName,
             dataSetSubdirName,
             speedLevel}, !enabled);
    if (!newPreferences.adjacencySubdirName.isEmpty()) adjacencySubdirName = newPreferences.adjacencySubdirName;
    if (!newPreferences.dataSetSubdirName.isEmpty()) dataSetSubdirName = newPreferences.dataSetSubdirName;
    speedLevel = newPreferences.speedLevel;

    for (auto plotWidget:plotWidgets) {
        plotWidget->setSpeed(speedLevel);
    }
}

void MainWindow::setMainEnabled(bool _enabled) {
    this->enabled = _enabled;
    emit enabledChanged(_enabled);
}

void MainWindow::updateFilterDataList() {
    FilterDataList _filterDataList;
    for (int i = 0; i < filterItem->childCount(); ++i) {
        auto filter = filterItem->child(i);
        FilterData filterData{filter->text(1), QMap<QString, bool>()};
        for (int j = 0; j < filter->childCount(); ++j) {
            auto child = filter->child(j);
            filterData.second.insert(child->text(0), child->checkState(0) == Qt::Checked);
        }
        _filterDataList.push_back(filterData);
    }
    filterDataList = _filterDataList;

    //
    emit filterDataListUpdated(filterDataList);
}

FilterDataList MainWindow::getFilterDataList() {
    return filterDataList;
}

void MainWindow::statusBarReady(const QString &message) {
    if (message.isNull()) emit statusBarMessage("Ready");
}


[[deprecated]] void MainWindow::importFilteredData() {
    QSet<QString> newCsvs;

    auto dataSetItem = importerItem->child(1);

    for (int i = 0; i < dataSetItem->childCount(); ++i) {
        auto date = dataSetItem->child(i);
        for (int j = 0; j < date->childCount(); ++j) {
            auto child = date->child(j);
            if (child->checkState(0) == Qt::Checked && child->text(1) == "DATA")
                newCsvs.insert(child->text(0));
        }
    }

    QList<QString> toInsert;
    QList<QString> toDelete;

    for (const auto &csv:curCsvs) {
        if (!newCsvs.contains(csv)) toDelete.append(csv);
    }
    for (const auto &csv:newCsvs) {
        if (!curCsvs.contains(csv)) toInsert.append(csv);
    }

    qInfo() << toInsert;
    qInfo() << toDelete;

    auto thread = QThread::create([this, toInsert, toDelete] {
        emit percentageComplete(0);

        auto threadDb = BDatabaseManager::connection("thread");
        qInfo() << threadDb.tables();

        threadDb.transaction();

        QSqlQuery query(threadDb);

        int total = toInsert.size();
        int cur = 0;
        for (const auto &csv:toInsert) {
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
            ++cur;
            emit percentageComplete(int((cur + 0.0) / total * 100));
        }

        total = toDelete.size();
        cur = 0;

        for (const auto &csv:toDelete) {
            query.prepare("DELETE FROM bz WHERE file = :file");
            query.bindValue(":file", fileId[csv]);
            if (!query.exec())
                qInfo() << query.lastError().text();
            ++cur;
            emit percentageComplete(int((cur + 0.0) / total * 100));
        }

        threadDb.commit();
        threadDb.close();
    });

    connect(thread, &QThread::started, this, &MainWindow::onImportStarted);
    connect(thread, &QThread::finished, this, &MainWindow::onImportFinished);
    connect(thread, &QThread::finished, [this, newCsvs] { curCsvs = newCsvs; });

    thread->start();
}