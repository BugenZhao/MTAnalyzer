#include "withlineflowplotwidget.h"
#include "ui_withlineflowplotwidget.h"
#include <QThread>
#include <QSqlQuery>
#include <QTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>
#include <QtConcurrent>
#include <QDateTimeAxis>
#include <QValueAxis>
#include "utilities/BDateTime.h"
#include "utilities/bdatabasemanager.h"


WithLineFlowPlotWidget::WithLineFlowPlotWidget(QWidget *parent) :
        BasePlotWidget(parent),
        ui(new Ui::WithLineFlowPlotWidget) {
    ui->setupUi(this);
    ui->startingTimeEdit->setTime(QTime::fromString("05:30", BugenZhao::TIME_FORMAT_NO_SEC));
    ui->endingTimeEdit->setTime(QTime::fromString("23:30", BugenZhao::TIME_FORMAT_NO_SEC));
    ui->timeStepBox->setValue(20);

    auto layout = new QVBoxLayout(ui->plotBox);
    layout->setContentsMargins(0, 0, 0, 0);
    chartView = new QChartView(ui->plotBox);
//    chartViewLeft->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    ui->plotBox->setLayout(layout);

    chartView->setRenderHint(QPainter::Antialiasing);

    connect(ui->analyzeButton, &QPushButton::clicked, this, &WithLineFlowPlotWidget::dynamicAnalyzeBetter);
//    connect(this, &WithLineFlowPlotWidget::preparedSplineChart, this, &WithLineFlowPlotWidget::setDateTimeSplineChart);

    connect(this, &WithLineFlowPlotWidget::preparedChart, this, &WithLineFlowPlotWidget::dynamicInitChart);
    connect(this, &WithLineFlowPlotWidget::newData, this, &WithLineFlowPlotWidget::dynamicAppendData);

}

WithLineFlowPlotWidget::~WithLineFlowPlotWidget() {
    delete ui;
}

void WithLineFlowPlotWidget::setBzEnabled(bool enabled) {
    ui->analyzeButton->setEnabled(enabled);
}

void WithLineFlowPlotWidget::dynamicAnalyzeBetter() {
    auto thread = QThread::create([this]() {
        auto _filterDataList = filterDataList;
        QMap<QString, bool> filterNeeded;

        QStringList linesToAnalyze;

        QStringList filterJudgmentPieces;
        for (const auto &filterData:_filterDataList) {
            if (filterData.first == "LINES") {
                for (const auto &item:filterData.second.keys()) {
                    if (filterData.second[item])
                        linesToAnalyze.append(item);
                }
                continue;
            }

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
            if (filterData.first == "PAY_TYPES") {
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


        QString TITLE = QString("Traffic Inflow Trend with Lines from %1 %2 to %3")
                .arg(dateStr)
                .arg(startingTimeStr)
                .arg(endingTimeStr);

        if (!filterJudgment.isEmpty()) TITLE += QString("\n Where %1").arg(filterJudgment);

        QVector<int> timestampsToDo;
        for (auto timestamp = startingTimestamp;
             timestamp <= endingTimestamp; timestamp = timestamp + 60 * timeStepMinutes) {
            timestampsToDo.push_back(timestamp);
        }

        auto queryStr = QString("SELECT\n"
                                "\tcount( * ) \n"
                                "FROM\n"
                                "\tbz \n"
                                "WHERE\n"
                                "\ttimestamp > %1 \n"
                                "\tAND timestamp < %2 \n"
                                "\tAND status = 1 \n"
                                "\tAND lineID = '%3' \n");

        if (!filterJudgment.isEmpty()) {
            queryStr += QString(" AND %1").arg(filterJudgment);
        }

        auto worker = [this, TITLE, timeStepMinutes, queryStr](
                int curTimestamp, const QString &line) -> QPair<int, int> {
            auto threadDb = BDatabaseManager::connection(
                    QString("plot_thread_") + QString::number(quintptr(QThread::currentThreadId())));
            QSqlQuery query(threadDb);


            auto curQueryStr = queryStr.arg(curTimestamp).arg(curTimestamp + 60 * timeStepMinutes).arg(line);

            if (!query.exec(curQueryStr))
                qInfo() << query.lastError().text();
            query.next();
            auto flow = query.value(0).toInt();

            threadDb.close();
            return {curTimestamp, flow};
        };

        qInfo() << linesToAnalyze;
        BDataTable emptyTable;
        for (int i = 0; i < linesToAnalyze.size(); ++i) {
            emptyTable.append(BDataList{});
        }
        emit preparedChart({TITLE,
                            linesToAnalyze,
                            emptyTable},
                           startingDateTime,
                           endingDateTime);

        BData tmpFlowData;

        QList<QPair<int, QPair<int, int>>> results;

        int cur = 0;
        for (auto timestamp:timestampsToDo) {
            auto _newData = BDataList();
            for (const auto &line:linesToAnalyze) {
                auto pair = worker(timestamp, line);
                qInfo() << timestamp * 1000L << pair.second;
                tmpFlowData = {QPointF{static_cast<qreal>(timestamp * 1000L),
                                       static_cast<qreal>(pair.second)}, ""};
                _newData.append(tmpFlowData);
            }
            QThread::msleep(expectedTimeMs() / timestampsToDo.size());
            emit newData(_newData);
            ++cur;
        }

        emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
                              3000);
    });

    thread->start();
    connect(thread, &QThread::started, this, &WithLineFlowPlotWidget::onAnalysisStarted);
    connect(thread, &QThread::finished, this, &WithLineFlowPlotWidget::onAnalysisFinished);
}

void WithLineFlowPlotWidget::dynamicInitChart(const WithLineFlowPlotWidget::BzChartData &chartBaseData,
                                              const QDateTime &dt0, const QDateTime &dt1) {
    auto oldChart = chartView->chart();
    auto chart = new QChart();

    if (speedLevel == FASTEST)
        chart->setAnimationOptions(QChart::NoAnimation);
    else
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

void WithLineFlowPlotWidget::dynamicAppendData(const BDataList &dataList) {
    auto axisX = dynamic_cast<QDateTimeAxis *>(_axisX);
    auto axisY = dynamic_cast<QValueAxis *>(_axisY);

    for (int i = 0; i < dataList.size(); ++i) {
        auto series = seriesList[i];
        auto point = dataList[i].first;
        series->append(point);
        maxY = (abs(point.y()) > maxY) ? abs(point.y()) : maxY;
    }

    axisY->setMax(static_cast<int>( maxY * 11 / 10 + 1));
    axisY->setMin(0);
}

void WithLineFlowPlotWidget::onAnalysisStarted() {
    emit statusBarMessage("Analyzing...", 0);
    ui->analyzeButton->setEnabled(false);
}

void WithLineFlowPlotWidget::onAnalysisFinished() {
    ui->analyzeButton->setEnabled(true);
}

void WithLineFlowPlotWidget::setDate(const QString &pureDateStr) {
    ui->dateEdit->setDate(QDate::fromString(pureDateStr, DATE_FORMAT));
}

void WithLineFlowPlotWidget::setFilterDataList(FilterDataList _filterDataList) {
    qInfo() << "filter data updated";
    this->filterDataList = _filterDataList;
}

