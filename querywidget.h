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

    void success();

    void failed();

private:
    Ui::QueryWidget *ui;
    QSqlQueryModel *model;

    QMap<QString, QString> exampleQueries;

    void doQuery();

    void setQueryText(const QString &name);

    void onQueryStarted();

    void onQueryFinished();

    void onSuccess();

    void onFailed();

    void onInputModified();
};

#endif // QUERYWIDGET_H
