#include "baseplotwidget.h"

BasePlotWidget::BasePlotWidget(QWidget *parent) : QWidget(parent) {
    qRegisterMetaType<BzChartData>("BzChartData");
    qRegisterMetaType<BDataList>("BDataList");
}
