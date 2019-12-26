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

    exampleQueries.insert("1. Total Count", "SELECT count(*) FROM bz");
    exampleQueries.insert("2. Inspect Table", "SELECT * FROM bz LIMIT 1000");
    exampleQueries.insert("3. Flow of Station",
                          "SELECT stationID, count(*) flow FROM bz GROUP BY stationID ORDER BY flow DESC");
    exampleQueries.insert("4. 'WHERE' Example", "SELECT * FROM bz WHERE stationID = 15 AND payType = 0");

    auto exampleNames = QStringList() << "Examples";
    for (const auto &name:exampleQueries.keys()) {
        exampleNames << name;
    }
    auto examplesModel = new QStringListModel(exampleNames);
    ui->examplesComboBox->setModel(examplesModel);
    connect(ui->examplesComboBox, &QComboBox::currentTextChanged,
            this, &QueryWidget::setQueryText);

    ui->queryInput->setPlaceholderText(
            "AnyExplore supports any standard SQL statements to explore the entire database. "
            "Default table name is 'bz'.\n"
            "Please see the exampleNames on the right.");

    ui->resultTable->setModel(model);

    connect(ui->queryButton, &QPushButton::clicked, this, &QueryWidget::doQuery);
    connect(ui->queryInput, &QPlainTextEdit::textChanged, this, &QueryWidget::onInputModified);

    connect(this, &QueryWidget::success, this, &QueryWidget::onSuccess);
    connect(this, &QueryWidget::failed, this, &QueryWidget::onFailed);
}

QueryWidget::~QueryWidget() {
    delete ui;
}

void QueryWidget::doQuery() {
    auto queryText = ui->queryInput->toPlainText();

    if (!queryText.toUpper().startsWith("SELECT")) {
        emit statusBarMessage("Sorry, the database is select-only", 3000);
        emit failed();
        return;
    }

    auto thread = QThread::create([this, queryText]() {
        auto db = BDatabaseManager::readOnlyConnection("query_thread");
        model->removeRows(0, model->rowCount());
        model->setQuery(queryText, db);
        model->query();
        while (model->canFetchMore()) model->fetchMore();

        if (model->lastError().type() == QSqlError::NoError) {
            emit statusBarMessage("Done", 3000);
            emit success();
        } else {
            emit statusBarMessage(QString("Query error: %1").
                    arg(model->lastError().databaseText()), 8000);
            emit failed();
        }
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

void QueryWidget::setQueryText(const QString &name) {
    if (name.startsWith("Example")) return;
    try {
        ui->queryInput->setPlainText(exampleQueries[name]);
        ui->examplesComboBox->setCurrentIndex(0);
    } catch (QException &) {}
}

void QueryWidget::onSuccess() {
    auto palette = ui->queryInput->palette();
    palette.setColor(QPalette::Text, Qt::darkGreen);
    ui->queryInput->setPalette(palette);
}

void QueryWidget::onFailed() {
    auto palette = ui->queryInput->palette();
    palette.setColor(QPalette::Text, Qt::darkRed);
    ui->queryInput->setPalette(palette);
}

void QueryWidget::onInputModified() {
    auto palette = ui->queryInput->palette();
    if (palette.color(QPalette::Text) == Qt::black) return;
    palette.setColor(QPalette::Text, Qt::black);
    ui->queryInput->setPalette(palette);
}
