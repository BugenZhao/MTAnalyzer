#include <QStandardItemModel>
#include <QQueue>
#include <QThread>
#include <QSqlQuery>
#include <QSqlResult>
#include <QDateTime>
#include <QtCore>
#include "pathsearchwidget.h"
#include "ui_pathsearchwidget.h"
#include "utilities/base.hpp"
#include "utilities/bdatabasemanager.h"


PathSearchWidget::PathSearchWidget(Adj *adj, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::PathSearchWidget),
        adj(adj) {
    ui->setupUi(this);

    connect(ui->searchButton, &QPushButton::clicked, this, &PathSearchWidget::doSearch);

    auto model = new QStandardItemModel(0, 1, this);
    ui->resultTable->setModel(model);
    ui->resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    model->clear();
    model->setHorizontalHeaderLabels(QStringList() << "Results");
}

PathSearchWidget::~PathSearchWidget() {
    delete ui;
}

void PathSearchWidget::doSearch() {
    auto model = reinterpret_cast<QStandardItemModel *>(ui->resultTable->model());
    model->clear();

    bool ok1 = true;
    bool ok2 = true;
    auto from = ui->fromBox->value();
    auto to = ui->toBox->value();

    if (ok1 & ok2) {
        bool ok3 = false;
        auto path = bfs(from, to, &ok3);
        model->appendRow(new QStandardItem(path));
        model->setVerticalHeaderLabels(QStringList() << "Path");
        model->setHorizontalHeaderLabels(QStringList() << "Results");
        ui->resultTable->resizeRowsToContents();
        ui->resultTable->setWordWrap(true);

        if (ok3) {
            auto thread = QThread::create([this, from, to, path]() {
                auto startTime = QTime::currentTime();

                auto model = reinterpret_cast<QStandardItemModel *>(ui->resultTable->model());

                QString timeInfo = "There's no enough information to get the result. \n"
                                   "Make sure you have imported the 'User ID' field, "
                                   "and try to import more data sets.";
                QVector<QString> details;

                if (from != to) {
                    auto threadDb = BDatabaseManager::connection("path_thread");
                    QSqlQuery query(threadDb);

                    auto timeOk = query.exec(QString("SELECT\n"
                                                     "\tbz.userID,\n"
                                                     "\ttmp.stationID,\n"
                                                     "\ttmp.time,\n"
                                                     "\tbz.stationID,\n"
                                                     "\tbz.time \n"
                                                     "FROM\n"
                                                     "\t(\n"
                                                     "\tSELECT\n"
                                                     "\t\tuserID,\n"
                                                     "\t\tstationID,\n"
                                                     "\t\ttime FROM bz \n"
                                                     "\tWHERE\n"
                                                     "\t\t(\n"
                                                     "\t\t\tstationID = %1 \n"
                                                     "\t\t\tAND status = 1 \n"
                                                     "\t\t\tAND payType <> 3 \n"
                                                     "\t\t) \n"
                                                     "\t\tLIMIT 20000 \n"
                                                     "\t) tmp\n"
                                                     "\tINNER JOIN bz ON tmp.userID = bz.userID \n"
                                                     "WHERE\n"
                                                     "\tbz.stationID = %2 \n"
                                                     "\tAND bz.status = 0 \n"
                                                     "GROUP BY\n"
                                                     "\tbz.userID \n"
                                                     "\tLIMIT 500").arg(from).arg(to));


                    if (timeOk) {
                        auto pathLength = path.split("-").size() - 1;

                        QVector<qlonglong> minDts;
                        while (query.next()) {
                            auto sTime0 = query.value(2).toString();
                            auto sTime1 = query.value(4).toString();
                            if (sTime0.split(' ')[0] != sTime1.split(' ')[0]) continue;

                            auto time0 = QDateTime::fromString(sTime0, BugenZhao::DATE_TIME_FORMAT);
                            auto time1 = QDateTime::fromString(sTime1, BugenZhao::DATE_TIME_FORMAT);
                            auto minDt = time0.secsTo(time1) / 60;

                            if (minDt > pathLength * 2 && minDt <= 6 + pathLength * 4 && minDt <= 240) {
                                minDts.push_back(minDt);
                                details.push_back(QString("%1 - %2").arg(sTime0).arg(sTime1));
                                qInfo() << minDt;
                            }

                        }
                        if (!minDts.isEmpty()) {
                            timeInfo = QString("About %1 minutes according to %2 record(s).")
                                    .arg(BugenZhao::bAverage(minDts)).arg(minDts.size());
                        }
                    }
                    threadDb.close();

                } else {
                    timeInfo = "You can stay here as long as you want :)";
                }

                model->appendRow(new QStandardItem(timeInfo));
                model->setVerticalHeaderLabels(QStringList() << "Path" << "Estimated Time");

                std::sort(details.begin(), details.end());
                for (const auto &detail:details) model->appendRow(new QStandardItem(detail));
                if (!details.isEmpty()) {
                    auto labels = QStringList() << "Path" << "Estimated Time" << "Details";
                    for (int i = 2; i <= details.size(); ++i) {
                        labels << QString::number(i);
                    }
                    model->setVerticalHeaderLabels(labels);
                }


                model->setHorizontalHeaderLabels(QStringList() << "Results");



                emit statusBarMessage(QString("Done in %1s").arg(startTime.msecsTo(QTime::currentTime()) / 1000.0),
                                      3000);
            });

            thread->start();
            connect(thread, &QThread::started, this, &PathSearchWidget::onSearchStarted);
            connect(thread, &QThread::finished, this, &PathSearchWidget::onSearchFinished);
        }
    }

}

void PathSearchWidget::setBzEnabled(bool enabled) {
    ui->searchButton->setEnabled(enabled);
}

QString PathSearchWidget::bfs(int from, int to, bool *ok) {
    auto N = adj->size();

    if (from < 0 || from >= N || to < 0 || to >= N) return "No such station(s).";

    QVector<bool> visited(N, false);
    QVector<int> previous(N, -1);
    QQueue<int> queue;

    queue.enqueue(from);
    visited[from] = true;

    while (!queue.empty()) {
        auto v = queue.dequeue();
        for (int s = 0; s < N; ++s) {
            if ((*adj)[v][s] && !visited[s]) {
                visited[s] = true;
                previous[s] = v;
                if (s == to) break;
                queue.enqueue(s);
            }
        }
    }

    if (!visited[to]) return "No such path.";

    QStringList path;
    int v = to;
    do {
        path.push_front(QString::number(v));
        v = previous[v];
    } while (v != -1);

    *ok = true;
    return path.join(" -> ");
}

void PathSearchWidget::onSearchStarted() {
    emit statusBarMessage("Estimating time...", 0);
    ui->searchButton->setEnabled(false);
}

void PathSearchWidget::onSearchFinished() {
    ui->searchButton->setEnabled(true);
    ui->resultTable->resizeRowsToContents();
}
