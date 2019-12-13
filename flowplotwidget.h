#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include "ui_flowplotwidget.h"
#include "utilities/base.hpp"
#include "baseplotwidget.h"

using namespace QtCharts;
using namespace BugenZhao;

namespace Ui {
    class FlowPlotWidget;
}

class FlowPlotWidget : public BasePlotWidget {
Q_OBJECT


public:

    explicit FlowPlotWidget(QWidget *parent = nullptr);

    ~FlowPlotWidget();

    void setBzEnabled(bool enabled) override;

    [[deprecated]] void setDateTimeSplineChart(const BzChartData &chartData, bool animated = true);

    [[deprecated]] void setDynamicDateTimeSplineChart(const BzChartData &newChartData, bool);

    void onAnalysisStarted() override;

    void onAnalysisFinished() override;

    void setDate(const QString &pureDateStr) override;

    [[deprecated]] void bzClear();

public slots:

    void setFilterDataList(FilterDataList _filterDataList) override;

signals:

//    void statusBarMessage(QString qString, int i);

    void preparedSplineChart(const BzChartData &, bool animated = true);

    void preparedChart(const BzChartData &, const QDateTime &dt0, const QDateTime &dt1);

    void newData(const BDataList &dataList);

private:
    Ui::FlowPlotWidget *ui;

protected:
    QChartView *chartView = nullptr;
    bool analyzing = false;
    qreal maxY = 0.0;
    QList<QSplineSeries *> seriesList{};
    QAbstractAxis *_axisX = nullptr;
    QAbstractAxis *_axisY = nullptr;
    FilterDataList filterDataList{};


    [[deprecated]] void analyze();

    [[deprecated]] void dynamicAnalyzeOldTenMinutes();

    void dynamicInitChart(const BzChartData &chartBaseData, const QDateTime &dt0, const QDateTime &dt1);

    void dynamicAppendData(const BDataList &dataList);

    void dynamicAnalyzeBetter();
};

#endif // PLOTWIDGET_H
