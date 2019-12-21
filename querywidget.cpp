#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QThread>
#include <QSqlError>
#include <QStringListModel>
#include <QException>
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
                                  << "Total Count => SELECT count(*) FROM bz"
                                  << "Inspect Table => SELECT * FROM bz LIMIT 1000"
                                  << "Flow of Station => SELECT stationID, count(*) flow FROM bz GROUP BY stationID ORDER BY flow DESC";
    auto examplesModel = new QStringListModel(examples);
    ui->examplesComboBox->setModel(examplesModel);
    connect(ui->examplesComboBox, &QComboBox::currentTextChanged,
            this, &QueryWidget::setQueryText);

    ui->queryInput->setPlaceholderText(
            "AnyExplore supports any standard SQL statements to explore the entire database. "
            "Default table name is 'bz'.\n"
            "Please see the examples on the right.");

    ui->resultTable->setModel(model);

    connect(ui->queryButton, &QPushButton::clicked, this, &QueryWidget::doQuery);
}

QueryWidget::~QueryWidget() {
    delete ui;
}

void QueryWidget::doQuery() {
    auto queryText = ui->queryInput->toPlainText();

    if (!queryText.toUpper().startsWith("SELECT")) {
        emit statusBarMessage("Sorry, the database is select-only", 3000);
        return;
    }

    auto thread = QThread::create([this, queryText]() {
        auto db = BDatabaseManager::readOnlyConnection("query_thread");
        model->removeRows(0, model->rowCount());
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
    ui->resultTable->setEnabled(true);
}

void QueryWidget::onQueryStarted() {
    emit statusBarMessage("Querying...");
    ui->queryButton->setEnabled(false);
    ui->resultTable->setEnabled(false);
}

void QueryWidget::setBzEnabled(bool enabled) {
    ui->queryButton->setEnabled(enabled);
}

void QueryWidget::setQueryText(const QString &text) {
    if (text.startsWith("Example")) return;
    try {
        ui->queryInput->setPlainText(text.split(" => ")[1]);
        ui->examplesComboBox->setCurrentIndex(0);
    } catch (QException &) {}
}