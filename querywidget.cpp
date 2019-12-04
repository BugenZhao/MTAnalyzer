#include <QSqlQueryModel>
#include <QSqlQuery>
#include "querywidget.h"
#include "ui_querywidget.h"

QueryWidget::QueryWidget(QSqlDatabase *pDb, QWidget *parent) :
        pDb(pDb),
        QWidget(parent),
        ui(new Ui::QueryWidget) {
    ui->setupUi(this);

    connect(ui->queryButton, &QPushButton::clicked, this, &QueryWidget::doQuery);
}

QueryWidget::~QueryWidget() {
    delete ui;
}

void QueryWidget::doQuery() {
    auto model = new QSqlQueryModel(this);
    ui->resultTable->setModel(model);
    model->setQuery(ui->queryInput->toPlainText(), *pDb);
    model->query();
    while (model->canFetchMore()) model->fetchMore();
}

void QueryWidget::setBzEnabled(bool enabled) {
    ui->queryButton->setEnabled(enabled);
}
