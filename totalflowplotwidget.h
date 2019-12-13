#ifndef TOTALFLOWPLOTWIDGET_H
#define TOTALFLOWPLOTWIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include "ui_totalflowplotwidget.h"
#include "utilities/base.hpp"
#include "baseplotwidget.h"

using namespace QtCharts;
using namespace BugenZhao;

namespace Ui {
    class TotalFlowPlotWidget;
}

class TotalFlowPlotWidget : public BasePlotWidget {
Q_OBJECT

public:

    explicit TotalFlowPlotWidget(QWidget *parent = nullptr);

    ~TotalFlowPlotWidget() override;

    void setBzEnabled(bool enabled) override;

    void onAnalysisStarted() override;

    void onAnalysisFinished() override;

    void setDate(const QString &pureDateStr) override;


public slots:

    void setFilterDataList(FilterDataList _filterDataList) override;

signals:

    void preparedChart(const BzChartData &, const QDateTime &dt0, const QDateTime &dt1);

    void newData(const BDataList &dataList);

private:
    Ui::TotalFlowPlotWidget *ui;

protected:
    QChartView *chartView = nullptr;
    qreal maxY = 0.0;
    QList<QSplineSeries *> seriesList{};
    QAbstractAxis *_axisX = nullptr;
    QAbstractAxis *_axisY = nullptr;
    FilterDataList filterDataList{};


    void dynamicInitChart(const BzChartData &chartBaseData, const QDateTime &dt0, const QDateTime &dt1);

    void dynamicAppendData(const BDataList &dataList);

    void dynamicAnalyzeBetter();
};

#endif // TOTALFLOWPLOTWIDGET_H
