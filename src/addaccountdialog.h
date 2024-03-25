#ifndef ADDACCOUNTDIALOG_H
#define ADDACCOUNTDIALOG_H

#include <QDialog>
class QAbstractItemModel;
namespace Ui {
class AddAccountDialog;
}

class AddAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAccountDialog(QWidget *parent = nullptr);
    ~AddAccountDialog();
    QString name() const;
    QString owner() const;
    int curr() const;
    int typ() const;
    void loadCombos(QAbstractItemModel *currenciesModel, QAbstractItemModel *accountTypesModel);

private:
    Ui::AddAccountDialog *ui;
};

#endif // ADDACCOUNTDIALOG_H
