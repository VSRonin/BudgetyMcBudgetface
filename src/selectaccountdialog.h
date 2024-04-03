#ifndef SELECTACCOUNTDIALOG_H
#define SELECTACCOUNTDIALOG_H

#include <QDialog>

namespace Ui {
class SelectAccountDialog;
}
class MainObject;
class SelectAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectAccountDialog(QWidget *parent = nullptr);
    ~SelectAccountDialog();
    void setMainObject(MainObject *mainObj);
    int selectedAccountId() const;

private:
    MainObject *m_object;
    Ui::SelectAccountDialog *ui;
};

#endif // SELECTACCOUNTDIALOG_H
