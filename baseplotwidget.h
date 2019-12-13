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

    virtual void setDate(const QString &pureDateStr) = 0;

signals:

public slots:
};

#endif // BASEPLOTWIDGET_H
