#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QThread>
#include "querywidget.h"
#include "ui_querywidget.h"

QueryWidget::QueryWidget(QSqlDatabase *pDb, QWidget *parent) :
        pDb(pDb),
        QWidget(parent),
        ui(new Ui::QueryWidget),
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

    ui->resultTable->resizeColumnsToContents();
    ui->queryButton->setEnabled(true);
}

void QueryWidget::setBzEnabled(bool enabled) {
    ui->queryButton->setEnabled(enabled);
}
