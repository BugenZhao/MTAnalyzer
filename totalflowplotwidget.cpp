#include "totalflowplotwidget.h"
#include "ui_totalflowplotwidget.h"

TotalFlowPlotWidget::TotalFlowPlotWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TotalFlowPlotWidget)
{
    ui->setupUi(this);
}

TotalFlowPlotWidget::~TotalFlowPlotWidget()
{
    delete ui;
}
