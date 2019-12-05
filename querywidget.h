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
    explicit QueryWidget(QSqlDatabase *pDb, QWidget *parent = nullptr);

    ~QueryWidget();

    void setBzEnabled(bool enabled);

private:
    Ui::QueryWidget *ui;
    QSqlDatabase *pDb;
    QSqlQueryModel *model;

    void doQuery();
};

#endif // QUERYWIDGET_H
