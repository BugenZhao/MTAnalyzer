#include "totalflowplotwidget.h"
#include "ui_totalflowplotwidget.h"
#include <QThread>
#include <QSqlQuery>
#include <QTime>
#include <QSqlError>
#include <QtCore>
#include <QDateTimeAxis>
#include <QValueAxis>
#include "utilities/BDateTime.h"
#include "utilities/bdatabasemanager.h"

TotalFlowPlotWidget::TotalFlowPlotWidget(QWidget *parent) :
        BasePlotWidget(parent),
        ui(new Ui::TotalFlowPlotWidget) {
    ui->setupUi(this);
    ui->startingTimeEdit->setTime(QTime::fromString("05:30", BugenZhao::TIME_FORMAT_NO_SEC));
    ui->endingTimeEdit->setTime(QTime::fromString("23:30", BugenZhao::TIME_FORMAT_NO_SEC));
    ui->timeStepBox->setValue(5);

    auto layout = new QVBoxLayout(ui->plotBox);
    layout->setContentsMargins(0, 0, 0, 0);
    chartView = new QChartView(ui->plotBox);
    layout->addWidget(chartView);
    ui->plotBox->setLayout(layout);

    chartView->setRenderHint(QPainter::Antialiasing);

    connect(ui->analyzeButton, &QPushButton::clicked, this, &TotalFlowPlotWidget::dynamicAnalyzeBetter);
    connect(this, &TotalFlowPlotWidget::preparedChart, this, &TotalFlowPlotWidget::dynamicInitChart);
    connect(this, &TotalFlowPlotWidget::newData, this, &TotalFlowPlotWidget::dynamicAppendData);
}

TotalFlowPlotWidget::~TotalFlowPlotWidget() {
    delete ui;
}

void TotalFlowPlotWidget::setBzEnabled(bool enabled) {
    ui->analyzeButton->setEnabled(enabled);
}

void TotalFlowPlotWidget::onAnalysisStarted() {
    emit statusBarMessage("Analyzing...", 0);
    ui->analyzeButton->setEnabled(false);
}

void TotalFlowPlotWidget::onAnalysisFinished() {
    ui->analyzeButton->setEnabled(true);
}

void TotalFlowPlotWidget::setDate(const QString &pureDateStr) {
    ui->dateEdit->setDate(QDate::fromString(pureDateStr, DATE_FORMAT));
}

void TotalFlowPlotWidget::setFilterDataList(FilterDataList _filterDataList) {
    qInfo() << "filter data updated";
    this->filterDataList = _filterDataList;
}


void TotalFlowPlotWidget::dynamicInitChart(const BasePlotWidget::BzChartData &chartBaseData, const QDateTime &dt0,
                                           const QDateTime &dt1) {
    auto oldChart = chartView->chart();
    auto chart = new QChart();

    chart->setAnimationOptions(QChart::SeriesAnimations);

    _axisX = new QDateTimeAxis(chart);
    auto axisX = dynamic_cast<QDateTimeAxis *>(_axisX);
    axisX->setFormat(TIME_FORMAT_NO_SEC);
    axisX->setTickCount(19);
    chart->addAxis(axisX, Qt::AlignBottom);

    _axisY = new QValueAxis(chart);
    auto axisY = dynamic_cast<QValueAxis *>(_axisY);
    axisY->setLabelFormat("%d");
    axisY->setTickCount(11);
    chart->addAxis(axisY, Qt::AlignLeft);

    chart->setTitle(chartBaseData.title);

    int cur = 0;
    maxY = 0;
    seriesList.clear();

    for (const auto &dataList:chartBaseData.dataTable) {
        auto series = new QSplineSeries(chart);
        series->setName(chartBaseData.seriesNames[cur++]);
        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        series->setUseOpenGL(true);

        seriesList.push_back(series);
    }

    axisX->setMin(dt0);
    axisX->setMax(dt1);

    axisY->setMin(0.0);
    axisY->setMax(static_cast<int>( maxY * 11 / 10 + 1));

    chartView->setChart(chart);
    delete oldChart;
}

void TotalFlowPlotWidget::dynamicAppendData(const BDataList &dataList) {
    auto axisX = dynamic_cast<QDateTimeAxis *>(_axisX);
    auto axisY = dynamic_cast<QValueAxis *>(_axisY);

    for (int i = 0; i < dataList.size(); ++i) {
        auto series = seriesList[i];
        auto point = dataList[i].first;
//        auto points = series->points();
//        points.append(point);
//        series->replace(points);
        series->append(point);
//        QApplication::processEvents();
        maxY = (point.y() > maxY) ? point.y() : maxY;
    }

    axisY->setMin(0.0);
    axisY->setMax(static_cast<int>( maxY * 11 / 10 + 1));
}

void TotalFlowPlotWidget::dynamicAnalyzeBetter() {
    auto thread = QThread::create([this]() {
        auto _filterDataList = filterDataList;
        QMap<QString, bool> filterNeeded;

        QStringList filterJudgmentPieces;
        for (const auto &filterData:_filterDataList) {
            bool allChecked = true;
            bool anyChecked = false;

            qInfo() << filterData.second;

            for (bool checked:filterData.second) {
                allChecked &= checked;
                anyChecked |= checked;
            }

            if (!anyChecked) {
                emit statusBarMessage("Some filters are set incorrectly", 5000);
                return;
            }
            if (allChecked) continue;

            QStringList pieces;
            if (filterData.first == "LINES") {
                for (const auto &item:filterData.second.keys()) {
                    if (filterData.second[item])
                        pieces.push_back(QString("lineId = '%1'").arg(item));
                }
            } else if (filterData.first == "PAY_TYPES") {
                for (const auto &item:filterData.second.keys()) {
                    if (filterData.second[item])
                        pieces.push_back(QString("payType = %1").arg(item));
                }
            }
            filterJudgmentPieces.push_back(QString("( %1 )").arg(pieces.join(" OR ")));
        }

        auto filterJudgment = filterJudgmentPieces.join(" AND ");
        qInfo() << filterJudgment;


        auto startTime = QTime::currentTime();

        auto timeStepMinutes = ui->timeStepBox->value();

        auto dateStr = ui->dateEdit->text();
        auto startingTimeStr = ui->startingTimeEdit->text();
        auto endingTimeStr = ui->endingTimeEdit->text();
        qInfo() << QString("%1 %2").arg(dateStr).arg(startingTimeStr);
        int startingTimestamp = BDateTime::bToLocalTimestamp(QString("%1 %2").arg(dateStr).arg(startingTimeStr));
        int endingTimestamp = BDateTime::bToLocalTimestamp(QString("%1 %2").arg(dateStr).arg(endingTimeStr));

        auto date = ui->dateEdit->date();
        auto _startingTime = ui->startingTimeEdit->time();
        auto _endingTime = ui->endingTimeEdit->time();
        QDateTime startingDateTime(date, QTime(_startingTime.hour(), _startingTime.minute() / 10 * 10));
        QDateTime endingDateTime(date, QTime(_endingTime.hour(), _endingTime.minute() / 10 * 10));


        constexpr int _LENGTH = 15;
        QString TITLE = QString("Flow from %1 %2 to %3")
                .arg(dateStr)
                .arg(startingTimeStr)
                .arg(endingTimeStr);

        if (!filterJudgment.isEmpty()) TITLE += QString("\n where %1").arg(filterJudgment);

        QVector<int> timestampsToDo;
        for (auto timestamp = startingTimestamp;
             timestamp <= endingTimestamp; timestamp = timestamp + 60 * timeStepMinutes) {
            timestampsToDo.push_back(timestamp);
        }

        int curPopulation = 0;
        auto worker = [this, TITLE, _LENGTH, timeStepMinutes, filterJudgment, &curPopulation](
                int curTimestamp) -> QPair<int, int> {
            auto threadDb = BDatabaseManager::connection(
                    QString("total_plot_thread_") + QString::number(quintptr(QThread::currentThreadId())));
            QSqlQuery query(threadDb);

            auto queryStr = QString("SELECT\n"
                                    "\tcount( * ) \n"
                                    "FROM\n"
                                    "\tbz \n"
                                    "WHERE\n"
                                    "\ttimestamp > %1 \n"
                                    "\tAND timestamp < %2 \n"
                                    "\tAND status = %3 ");

            if (!filterJudgment.isEmpty()) {
                queryStr += QString(" AND %1").arg(filterJudgment);
            }

            auto curQueryStr = queryStr.arg(curTimestamp).arg(curTimestamp + 60 * timeStepMinutes);

            if (!query.exec(curQueryStr.arg(1)))
                qInfo() << query.lastError().text();
            query.next();
            curPopulation += query.value(0).toInt();

            if (!query.exec(curQueryStr.arg(0)))
                qInfo() << query.lastError().text();
            query.next();
            curPopulation -= query.value(0).toInt();

            threadDb.close();
            qInfo() << curTimestamp;
            return {curTimestamp, curPopulation};
        };


        emit preparedChart({TITLE,
                            QStringList{"Flow"},
                            BDataTable{{}}},
                           startingDateTime,
                           endingDateTime);

        BData flowData;

        QList<QPair<int, QPair<int, int>>> results;

        int cur = 0;
        for (auto timestamp:timestampsToDo) {
            auto pair = worker(timestamp);
            QThread::msleep(expectedTimeMs() / timestampsToDo.size());

            qInfo() << timestamp * 1000L << pair.second;

            flowData = {QPointF{static_cast<qreal>(timestamp * 1000L),
                                static_cast<qreal>(pair.second)}, ""};

            emit newData(BDataList{flowData});
            ++cur;
        }

        emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
                              3000);
    });

    thread->start();
    connect(thread, &QThread::started, this, &TotalFlowPlotWidget::onAnalysisStarted);
    connect(thread, &QThread::finished, this, &TotalFlowPlotWidget::onAnalysisFinished);
}
