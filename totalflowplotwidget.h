#ifndef TOTALFLOWPLOTWIDGET_H
#define TOTALFLOWPLOTWIDGET_H

#include <QWidget>

namespace Ui {
class TotalFlowPlotWidget;
}

class TotalFlowPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TotalFlowPlotWidget(QWidget *parent = nullptr);
    ~TotalFlowPlotWidget();

private:
    Ui::TotalFlowPlotWidget *ui;
};

#endif // TOTALFLOWPLOTWIDGET_H
