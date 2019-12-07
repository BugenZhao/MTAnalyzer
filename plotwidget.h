#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include "utilities/base.hpp"

using namespace QtCharts;
using namespace BugenZhao;

namespace Ui {
    class PlotWidget;
}

class PlotWidget : public QWidget {
Q_OBJECT


public:
    struct BzChartData {
        QString title;
        QStringList seriesNames;
        BDataTable dataTable;
    };

    explicit PlotWidget(QWidget *parent = nullptr);

    ~PlotWidget();

    void setBzEnabled(bool enabled);

    void setDateTimeSplineChart(const BzChartData &chartData, bool animated = true);

    [[deprecated]] void setDynamicDateTimeSplineChart(const BzChartData &newChartData, bool);

    void onAnalysisStarted();

    void onAnalysisFinished();

    void setDate(const QString &pureDateStr);

    [[deprecated]] void bzClear();

signals:

    void statusBarMessage(QString qString, int i);

    void preparedSplineChart(const BzChartData &, bool animated = true);

    void preparedChart(const BzChartData &, const QDateTime &dt0, const QDateTime &dt1);

    void newData(const BDataList &dataList);

private:
    Ui::PlotWidget *ui;
    QChartView *chartView = nullptr;
    bool analyzing = false;
    qreal maxY = 0.0;
    QList<QSplineSeries *> seriesList{};
    QAbstractAxis *_axisX = nullptr;
    QAbstractAxis *_axisY = nullptr;


    [[deprecated]] void analyze();

    [[deprecated]] void dynamicAnalyzeOldTenMinutes();

    void dynamicInitChart(const BzChartData &chartBaseData, const QDateTime &dt0, const QDateTime &dt1);

    void dynamicAppendData(const BDataList &dataList);

    void dynamicAnalyzeBetter();
};

#endif // PLOTWIDGET_H
