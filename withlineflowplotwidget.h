#ifndef WITHLINEFLOWPLOTWIDGET_H
#define WITHLINEFLOWPLOTWIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include "ui_withlineflowplotwidget.h"
#include "utilities/base.hpp"
#include "baseplotwidget.h"

using namespace QtCharts;
using namespace BugenZhao;

namespace Ui {
    class WithLineFlowPlotWidget;
}

class WithLineFlowPlotWidget : public BasePlotWidget {
Q_OBJECT


public:

    explicit WithLineFlowPlotWidget(QWidget *parent = nullptr);

    ~WithLineFlowPlotWidget() override;

    void setBzEnabled(bool enabled) override;

    void onAnalysisStarted() override;

    void onAnalysisFinished() override;

    void setDate(const QString &pureDateStr) override;


public slots:

    void setFilterDataList(FilterDataList _filterDataList) override;

signals:

//    void statusBarMessage(QString qString, int i);

    void preparedSplineChart(const BzChartData &, bool animated = true);

    void preparedChart(const BzChartData &, const QDateTime &dt0, const QDateTime &dt1);

    void newData(const BDataList &dataList);

private:
    Ui::WithLineFlowPlotWidget *ui;

protected:
    QChartView *chartView = nullptr;
    bool analyzing = false;
    qreal maxY = 0.0;
    QList<QSplineSeries *> seriesList{};
    QAbstractAxis *_axisX = nullptr;
    QAbstractAxis *_axisY = nullptr;
    FilterDataList filterDataList{};


    void dynamicInitChart(const BzChartData &chartBaseData, const QDateTime &dt0, const QDateTime &dt1);

    void dynamicAppendData(const BDataList &dataList);

    void dynamicAnalyzeBetter();
};

#endif // WITHLINEFLOWPLOTWIDGET_H
