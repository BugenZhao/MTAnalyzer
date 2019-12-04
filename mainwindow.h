#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QTreeWidgetItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    using Record=QString;

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void importDataSet();

private slots:
    void on_tabs_currentChanged(int index);

private:
    Ui::MainWindow *ui;
    QDir mainDir = QDir();
    QDir dataSetDir = QDir();
    QDir adjacencyDir = QDir();

    QTreeWidgetItem *dataSetItem;

    QVector2D *adj = nullptr;

    QSqlDatabase db;
    QHash<int, QVector<Record>> data;
    QMap<QString, int> fileId;

    const QString timeFormat = "yyyy-MM-dd hh:mm:ss";

    void setupUi();

    void updateFilterWidgetAdjacency();

    void updateFilterWidgetDataSet();

    void updateFilterWidgetOthers(const QStringList &lines, const QStringList &payTypes);

    void test();
};

#endif // MAINWINDOW_H
