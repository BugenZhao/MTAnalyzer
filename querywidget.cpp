#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QThread>
#include <QSqlError>
#include <QStringListModel>
#include "querywidget.h"
#include "ui_querywidget.h"
#include "utilities/bdatabasemanager.h"

QueryWidget::QueryWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::QueryWidget),
        model(new QSqlQueryModel(this)) {
    ui->setupUi(this);
    ui->examplesComboBox->setCurrentText("Examples...");
    auto examples = QStringList() << "Examples"
                                  << "SELECT count(*) FROM bz"
                                  << "SELECT * FROM bz LIMIT 1000"
                                  << "SELECT stationID, count(*) flow FROM bz GROUP BY stationID ORDER BY flow DESC";
    auto examplesModel = new QStringListModel(examples);
    ui->examplesComboBox->setModel(examplesModel);
    connect(ui->examplesComboBox, &QComboBox::currentTextChanged,
            this, &QueryWidget::setQueryText);

    ui->resultTable->setModel(model);

    connect(ui->queryButton, &QPushButton::clicked, this, &QueryWidget::doQuery);
}

QueryWidget::~QueryWidget() {
    delete ui;
}

void QueryWidget::doQuery() {
    auto queryText = ui->queryInput->toPlainText();

    if (!queryText.toUpper().startsWith("SELECT")) {
        emit statusBarMessage("Sorry, the database is read-only", 3000);
        return;
    }

    auto thread = QThread::create([this, queryText]() {
        auto db = BDatabaseManager::readOnlyConnection("query_thread");
        model->setQuery(queryText, db);
        model->query();
        while (model->canFetchMore()) model->fetchMore();

        if (model->lastError().type() == QSqlError::NoError)
                emit statusBarMessage("Done", 3000);
        else
                emit statusBarMessage(QString("Query error: %1").
                    arg(model->lastError().databaseText()), 8000);
    });

    connect(thread, &QThread::started, this, &QueryWidget::onQueryStarted);
    connect(thread, &QThread::finished, this, &QueryWidget::onQueryFinished);
    thread->start();
}

void QueryWidget::onQueryFinished() {
    ui->resultTable->resizeColumnsToContents();
    ui->queryButton->setEnabled(true);
}

void QueryWidget::onQueryStarted() {
    emit statusBarMessage("Querying...");
    ui->queryButton->setEnabled(false);
}

void QueryWidget::setBzEnabled(bool enabled) {
    if (!enabled) model->clear();
    ui->queryButton->setEnabled(enabled);
}

void QueryWidget::setQueryText(const QString &text) {
    if (text.startsWith("Example")) return;
    ui->queryInput->setPlainText(text);
    ui->examplesComboBox->setCurrentIndex(0);
}