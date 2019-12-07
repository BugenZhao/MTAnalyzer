#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
Q_OBJECT

public:
    struct Preferences {
        QString adjacencySubdirName;
        QString dataSetSubdirName;
    };

    explicit PreferencesDialog(QWidget *parent = nullptr);

    ~PreferencesDialog();

    static PreferencesDialog::Preferences
    getPreferences(const PreferencesDialog::Preferences &current, bool enabled = true);

    void accept() override;

    void reject() override;

private:
    Ui::PreferencesDialog *ui;

    Preferences data{};
};

#endif // PREFERENCESDIALOG_H
