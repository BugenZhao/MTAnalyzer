#ifndef QUERYWIDGET_H
#define QUERYWIDGET_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

namespace Ui {
    class QueryWidget;
}

class QueryWidget : public QWidget {
Q_OBJECT

public:
    explicit QueryWidget(QWidget *parent = nullptr);

    ~QueryWidget();

    void setBzEnabled(bool enabled);

signals:

    void statusBarMessage(const QString &message, int timeout = 0);

private:
    Ui::QueryWidget *ui;
    QSqlQueryModel *model;

    void doQuery();

    void setQueryText(const QString &text);

    void onQueryStarted();

    void onQueryFinished();
};

#endif // QUERYWIDGET_H
