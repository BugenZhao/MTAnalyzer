#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::PreferencesDialog) {
    ui->setupUi(this);
}

PreferencesDialog::~PreferencesDialog() {
    delete ui;
}

void PreferencesDialog::accept() {
    data = {ui->adjEdit->text(),
            ui->dataSetEdit->text(),
            ui->speedSlider->value()};
    QDialog::accept();
}

void PreferencesDialog::reject() {
    QDialog::reject();
}

PreferencesDialog::Preferences
PreferencesDialog::getPreferences(const PreferencesDialog::Preferences &current, bool enabled) {
    PreferencesDialog dialog;
    dialog.ui->subDirGroupBox->setEnabled(enabled);
    dialog.ui->adjEdit->setText(current.adjacencySubdirName);
    dialog.ui->dataSetEdit->setText(current.dataSetSubdirName);
    dialog.ui->speedSlider->setValue(current.speedLevel);
    dialog.data = current;
    dialog.exec();
    return dialog.data;
}
