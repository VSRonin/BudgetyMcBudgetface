#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}
class MainObject;
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    void setMainObject(MainObject *mainObj);

private:
    void onAddAccount();
    MainObject *m_object;
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
