#include "stationflowplotwidget.h"
#include "ui_stationflowplotwidget.h"
#include <QThread>
#include <QSqlQuery>
#include <QTime>
#include <QSqlError>
#include <QtCore>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QPieSeries>
#include "utilities/BDateTime.h"
#include "utilities/bdatabasemanager.h"

StationFlowPlotWidget::StationFlowPlotWidget(QWidget *parent) :
        BasePlotWidget(parent),
        ui(new Ui::StationFlowPlotWidget) {
    qRegisterMetaType<PieData>("PieData");


    ui->setupUi(this);
    ui->startingTimeEdit->setTime(QTime::fromString("08:00", BugenZhao::TIME_FORMAT_NO_SEC));
    ui->endingTimeEdit->setTime(QTime::fromString("10:00", BugenZhao::TIME_FORMAT_NO_SEC));
    ui->timeStepBox->setValue(20);

    auto layout = new QHBoxLayout(ui->plotBox);
    layout->setContentsMargins(0, 0, 0, 0);
    chartViewLeft = new QChartView(ui->plotBox);
    chartViewRight = new QChartView(ui->plotBox);
    layout->addWidget(chartViewLeft);
    layout->addWidget(chartViewRight);
    ui->plotBox->setLayout(layout);

    chartViewLeft->setRenderHint(QPainter::Antialiasing);
    chartViewRight->setRenderHint(QPainter::Antialiasing);

    connect(ui->timeSlider, &QSlider::valueChanged, this, &StationFlowPlotWidget::onCurrentTimeUpdated);
    connect(ui->analyzeButton, &QPushButton::clicked, this, &StationFlowPlotWidget::analyze);
    connect(this, &StationFlowPlotWidget::preparedPieChart, this, &StationFlowPlotWidget::chart);
}

StationFlowPlotWidget::~StationFlowPlotWidget() {
    delete ui;
}

void StationFlowPlotWidget::setBzEnabled(bool enabled) {
    ui->analyzeButton->setEnabled(enabled);
    ui->sliderWidget->setEnabled(enabled);
    ui->detailedCountBox->setEnabled(enabled);
}

void StationFlowPlotWidget::onAnalysisStarted() {
    emit statusBarMessage("Analyzing...", 0);
    setBzEnabled(false);
}

void StationFlowPlotWidget::onAnalysisFinished() {
    setBzEnabled(true);
    for (const auto &charts:chartMap.values()) {
//        charts.first->setAnimationOptions(QChart::AllAnimations);
//        charts.second->setAnimationOptions(QChart::AllAnimations);
    }
}

void StationFlowPlotWidget::setDate(const QString &pureDateStr) {
    ui->dateEdit->setDate(QDate::fromString(pureDateStr, DATE_FORMAT));
}

void StationFlowPlotWidget::setFilterDataList(FilterDataList _filterDataList) {
    qInfo() << "filter data updated";
    this->filterDataList = _filterDataList;
}

void StationFlowPlotWidget::init() {
    auto dateStr = ui->dateEdit->text();
    auto startingTimeStr = ui->startingTimeEdit->text();
    auto endingTimeStr = ui->endingTimeEdit->text();

    startingTimestamp = BDateTime::bToLocalTimestamp(QString("%1 %2").arg(dateStr).arg(startingTimeStr));
    endingTimestamp = BDateTime::bToLocalTimestamp(QString("%1 %2").arg(dateStr).arg(endingTimeStr));
    stepTimestamp = ui->timeStepBox->value() * 60;
    auto count = (endingTimestamp - startingTimestamp) / stepTimestamp;
    ui->timeSlider->setMinimum(0);
    ui->timeSlider->setMaximum(count);
    ui->timeSlider->setValue(0);

    chartMap.clear();
}

void StationFlowPlotWidget::chart(QString title, PieData flows) {
    auto index = flows.first;

    auto detailedCount = ui->detailedCountBox->value();
    const auto DETAILED_COUNT_IN = std::min(detailedCount, flows.second.first.size());
    const auto DETAILED_COUNT_OUT = std::min(detailedCount, flows.second.second.size());
    int sum = 0;

    auto theme = QChart::ChartThemeBlueNcs;

    auto inflowChart = new QChart();
    inflowChart->setTheme(theme);
    if (detailedCount > 20 || speedLevel == FASTEST)
        inflowChart->setAnimationOptions(QChart::NoAnimation);
    else
        inflowChart->setAnimationOptions(QChart::AllAnimations);
    inflowChart->setTitle(title + " - Inflow");
    inflowChart->legend()->setAlignment(Qt::AlignRight);
//    inflowChart->legend()->hide();

    auto inflowSeries = new QPieSeries(inflowChart);

    for (int i = 0; i < DETAILED_COUNT_IN; ++i) {
        auto value = flows.second.first[i].second;
        auto label = QString("S%1: %2").arg(flows.second.first[i].first).arg(value);
        auto slice = new QPieSlice(label, value);
        if (i == 0) slice->setExploded(true);
        *inflowSeries << slice;
    }
    if (DETAILED_COUNT_IN < flows.second.first.size()) {
        sum = 0;
        for (int i = DETAILED_COUNT_IN; i < flows.second.first.size(); ++i) {
            sum += flows.second.first[i].second;
        }
    }

    *inflowSeries << new QPieSlice(QString("Others: %1").arg(sum), sum);
    inflowSeries->setLabelsVisible(true);

    inflowChart->addSeries(inflowSeries);
    inflowSeries->setUseOpenGL(true);


    auto outflowChart = new QChart();
    outflowChart->setTheme(theme);
    if (detailedCount > 20 || speedLevel == FASTEST)
        outflowChart->setAnimationOptions(QChart::NoAnimation);
    else
        outflowChart->setAnimationOptions(QChart::AllAnimations);
    outflowChart->setTitle(title + " - Outflow");
    outflowChart->legend()->setAlignment(Qt::AlignLeft);
//    outflowChart->legend()->hide();

    auto outflowSeries = new QPieSeries(outflowChart);

    for (int i = 0; i < DETAILED_COUNT_OUT; ++i) {
        auto value = flows.second.second[i].second;
        auto label = QString("S%1: %2").arg(flows.second.second[i].first).arg(value);
        auto slice = new QPieSlice(label, value);
        if (i == 0) slice->setExploded(true);
        *outflowSeries << slice;
    }
    if (DETAILED_COUNT_OUT < flows.second.second.size()) {
        sum = 0;
        for (int i = DETAILED_COUNT_OUT; i < flows.second.second.size(); ++i) {
            sum += flows.second.second[i].second;
        }
    }
    *outflowSeries << new QPieSlice(QString("Oth: %1").arg(sum), sum);
    outflowSeries->setLabelsVisible(true);

    outflowChart->addSeries(outflowSeries);
    outflowSeries->setUseOpenGL(true);


    chartMap.insert(index, {inflowChart, outflowChart});
    if (index == 0) onCurrentTimeUpdated(0);
    else ui->timeSlider->setValue(index);
}

void StationFlowPlotWidget::analyze() {
    init();

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


        auto dateStr = ui->dateEdit->text();
        auto startingTimeStr = ui->startingTimeEdit->text();
        auto endingTimeStr = ui->endingTimeEdit->text();
        qInfo() << QString("%1 %2").arg(dateStr).arg(startingTimeStr);

        auto date = ui->dateEdit->date();
        auto _startingTime = ui->startingTimeEdit->time();
        auto _endingTime = ui->endingTimeEdit->time();
        QDateTime startingDateTime(date, QTime(_startingTime.hour(), _startingTime.minute() / 10 * 10));
        QDateTime endingDateTime(date, QTime(_endingTime.hour(), _endingTime.minute() / 10 * 10));


        constexpr int _LENGTH = 15;
        QString TITLE = QString("Flow of Stations from %1 %2 to %3")
                .arg(dateStr);

        if (!filterJudgment.isEmpty()) TITLE += QString("\n Where %1").arg(filterJudgment);

        QVector<int> indicesToDo;
        for (auto index = 0;
             startingTimestamp + stepTimestamp * index <= endingTimestamp; ++index) {
            indicesToDo.push_back(index);
        }

        auto worker = [this, TITLE, _LENGTH, filterJudgment](
                int curIndex) -> QPair<int, QPair<QVector<QPair<QString, int>>, QVector<QPair<QString, int>>>> {
            auto threadDb = BDatabaseManager::connection(
                    QString("total_plot_thread_") + QString::number(quintptr(QThread::currentThreadId())));
            QSqlQuery query(threadDb);

            auto queryStr = QString("SELECT\n"
                                    "\tstationID, count( * ) flow\n"
                                    "FROM\n"
                                    "\tbz \n"
                                    "WHERE\n"
                                    "\ttimestamp > %1 \n"
                                    "\tAND timestamp < %2 \n"
                                    "\tAND status = %3 ");

            if (!filterJudgment.isEmpty()) {
                queryStr += QString(" AND %1").arg(filterJudgment);
            }
            queryStr += QString("GROUP BY stationID ORDER BY flow DESC");

            auto curTimestamp = startingTimestamp + stepTimestamp * curIndex;
            auto curQueryStr = queryStr.arg(curTimestamp).arg(curTimestamp + stepTimestamp);


            if (!query.exec(curQueryStr.arg(1)))
                qInfo() << query.lastError().text();
            QVector<QPair<QString, int>> inflow;
            while (query.next()) inflow.append({query.value(0).toString(), query.value(1).toInt()});

            if (!query.exec(curQueryStr.arg(0)))
                qInfo() << query.lastError().text();
            QVector<QPair<QString, int>> outflow;
            while (query.next()) outflow.append({query.value(0).toString(), query.value(1).toInt()});

//            auto inflowChart = new QChart();
//            inflowChart->setAnimationOptions(QChart::AllAnimations);
//            auto inflowSeries = new QPieSeries(inflowChart);
//            inflowSeries->setLabelsVisible(true);
//            for (const auto &pair:inflow) {
//                *inflowSeries << new QPieSlice(pair.first, pair.second);
//            }
//            inflowChart->addSeries(inflowSeries);


            threadDb.close();
            qInfo() << curTimestamp;

            return {curIndex, {inflow, outflow}};
        };

        for (auto index:indicesToDo) {
            auto flows = worker(index);
            auto title = TITLE
                    .arg(QDateTime::fromTime_t(startingTimestamp + stepTimestamp * index)
                                 .toString(TIME_FORMAT_NO_SEC))
                    .arg(QDateTime::fromTime_t(startingTimestamp + stepTimestamp * (index + 1))
                                 .toString(TIME_FORMAT_NO_SEC));
            emit preparedPieChart(title, flows);
            QThread::msleep(expectedTimeMs() / indicesToDo.size());
        }

        emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
                              3000);
    });

    connect(thread, &QThread::started, this, &StationFlowPlotWidget::onAnalysisStarted);
    connect(thread, &QThread::finished, this, &StationFlowPlotWidget::onAnalysisFinished);
    thread->start();
}

void StationFlowPlotWidget::onCurrentTimeUpdated(int index) {
    ui->currentTimeLabel->setText(QString("Current: %1").arg(
            QDateTime::fromTime_t(startingTimestamp + stepTimestamp * index).toString(TIME_FORMAT_NO_SEC)));

    if (chartMap.contains(index)) {
        auto charts = chartMap[index];
        chartViewLeft->setChart(charts.first);
        chartViewRight->setChart(charts.second);
    }
}

int StationFlowPlotWidget::expectedTimeMs() {
    switch (speedLevel) {
        case BugenZhao::FAST:
            return 10000;
        case BugenZhao::FASTER:
            return 6000;
        case BugenZhao::FASTEST:
            return 0;
        default:
            return 6000;
    }
}
