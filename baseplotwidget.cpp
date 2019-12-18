#include "baseplotwidget.h"

BasePlotWidget::BasePlotWidget(QWidget *parent) : QWidget(parent) {
    qRegisterMetaType<BzChartData>("BzChartData");
    qRegisterMetaType<BDataList>("BDataList");
}

void BasePlotWidget::setSpeed(int _speedLevel) {
    this->speedLevel = _speedLevel;
}

int BasePlotWidget::expectedTimeMs() {
    switch (speedLevel) {
        case BugenZhao::FAST:
            return 1500;
        case BugenZhao::FASTER:
            return 800;
        case BugenZhao::FASTEST:
            return 0;
        default:
            return 1500;
    }
}
