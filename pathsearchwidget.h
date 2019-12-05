#ifndef PATHSEARCHWIDGET_H
#define PATHSEARCHWIDGET_H

#include <QWidget>
#include "utilities/base.hpp"

namespace Ui {
    class PathSearchWidget;
}

class PathSearchWidget : public QWidget {
Q_OBJECT

public:
    explicit PathSearchWidget(Adj *adj, QWidget *parent = nullptr);

    ~PathSearchWidget();

    void setBzEnabled(bool enabled);

private:
    Ui::PathSearchWidget *ui;
    Adj *adj;

    QString bfs(int from, int to, bool *ok);

    void doSearch();

private slots:

    void onSearchStarted();

    void onSearchFinished();
};

#endif // PATHSEARCHWIDGET_H
