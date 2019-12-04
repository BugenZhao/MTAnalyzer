#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include "querywidget.h"
#include <string>

using std::string;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow),
          db(QSqlDatabase::addDatabase("QSQLITE")) {
    ui->setupUi(this);
    setupUi();

    auto importAction = new QAction(tr("&Open data set folder..."), this);
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(importAction);
    connect(importAction, &QAction::triggered, this, &MainWindow::importDataSet);

    connect(ui->filterButton, &QPushButton::clicked, this, &MainWindow::test);

    db.setDatabaseName("file::memory:");
    db.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
    db.open();

    QSqlQuery query;
    qInfo() << query.exec("DROP TABLE bz");
    qInfo() << query.exec("CREATE TABLE \"bz\" (\n"
                          "  \"file\" integer,\n"
                          "  \"time\" text(20),\n"
                          "  \"lineID\" text(3),\n"
                          "  \"stationID\" integer,\n"
                          "  \"deviceID\" integer,\n"
                          "  \"status\" integer,\n"
                          // "  \"userID\" text(80),\n"
                          "  \"payType\" integer\n"
                          ");");
    qInfo() << query.exec("\n"
                          "CREATE INDEX \"time\"\n"
                          "ON \"bz\" (\n"
                          "  \"time\"\n"
                          ");");
}

void MainWindow::setupUi() {
    ui->filterTree->setHeaderLabels({tr("Filters")});
    auto tab3Layout = new QVBoxLayout(ui->tab3);
    queryWidget = new QueryWidget(&db, this);
    tab3Layout->addWidget(queryWidget);
    ui->tab3->setLayout(tab3Layout);

    ui->progressBar->setMaximum(100);
    connect(this, &MainWindow::percentageComplete, ui->progressBar, &QProgressBar::setValue);
}

void MainWindow::importDataSet() {
//    auto dir = QFileDialog::getExistingDirectory(this, tr("Select data set directory"));
    auto dir = QString("/Users/bugenzhao/Codes/CLionProjects/FinalProject/dataset_CS241");
    if (dir.isEmpty()) { return; }
    mainDir = QDir(dir);
    adjacencyDir = mainDir;
    adjacencyDir.cd("adjacency_adjacency");
    dataSetDir = mainDir;
    dataSetDir.cd("dataset");

    updateFilterWidgetAdjacency();
    updateFilterWidgetDataSet();

    updateFilterWidgetOthers({}, {});

    ui->filterButton->setEnabled(true);
}

void MainWindow::updateFilterWidgetAdjacency() {
    auto adjacencyCsv = adjacencyDir.entryList(QStringList() << "*.csv", QDir::Files)[0];

    auto adjacencyItem = new QTreeWidgetItem(ui->filterTree);
    adjacencyItem->setText(0, tr("Adjacency table"));
    adjacencyItem->setText(1, tr("ADJ_TABLE"));
    adjacencyItem->setCheckState(0, Qt::Checked);
    adjacencyItem->setFlags(adjacencyItem->flags() | Qt::ItemIsAutoTristate);
    adjacencyItem->setDisabled(true);

    auto csvItem = new QTreeWidgetItem(adjacencyItem);
    csvItem->setText(0, adjacencyCsv);
    csvItem->setText(1, "ADJ_DATA");
    csvItem->setCheckState(0, Qt::Checked);
    csvItem->setFlags(csvItem->flags() | Qt::ItemIsAutoTristate);
}

void MainWindow::updateFilterWidgetDataSet() {
    auto dataSets = dataSetDir.entryList(QStringList() << "*.csv", QDir::Files);
    auto datesMap = QMap<QString, QVector<QString>>();

    auto pattern = QRegExp("\\d{4}-\\d{2}-\\d{2}");
    for (const auto &csv:dataSets) {
//        qInfo() << csv;
        fileId.insert(csv, fileId.size());
        auto date = csv.split("_")[1];
        if (pattern.exactMatch(date)) {
            if (!datesMap.contains(date)) datesMap.insert(date, QVector<QString>());
            datesMap[date].push_back(csv);
        }
    }

    dataSetItem = new QTreeWidgetItem(ui->filterTree);
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
        dateItem->setCheckState(0, Qt::Checked);
        dateItem->setFlags(dateItem->flags() | Qt::ItemIsAutoTristate);

        for (const auto &csv:datesMap[date]) {
            auto csvItem = new QTreeWidgetItem(dateItem);
            csvItem->setText(0, csv);
            csvItem->setText(1, "DATA");
            csvItem->setCheckState(0, Qt::Checked);
        }
    }
}

void MainWindow::updateFilterWidgetOthers(const QStringList &lines, const QStringList &payTypes) {
    auto linesItem = new QTreeWidgetItem(ui->filterTree);
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

    auto payTypesItem = new QTreeWidgetItem(ui->filterTree);
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

void MainWindow::test() {
    QSet<QString> newCsvs;
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
        int total = toInsert.size();
        int cur = 0;
        emit percentageComplete(0);

        auto threadDb = QSqlDatabase::addDatabase("QSQLITE", "thread");
        threadDb.setDatabaseName("file::memory:");
        threadDb.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
        threadDb.open();
        qInfo() << threadDb.tables();

        threadDb.transaction();

        QSqlQuery query(threadDb);


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

        for (const auto &csv:toDelete) {
            query.prepare("DELETE FROM bz WHERE file = :file");
            query.bindValue(":file", fileId[csv]);
            if (!query.exec())
                qInfo() << query.lastError().text();
        }

        threadDb.commit();
        threadDb.close();
    });
    connect(thread, &QThread::started, this, &MainWindow::onImportStarted);
    connect(thread, &QThread::finished, this, &MainWindow::onImportFinished);
    connect(thread, &QThread::finished, [this, newCsvs] { curCsvs = newCsvs; });

    thread->start();
}


MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_tabs_currentChanged(int index) {
//    if (index == 2) resize(1200, 900);
//    else resize(800, 600);
}

void MainWindow::onImportStarted() {
    ui->filterButton->setEnabled(false);
    queryWidget->setBzEnabled(false);
    statusBar()->showMessage("Please wait while data sets are processing...");
}

void MainWindow::onImportFinished() {
    ui->filterButton->setEnabled(true);
    queryWidget->setBzEnabled(true);
    statusBar()->showMessage("Done", 3000);
}
