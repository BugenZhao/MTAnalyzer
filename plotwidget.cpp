#include "plotwidget.h"
#include "ui_plotwidget.h"
#include <QThread>
#include <QSqlQuery>
#include <QTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>
#include <QtConcurrent>
#include <QDateTimeAxis>
#include <QValueAxis>


PlotWidget::PlotWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::PlotWidget) {
    qRegisterMetaType<BzChartData>("BzChartData");
    qRegisterMetaType<BDataList>("BDataList");


    ui->setupUi(this);
    ui->startingTimeEdit->setTime(QTime::fromString("07:00", BugenZhao::TIME_FORMAT_NO_SEC));
    ui->endingTimeEdit->setTime(QTime::fromString("10:00", BugenZhao::TIME_FORMAT_NO_SEC));

    auto layout = new QVBoxLayout(ui->plotBox);
    layout->setContentsMargins(0, 0, 0, 0);
    chartView = new QChartView(ui->plotBox);
//    chartView->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    ui->plotBox->setLayout(layout);

    chartView->setRenderHint(QPainter::Antialiasing);

    connect(ui->analyzeButton, &QPushButton::clicked, this, &PlotWidget::dynamicAnalyze);
//    connect(this, &PlotWidget::preparedSplineChart, this, &PlotWidget::setDateTimeSplineChart);

    connect(this, &PlotWidget::preparedChart, this, &PlotWidget::dynamicInitChart);
    connect(this, &PlotWidget::newData, this, &PlotWidget::dynamicAppendData);

}

PlotWidget::~PlotWidget() {
    delete ui;
}

void PlotWidget::setBzEnabled(bool enabled) {
    ui->analyzeButton->setEnabled(enabled);
}

void PlotWidget::setDateTimeSplineChart(const PlotWidget::BzChartData &chartData, bool animated) {
    auto oldChart = chartView->chart();
    auto chart = new QChart();

    if (animated)
        chart->setAnimationOptions(QChart::SeriesAnimations);
    else
        chart->setAnimationOptions(QChart::NoAnimation);

    auto axisX = new QDateTimeAxis(chart);
    axisX->setFormat(TIME_FORMAT_NO_SEC);
    axisX->setTickCount(19);
    chart->addAxis(axisX, Qt::AlignBottom);

    auto axisY = new QValueAxis(chart);
    axisY->setLabelFormat("%d");
    axisY->setTickCount(11);
    chart->addAxis(axisY, Qt::AlignLeft);

    chart->setTitle(chartData.title);

    int cur = 0;
    qreal max = INT_MIN;
    for (const auto &dataList:chartData.dataTable) {
        auto series = new QSplineSeries(chart);
        series->setName(chartData.seriesNames[cur++]);
        for (const auto &data:dataList) {
            series->append(data.first);
            max = (data.first.y() > max) ? data.first.y() : max;
        }
        chart->addSeries(series);

        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
    axisY->setMin(0.0);
    axisY->setMax(static_cast<int>( max * 11 / 10));

    chartView->setChart(chart);
    delete oldChart;
}

[[deprecated]] void PlotWidget::analyze() {
    auto thread = QThread::create([this]() {
        auto startTime = QTime::currentTime();

        auto date = ui->dateEdit->date();
        auto timeStepTenMinutes = ui->timeStepBox->value();
        auto stationId = ui->stationBox->text();
        auto _startingTime = ui->startingTimeEdit->time();
        auto _endingTime = ui->endingTimeEdit->time();
        QDateTime startingDateTime(date, QTime(_startingTime.hour(), _startingTime.minute() / 10 * 10));
        QDateTime endingDateTime(date, QTime(_endingTime.hour(), _endingTime.minute() / 10 * 10));

        constexpr int _LENGTH = 15;
        const QString TITLE = QString("Traffic inflow and outflow trend of Station %1 from %2 to %3")
                .arg(stationId)
                .arg(startingDateTime.toString(DATE_TIME_FORMAT_NO_SEC))
                .arg(endingDateTime.toString(DATE_TIME_FORMAT_NO_SEC));

        QVector<QDateTime> dateTimesToDo;
        for (auto curDateTime = startingDateTime;
             curDateTime <= endingDateTime; curDateTime = curDateTime.addSecs(10 * 60)) {
            dateTimesToDo.push_back(curDateTime);
        }

        auto worker = [this, stationId, TITLE, _LENGTH](
                const QDateTime &curDateTime) -> QPair<QDateTime, QPair<int, int>> {
            auto threadDb = QSqlDatabase::addDatabase(
                    "QSQLITE",
                    QString("plot_thread_") + QString::number(quintptr(QThread::currentThreadId())));
            threadDb.setDatabaseName("file::memory:");
            threadDb.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
            threadDb.open();
            QSqlQuery query(threadDb);

            const auto queryStr = QString("SELECT\n"
                                          "\tcount( * ) \n"
                                          "FROM\n"
                                          "\tbz \n"
                                          "WHERE\n"
                                          "\ttime LIKE ( '%1%' ) \n"
                                          "\tAND status = %2 \n"
                                          "\tAND stationID = ") + stationId;

            auto curTimePrefixStr = curDateTime.toString(DATE_TIME_FORMAT).mid(0, _LENGTH);
//            qInfo().noquote() << queryStr.arg(curTimePrefixStr).arg(1);
            auto curQueryStr = queryStr.arg(curTimePrefixStr);

            if (!query.exec(curQueryStr.arg(1)))
                qInfo() << query.lastError().text();
            query.next();
            auto inflow = query.value(0).toInt();

            if (!query.exec(curQueryStr.arg(0)))
                qInfo() << query.lastError().text();
            query.next();
            auto outflow = query.value(0).toInt();

            threadDb.close();
            return {curDateTime, QPair{inflow, outflow}};
        };

//        auto results = QtConcurrent::blockingMapped(
//                dateTimesToDo,
//                std::function<QPair<QDateTime, QPair<int, int>>(const QDateTime &)>(worker));




//        QList<QPair<QDateTime, QPair<int, int>>> results;
//        for (const auto &dateTime:dateTimesToDo) {
//            auto pair = worker(dateTime);
//
//            emit preparedSplineChart({QString("Results for station ") + stationId,
//                                      QStringList{"Inflow", "Outflow"},
//                                      BDataTable{
//                                              BDataList{
//                                                      BData{QPointF{static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
//                                                                    static_cast<qreal>(pair.second.first)}, ""}},
//                                              BDataList{
//                                                      BData{QPointF{
//                                                              static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
//                                                              static_cast<qreal>(pair.second.second)}, ""}}}},
//                                     false);
//        }
//
//        emit preparedSplineChart({QString("Results for station ") + stationId,
//                                  QStringList{"Inflow", "Outflow"},
//                                  BDataTable{}}, true);
//
//
//        emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
//                              3000);
//    });

        BDataList inflowDataList;
        BDataList outflowDataList;
        QList<QPair<QDateTime, QPair<int, int>>> results;

        int cur = 0;
        for (const auto &dateTime:dateTimesToDo) {
            auto pair = worker(dateTime);

            if (cur % timeStepTenMinutes == 0) {
                inflowDataList.push_back(
                        {QPointF{static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
                                 static_cast<qreal>(0.0)}, ""});
                outflowDataList.push_back(
                        {QPointF{static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
                                 static_cast<qreal>(0.0)}, ""});
            }
            inflowDataList.last().first.ry() += static_cast<qreal>(pair.second.first);
            outflowDataList.last().first.ry() += static_cast<qreal>(pair.second.second);

            if (cur % timeStepTenMinutes == timeStepTenMinutes - 1) {
                emit preparedSplineChart({TITLE,
                                          QStringList{"Inflow", "Outflow"},
                                          BDataTable{inflowDataList, outflowDataList}}, false);
            }
            ++cur;
        }

        emit preparedSplineChart({TITLE,
                                  QStringList{"Inflow", "Outflow"},
                                  BDataTable{inflowDataList, outflowDataList}}, true);

        emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
                              3000);
    });

//        QList<QPair<QDateTime, QPair<int, int>>> results;
//        for (const auto &dateTime:dateTimesToDo) {
//            results.push_back(worker(dateTime));
//        }
//
//        for (const auto &pair:results) {
//            inflowDataList.push_back(
//                    {QPointF{static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
//                             static_cast<qreal>(pair.second.first)}, ""});
//            outflowDataList.push_back(
//                    {QPointF{static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
//                             static_cast<qreal>(pair.second.second)}, ""});
//        }
//
//        emit preparedSplineChart({QString("Results for station ") + stationId,
//                                  QStringList{"Inflow", "Outflow"},
//                                  BDataTable{inflowDataList, outflowDataList}});
//
//
//        emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
//                              3000);
//    });

    thread->start();
    connect(thread, &QThread::started, this, &PlotWidget::onAnalysisStarted);
    connect(thread, &QThread::finished, this, &PlotWidget::onAnalysisFinished);
}

void PlotWidget::dynamicAnalyze() {
    auto thread = QThread::create([this]() {
        auto startTime = QTime::currentTime();

        auto date = ui->dateEdit->date();
        auto timeStepTenMinutes = ui->timeStepBox->value();
        auto stationId = ui->stationBox->text();
        auto _startingTime = ui->startingTimeEdit->time();
        auto _endingTime = ui->endingTimeEdit->time();
        QDateTime startingDateTime(date, QTime(_startingTime.hour(), _startingTime.minute() / 10 * 10));
        QDateTime endingDateTime(date, QTime(_endingTime.hour(), _endingTime.minute() / 10 * 10));

        constexpr int _LENGTH = 15;
        const QString TITLE = QString("Traffic inflow and outflow trend of Station %1 from %2 to %3")
                .arg(stationId)
                .arg(startingDateTime.toString(DATE_TIME_FORMAT_NO_SEC))
                .arg(endingDateTime.toString(DATE_TIME_FORMAT_NO_SEC));

        QVector<QDateTime> dateTimesToDo;
        for (auto curDateTime = startingDateTime;
             curDateTime <= endingDateTime; curDateTime = curDateTime.addSecs(10 * 60)) {
            dateTimesToDo.push_back(curDateTime);
        }

        auto worker = [this, stationId, TITLE, _LENGTH](
                const QDateTime &curDateTime) -> QPair<QDateTime, QPair<int, int>> {
            auto threadDb = QSqlDatabase::addDatabase(
                    "QSQLITE",
                    QString("plot_thread_") + QString::number(quintptr(QThread::currentThreadId())));
            threadDb.setDatabaseName("file::memory:");
            threadDb.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
            threadDb.open();
            QSqlQuery query(threadDb);

            const auto queryStr = QString("SELECT\n"
                                          "\tcount( * ) \n"
                                          "FROM\n"
                                          "\tbz \n"
                                          "WHERE\n"
                                          "\ttime LIKE ( '%1%' ) \n"
                                          "\tAND status = %2 \n"
                                          "\tAND stationID = ") + stationId;

            auto curTimePrefixStr = curDateTime.toString(DATE_TIME_FORMAT).mid(0, _LENGTH);
//            qInfo().noquote() << queryStr.arg(curTimePrefixStr).arg(1);
            auto curQueryStr = queryStr.arg(curTimePrefixStr);

            if (!query.exec(curQueryStr.arg(1)))
                qInfo() << query.lastError().text();
            query.next();
            auto inflow = query.value(0).toInt();

            if (!query.exec(curQueryStr.arg(0)))
                qInfo() << query.lastError().text();
            query.next();
            auto outflow = query.value(0).toInt();

            threadDb.close();
            return {curDateTime, QPair{inflow, outflow}};
        };

        emit preparedChart({TITLE,
                            QStringList{"Inflow", "Outflow"},
                            BDataTable{{},
                                       {}}},
                           startingDateTime,
                           endingDateTime);

        BData tmpInflowData;
        BData tmpOutflowData;

        QList<QPair<QDateTime, QPair<int, int>>> results;

        int cur = 0;
        for (const auto &dateTime:dateTimesToDo) {
            auto pair = worker(dateTime);

            if (cur % timeStepTenMinutes == 0) {
                tmpInflowData = {QPointF{static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
                                         static_cast<qreal>(0.0)}, ""};
                tmpOutflowData = {QPointF{static_cast<qreal>(pair.first.toMSecsSinceEpoch()),
                                          static_cast<qreal>(0.0)}, ""};
            }

            tmpInflowData.first.ry() += static_cast<qreal>(pair.second.first);
            tmpOutflowData.first.ry() += static_cast<qreal>(pair.second.second);

            if (cur % timeStepTenMinutes == timeStepTenMinutes - 1) {
                emit newData(BDataList{tmpInflowData, tmpOutflowData});
            }
            ++cur;
        }

        emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
                              3000);
    });

    thread->start();
    connect(thread, &QThread::started, this, &PlotWidget::onAnalysisStarted);
    connect(thread, &QThread::finished, this, &PlotWidget::onAnalysisFinished);
}

void PlotWidget::dynamicInitChart(const PlotWidget::BzChartData &chartBaseData,
                                  const QDateTime &dt0, const QDateTime &dt1) {
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

        seriesList.push_back(series);
    }

    axisX->setMin(dt0);
    axisX->setMax(dt1);

    axisY->setMin(0.0);
    axisY->setMax(static_cast<int>( maxY * 11 / 10 + 1));

    chartView->setChart(chart);
    delete oldChart;
}

void PlotWidget::dynamicAppendData(const BDataList &dataList) {
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

void PlotWidget::onAnalysisStarted() {
    emit statusBarMessage("Analyzing...", 0);
    ui->analyzeButton->setEnabled(false);
}

void PlotWidget::onAnalysisFinished() {
    ui->analyzeButton->setEnabled(true);
}

void PlotWidget::setDate(const QString &pureDateStr) {
    ui->dateEdit->setDate(QDate::fromString(pureDateStr, DATE_FORMAT));
}

[[deprecated]] void PlotWidget::bzClear() {
//    chartView->close();
}

[[deprecated]] void PlotWidget::setDynamicDateTimeSplineChart(const PlotWidget::BzChartData &newChartData, bool) {
    if (newChartData.dataTable.isEmpty()) {
        analyzing = false;
        maxY = INT_MIN;
        return;
    } else if (!analyzing) {
        analyzing = true;
        setDateTimeSplineChart(newChartData);
    } else {
        auto chart = chartView->chart();
        auto point0 = newChartData.dataTable[0][0].first;
        auto point1 = newChartData.dataTable[1][0].first;
        dynamic_cast<QSplineSeries *>(chart->series()[0])->append(point0);
        dynamic_cast<QSplineSeries *>(chart->series()[1])->append(point1);
        maxY = std::max({maxY, point0.y(), point1.y()});
        dynamic_cast<QValueAxis *>(chart->axes()[1])->setMax(maxY * 11 / 10);
    }


}

