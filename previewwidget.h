#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

namespace Ui {
    class PreviewWidget;
}

class PreviewWidget : public QWidget {
Q_OBJECT

public:
    explicit PreviewWidget(QWidget *parent = nullptr);

    ~PreviewWidget();

    void setBzEnabled(bool enabled);

    void updateBz(const QString &sqlText);

    void release();

signals:

    void statusBarMessage(const QString &message, int timeout = 0);

    void count(int _count);

private:
    Ui::PreviewWidget *ui;
    QSqlQueryModel *model;

    void onQueryStarted();

    void onQueryFinished();

    void newTitle(int _count);
};

#endif // PREVIEWWIDGET_H
