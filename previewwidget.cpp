#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QThread>
#include <QSqlError>
#include <QStringListModel>
#include "previewwidget.h"
#include "ui_previewwidget.h"
#include "utilities/bdatabasemanager.h"

PreviewWidget::PreviewWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::PreviewWidget),
        model(new QSqlQueryModel(this)) {
    ui->setupUi(this);
    ui->resultTable->setModel(model);
    connect(this, &PreviewWidget::count, this, &PreviewWidget::newTitle);
}

PreviewWidget::~PreviewWidget() {
    delete ui;
}

void PreviewWidget::onQueryFinished() {
    ui->resultTable->resizeColumnsToContents();
//    ui->resultTable->setEnabled(true);
}

void PreviewWidget::onQueryStarted() {
//    ui->resultTable->setEnabled(false);
}

void PreviewWidget::setBzEnabled(bool enabled) {
}

void PreviewWidget::release() {
    model->clear();
    model = new QSqlQueryModel(this);
    ui->resultTable->setModel(model);
}

void PreviewWidget::updateBz(const QString &sqlText) {
    const auto &queryText = sqlText;
    model->removeRows(0, model->rowCount());


    auto thread = QThread::create([this, queryText]() {
        auto db = BDatabaseManager::readOnlyConnection("preview_thread");
        QSqlQuery countQuery(db);
        countQuery.exec("SELECT count(*) FROM bz");
        countQuery.next();
        emit count(countQuery.value(0).toInt());
        model->setQuery(queryText, db);
        model->query();
//        while (model->canFetchMore()) model->fetchMore();

        if (model->lastError().type() == QSqlError::NoError) {
            emit statusBarMessage("Done", 3000);
        } else {
            emit statusBarMessage(QString("Query error: %1").
                    arg(model->lastError().databaseText()), 8000);
        }
    });

    connect(thread, &QThread::started, this, &PreviewWidget::onQueryStarted);
    connect(thread, &QThread::finished, this, &PreviewWidget::onQueryFinished);
    thread->start();
}


void PreviewWidget::newTitle(int _count) {
    ui->groupBox_2->setTitle(QString("All %1 Records").arg(_count));
}