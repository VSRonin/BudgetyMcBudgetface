/****************************************************************************\
   Copyright 2024 Luca Beldi
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
\****************************************************************************/
#include "addaccountdialog.h"
#include "ui_addaccountdialog.h"
#include <globals.h>
#include <QSqlQuery>
#include <QSqlDatabase>

AddAccountDialog::AddAccountDialog(QWidget *parent)
    : QDialog(parent)
    , m_currKeyCol(0)
    , m_accTypKeyCol(0)
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
    return ui->accountTypeCombo->model()->index(ui->accountTypeCombo->currentIndex(), m_currKeyCol).data().toInt();
}

int AddAccountDialog::curr() const
{
    return ui->currencyCombo->model()->index(ui->currencyCombo->currentIndex(), m_accTypKeyCol).data().toInt();
}

void AddAccountDialog::loadCombos(QAbstractItemModel *currenciesModel, QAbstractItemModel *accountTypesModel, int currModelKeyCol,
                                  int currModelValCol, int AccTypModelKeyCol, int AccTypModelValCol)
{
    ui->currencyCombo->setModel(currenciesModel);
    m_currKeyCol = currModelKeyCol;
    ui->currencyCombo->setModelColumn(currModelValCol);
    ui->accountTypeCombo->setModel(accountTypesModel);
    m_accTypKeyCol = AccTypModelKeyCol;
    ui->accountTypeCombo->setModelColumn(AccTypModelValCol);
}
