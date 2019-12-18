#ifndef BASEPLOTWIDGET_H
#define BASEPLOTWIDGET_H

#include <QWidget>
#include "utilities/base.hpp"

class BasePlotWidget : public QWidget {
Q_OBJECT
public:
    struct BzChartData {
        QString title;
        QStringList seriesNames;
        BDataTable dataTable;
    };

    explicit BasePlotWidget(QWidget *parent = nullptr);

    virtual void setBzEnabled(bool enabled) = 0;

    virtual void setSpeed(int _speedLevel);

    virtual void onAnalysisStarted() = 0;

    virtual void onAnalysisFinished() = 0;

    virtual void setDate(const QString &pureDateStr) = 0;

signals:

    void statusBarMessage(QString qString, int i);

public slots:

    virtual void setFilterDataList(FilterDataList list) = 0;

protected:
    int speedLevel;

    virtual int expectedTimeMs();
};

#endif // BASEPLOTWIDGET_H
