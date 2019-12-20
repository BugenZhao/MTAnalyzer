#ifndef STATIONFLOWPLOTWIDGET_H
#define STATIONFLOWPLOTWIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include "ui_stationflowplotwidget.h"
#include "utilities/base.hpp"
#include "baseplotwidget.h"

using namespace QtCharts;
using namespace BugenZhao;

namespace Ui {
    class StationFlowPlotWidget;
}

class StationFlowPlotWidget : public BasePlotWidget {
Q_OBJECT

public:
    using PieData = QPair<int, QPair<QVector<QPair<QString, int>>, QVector<QPair<QString, int>>>>;

    explicit StationFlowPlotWidget(QWidget *parent = nullptr);

    ~StationFlowPlotWidget() override;

    void setBzEnabled(bool enabled) override;

    void onAnalysisStarted() override;

    void onAnalysisFinished() override;

    void setDate(const QString &pureDateStr) override;


public slots:

    void setFilterDataList(FilterDataList _filterDataList) override;

signals:

    void preparedPieChart(QString title, PieData flows);

    void newData(const BDataList &dataList);

private:
    Ui::StationFlowPlotWidget *ui;

protected:
    QChartView *chartViewLeft = nullptr;
    QChartView *chartViewRight = nullptr;
    qreal maxY = 0.0;
    QList<QSplineSeries *> seriesList{};
    QAbstractAxis *_axisX = nullptr;
    QAbstractAxis *_axisY = nullptr;
    FilterDataList filterDataList{};
    QMap<int, QPair<QChart *, QChart *>> chartMap;

    int startingTimestamp;
    int endingTimestamp;
    int stepTimestamp;

    void chart(QString title, PieData flows);

    void analyze();

    void init();

    void onCurrentTimeUpdated(int index);

    int expectedTimeMs() override;
};

#endif // STATIONFLOWPLOTWIDGET_H
