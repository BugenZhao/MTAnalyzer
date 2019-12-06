#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include "querywidget.h"
#include <string>
#include "dataimportingthread.h"
#include <QtConcurrent>
#include <QFuture>

using std::string;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow),
          db(QSqlDatabase::addDatabase("QSQLITE")) {
    ui->setupUi(this);
    setupBzUi();

    auto importAction = new QAction(tr("&Open data set folder..."), this);
    importAction->setShortcut(QKeySequence::Open);
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(importAction);
    connect(importAction, &QAction::triggered, this, &MainWindow::preloadDataSets);

    auto showDockAction = new QAction(tr("&Importer && Filter"), this);
    showDockAction->setShortcut(QKeySequence("Ctrl+I"));
    showDockAction->setCheckable(true);
    auto windowMenu = menuBar()->addMenu(tr("&Window"));
    windowMenu->addAction(showDockAction);
    connect(ui->dockWidget, &QDockWidget::visibilityChanged, showDockAction, &QAction::setChecked);
    connect(showDockAction, &QAction::toggled, ui->dockWidget, &QDockWidget::setVisible);

    connect(ui->filterButton, &QPushButton::clicked, this, &MainWindow::importFilteredAll);

    db.setDatabaseName("file::memory:");
    db.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
    db.open();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupBzUi() {
    ui->filterTree->setHeaderLabels({tr("Filters")});

    auto tab1Layout = new QVBoxLayout(ui->tab1);
    tab1Layout->addWidget(ui->plotTabs);
    tab1Layout->setContentsMargins(0, 0, 0, 0);
    ui->tab1->setLayout(tab1Layout);

    plotWidgets.clear();
    for (int i = 0; i < ui->plotTabs->count(); ++i) {
        auto plotTab = ui->plotTabs->widget(i);
        auto plotWidget = new PlotWidget(plotTab);
        plotWidgets.push_back(plotWidget);
        auto tabLayout = new QVBoxLayout(plotTab);
        tabLayout->addWidget(plotWidget);
        tabLayout->setContentsMargins(12, 0, 12, 0);
        plotTab->setLayout(tabLayout);

        connect(plotWidget, &PlotWidget::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);
    }

    auto tab2Layout = new QVBoxLayout(ui->tab2);
    pathSearchWidget = new PathSearchWidget(&adj, this);
    tab2Layout->addWidget(pathSearchWidget);
    ui->tab2->setLayout(tab2Layout);

    auto tab3Layout = new QVBoxLayout(ui->tab3);
    queryWidget = new QueryWidget(&db, this);
    tab3Layout->addWidget(queryWidget);
    ui->tab3->setLayout(tab3Layout);

    ui->progressBar->setMaximum(100);
    connect(this, &MainWindow::percentageComplete, ui->progressBar, &QProgressBar::setValue);

    connect(queryWidget, &QueryWidget::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);
    connect(pathSearchWidget, &PathSearchWidget::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);
    connect(this, &MainWindow::statusBarMessage, ui->statusBar, &QStatusBar::showMessage);

}

void MainWindow::preloadDataSets() {
    ui->filterTree->clear();
    curCsvs.clear();

    QSqlQuery query;
    qInfo() << query.exec("DROP TABLE bz");
    qInfo() << query.exec("CREATE TABLE \"bz\" (\n"
                          "  \"file\" integer,\n"
                          "  \"time\" text(20),\n"
                          "  \"lineID\" text(3),\n"
                          "  \"stationID\" integer,\n"
                          "  \"deviceID\" integer,\n"
                          "  \"status\" integer,\n"
                          "  \"payType\" integer,\n"
                          "  \"userID\" text(40)\n"
                          ");");
    qInfo() << query.exec("\n"
                          "CREATE INDEX \"userID\"\n"
                          "ON \"bz\" (\n"
                          "  \"userID\"\n"
                          ");");
    qInfo() << query.exec("\n"
                          "CREATE INDEX \"file\"\n"
                          "ON \"bz\" (\n"
                          "  \"file\"\n"
                          ");");

//    auto dir = QFileDialog::getExistingDirectory(this, tr("Select data set directory"));
    auto dir = QString("/Users/bugenzhao/Codes/CLionProjects/FinalProject/dataset_CS241");
    if (dir.isEmpty()) { return; }
    mainDir = QDir(dir);
    adjacencyDir = mainDir;
    adjacencyDir.cd("adjacency_adjacency");
    dataSetDir = mainDir;
    dataSetDir.cd("dataset");


    importItem = new QTreeWidgetItem(ui->filterTree);
    importItem->setText(0, tr("Importer"));
    importItem->setText(1, tr("IMPORTER"));
    importItem->setCheckState(0, Qt::Checked);
    importItem->setFlags(importItem->flags() & (~Qt::ItemIsUserCheckable) | Qt::ItemIsAutoTristate);
    importItem->setExpanded(true);


    updateFilterWidgetImportAdjacency(importItem);
    updateFilterWidgetImportDataSet(importItem);
    updateFilterWidgetImportFields(importItem);
    updateFilterWidgetFiltersFields({}, {});

    importAdjacency();

    for (const auto &plotWidget:plotWidgets) {
        plotWidget->bzClear();
        plotWidget->setDate(importItem->child(1)->child(0)->text(0));
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
    for (const auto &csv:dataSets) {
//        qInfo() << csv;
        fileId.insert(csv, fileId.size());
        auto date = csv.split("_")[1];
        if (pattern.exactMatch(date)) {
            if (!datesMap.contains(date)) datesMap.insert(date, QVector<QString>());
            datesMap[date].push_back(csv);
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
    fieldsItem->setFlags(fieldsItem->flags() | Qt::ItemIsAutoTristate);
    fieldsItem->setExpanded(true);

    auto userIdItem = new QTreeWidgetItem(fieldsItem);
    userIdItem->setText(0, tr("User ID"));
    userIdItem->setText(1, tr("USER_ID"));
    userIdItem->setCheckState(0, Qt::Unchecked);
    userIdItem->setFlags(userIdItem->flags() | Qt::ItemIsAutoTristate);
}

void MainWindow::updateFilterWidgetFiltersFields(const QStringList &payTypes, const QStringList &lines) {

    auto filtersFields = new QTreeWidgetItem(ui->filterTree);
    filtersFields->setText(0, tr("Filters for plots"));
    filtersFields->setText(1, "FILTERS");
    filtersFields->setCheckState(0, Qt::Checked);
    filtersFields->setFlags(filtersFields->flags() | Qt::ItemIsAutoTristate);
    filtersFields->setExpanded(true);

    auto linesItem = new QTreeWidgetItem(filtersFields);
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

    auto payTypesItem = new QTreeWidgetItem(filtersFields);
    payTypesItem->setText(0, tr("Payment types"));
    payTypesItem->setText(1, "PAY_TYPES");
    payTypesItem->setCheckState(0, Qt::Checked);
    payTypesItem->setFlags(payTypesItem->flags() | Qt::ItemIsAutoTristate);

    for (const auto &s:QStringList{"0", "1", "2"}) {
        auto item = new QTreeWidgetItem(payTypesItem);
        item->setText(0, s);
        item->setText(1, "PAY_TYPE");
        item->setCheckState(0, Qt::Checked);
        item->setFlags(item->flags() | Qt::ItemIsAutoTristate);
    }
}

void MainWindow::importFilteredAll() {
    importFilteredDataMt();
}

void MainWindow::importFilteredDataMt() {
    auto thread = QThread::create([this] {
        auto startTime = QTime::currentTime();

        auto worker = [this](const QString &csv) -> QStringList {
            auto filePath = dataSetDir.absolutePath() + QDir::separator() + csv;
            QFile csvFile(filePath);
            qInfo() << filePath;

            if (!csvFile.open(QFile::ReadOnly | QFile::Text)) {
                return {};
                //TODO
            }

            QTextStream stream(&csvFile);
            stream.readLine();

            QStringList queries;

            while (!stream.atEnd()) {
                auto line = stream.readLine();
                auto pieces = line.split(',');
                QStringList queryPieces;
                queryPieces << QString::number(fileId[csv])
                            << QString("\"%1\"").arg(pieces[0])
                            << QString("\"%1\"").arg(pieces[1])
                            << pieces[2]
                            << pieces[3]
                            << pieces[4]
                            << pieces[6];
                if (curUserIdChecked) queryPieces << QString("\"%1\"").arg(pieces[5]);
                else queryPieces << "NULL";

                queries.append(QString("INSERT INTO bz VALUES (%1)").arg(queryPieces.join(", ")));
            }
            return queries;
        };

        auto dataSetItem = importItem->child(1);
        auto userIdItem = importItem->child(2)->child(0);

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
        bool needToClearUserId = false;

        if (userIdItem->checkState(0) == Qt::Checked && !curUserIdChecked) {
            curUserIdChecked = true;
            needToReload = true;
        } else if (userIdItem->checkState(0) == Qt::Unchecked && curUserIdChecked) {
            curUserIdChecked = false;
            needToClearUserId = true;
        }

        auto threadDb = QSqlDatabase::addDatabase("QSQLITE", "thread");
        threadDb.setDatabaseName("file::memory:");
        threadDb.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
        threadDb.open();
        QSqlQuery query(threadDb);

//        qInfo() << threadDb.tables();

        threadDb.transaction();

        if (newCsvs.isEmpty()) {
            emit statusBarMessage("Removing all data...");
            if (!query.exec("DELETE FROM bz"))
                qInfo() << query.lastError().text();
        } else {
            QList<QString> toInsert;
            QList<QString> toDelete;


            if (needToReload) {
                emit statusBarMessage("Removing all data...");
                if (!query.exec("DELETE FROM bz"))
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

            emit statusBarMessage("Loading data sets...");
            auto results = QtConcurrent::blockingMapped(toInsert, std::function<QStringList(const QString &)>(worker));

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
            total = results.size();
            cur = 0;
            QVector<int> perTimesMs;

            for (auto &result:results) {
                auto t0 = QTime::currentTime();

                for (const auto &queryText:result) {
                    if (!query.exec(queryText))
                        qInfo() << query.lastError().text();
                }
                result.clear();

                perTimesMs.push_back(t0.msecsTo(QTime::currentTime()));
                emit statusBarMessage(
                        QString("Please wait while data sets are processing. Remaining time: %1s")
                                .arg((total - cur) * BugenZhao::bAverage(perTimesMs) / 1000));
                emit percentageComplete((++cur) * 100 / total);
            }

            // Clear userId [optional]
            if (needToClearUserId) {
                emit statusBarMessage("Clearing User ID...");
                emit percentageComplete(0);
                if (!query.exec("UPDATE bz SET userId = NULL"))
                    qInfo() << query.lastError().text();
                emit percentageComplete(100);
            }
        }

        threadDb.commit();
        threadDb.close();

        curCsvs = newCsvs;

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
}

void MainWindow::onImportStarted() {
    ui->filterButton->setEnabled(false);
    ui->progressBar->setValue(0);
    queryWidget->setBzEnabled(false);
    pathSearchWidget->setBzEnabled(false);
    for (auto plotWidget:plotWidgets) {
        plotWidget->setBzEnabled(false);
    }
}

void MainWindow::onImportFinished() {
    ui->filterButton->setEnabled(true);
    ui->progressBar->setValue(100);
    queryWidget->setBzEnabled(true);
    pathSearchWidget->setBzEnabled(true);
    for (auto plotWidget:plotWidgets) {
        plotWidget->setBzEnabled(true);
    }
}

void MainWindow::testPlot() {
    PlotWidget::BzChartData chartData;
    chartData.title = "Test";
    chartData.seriesNames = QStringList() << "Line 1";
    BDataList dataList;
    for (int i = 0; i < 10; ++i) {
        dataList.append(BData{QPointF{static_cast<qreal>(i), static_cast<qreal>(i * i)}, "Test"});
    }
    chartData.dataTable = QList<BDataList>() << dataList;
    plotWidgets[0]->setDateTimeSplineChart(chartData);
}


[[deprecated]] void MainWindow::importFilteredData() {
    QSet<QString> newCsvs;

    auto dataSetItem = importItem->child(1);

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

        auto threadDb = QSqlDatabase::addDatabase("QSQLITE", "thread");
        threadDb.setDatabaseName("file::memory:");
        threadDb.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
        threadDb.open();
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
