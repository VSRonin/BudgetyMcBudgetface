#include "addaccountdialog.h"
#include "ui_addaccountdialog.h"
#include <globals.h>
#include <QSqlQuery>
#include <QSqlDatabase>

AddAccountDialog::AddAccountDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddAccountDialog)
{
    ui->setupUi(this);
}

AddAccountDialog::~AddAccountDialog()
{
    delete ui;
}

QString AddAccountDialog::name() const
{
    return ui->nameEdit->text();
}

QString AddAccountDialog::owner() const
{
    return ui->ownerEdit->text();
}

int AddAccountDialog::typ() const
{
    return ui->accountTypeCombo->currentData().toInt();
}

int AddAccountDialog::curr() const
{
    return ui->currencyCombo->currentData().toInt();
}

void AddAccountDialog::loadCombos(QAbstractItemModel *currenciesModel, QAbstractItemModel *accountTypesModel)
{
    ui->currencyCombo->setModel(currenciesModel);
    ui->accountTypeCombo->setModel(accountTypesModel);
}
