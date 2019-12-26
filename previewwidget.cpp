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
}

PreviewWidget::~PreviewWidget() {
    delete ui;
}

void PreviewWidget::onQueryFinished() {
    ui->resultTable->resizeColumnsToContents();
    ui->resultTable->setEnabled(true);
}

void PreviewWidget::onQueryStarted() {
    ui->resultTable->setEnabled(false);
}

void PreviewWidget::setBzEnabled(bool enabled) {
}

void PreviewWidget::updateBz(const QString &sqlText) {
    const auto &queryText = sqlText;
    model->removeRows(0, model->rowCount());


    auto thread = QThread::create([this, queryText]() {
        auto db = BDatabaseManager::readOnlyConnection("preview_thread");
        model->setQuery(queryText, db);
        model->query();
        while (model->canFetchMore()) model->fetchMore();

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
