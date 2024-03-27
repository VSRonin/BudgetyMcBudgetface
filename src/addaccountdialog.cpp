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
#include <QPushButton>
#include <QRegularExpression>
#include <mainobject.h>
AddAccountDialog::AddAccountDialog(QWidget *parent)
    : QDialog(parent)
    , m_object(nullptr)
    , ui(new Ui::AddAccountDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(ui->nameEdit, &QLineEdit::textChanged, this, &AddAccountDialog::checkOkEnabled);
    connect(ui->ownerCombo, &MultichoiceCombo::chosenTextChanged, this, &AddAccountDialog::checkOkEnabled);
    connect(ui->currencyCombo, &QComboBox::currentIndexChanged, this, &AddAccountDialog::checkOkEnabled);
    connect(ui->accountTypeCombo, &QComboBox::currentIndexChanged, this, &AddAccountDialog::checkOkEnabled);
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
    if (!m_object)
        return QString();
    const QList<int> ownersIdx = ui->ownerCombo->checkedIndexes();
    QString result;
    for (int i : ownersIdx)
        result += QString::number(m_object->familyModel()->index(i, MainObject::fcId).data().toInt()) + QLatin1Char(',');
    result.chop(1);
    return result;
}

int AddAccountDialog::typ() const
{
    return ui->accountTypeCombo->model()->index(ui->accountTypeCombo->currentIndex(), MainObject::ccId).data().toInt();
}

int AddAccountDialog::curr() const
{
    return ui->currencyCombo->model()->index(ui->currencyCombo->currentIndex(), MainObject::atcId).data().toInt();
}

void AddAccountDialog::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    if (m_object) {
        ui->currencyCombo->setModel(m_object->currenciesModel());
        ui->accountTypeCombo->setModel(m_object->accountTypesModel());
        ui->ownerCombo->setModel(m_object->familyModel());
    }
    ui->currencyCombo->setModelColumn(MainObject::ccCurrency);
    ui->accountTypeCombo->setModelColumn(MainObject::atcName);
    ui->ownerCombo->setModelColumn(MainObject::fcName);
}

void AddAccountDialog::checkOkEnabled()
{
    const bool result = !ui->ownerCombo->chosenText().isEmpty() && !ui->nameEdit->text().isEmpty() && !ui->currencyCombo->currentText().isEmpty()
            && !ui->accountTypeCombo->currentText().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(result);
}
