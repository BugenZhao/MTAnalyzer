#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QThread>
#include <QSqlError>
#include "querywidget.h"
#include "ui_querywidget.h"

QueryWidget::QueryWidget(QSqlDatabase *pDb, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::QueryWidget),
        pDb(pDb),
        model(new QSqlQueryModel(this)) {
    ui->setupUi(this);

    ui->resultTable->setModel(model);

    connect(ui->queryButton, &QPushButton::clicked, this, &QueryWidget::doQuery);
}

QueryWidget::~QueryWidget() {
    delete ui;
}

void QueryWidget::doQuery() {
    ui->queryButton->setEnabled(false);

    model->setQuery(ui->queryInput->toPlainText(), *pDb);
    model->query();
    while (model->canFetchMore()) model->fetchMore();

    if (model->lastError().type() == QSqlError::NoError)
            emit statusBarMessage("Done", 3000);
    else
            emit statusBarMessage(QString("Query error: %1").
                arg(model->lastError().databaseText()), 8000);

    ui->resultTable->resizeColumnsToContents();
    ui->queryButton->setEnabled(true);
}

void QueryWidget::setBzEnabled(bool enabled) {
    if (!enabled) model->clear();
    ui->queryButton->setEnabled(enabled);
}
