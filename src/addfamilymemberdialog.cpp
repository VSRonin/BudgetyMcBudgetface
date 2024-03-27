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
#include "addfamilymemberdialog.h"
#include "ui_addfamilymemberdialog.h"
#include <mainobject.h>
#include <QPushButton>
AddFamilyMemberDialog::AddFamilyMemberDialog(QWidget *parent)
    : QDialog(parent)
    , m_object(nullptr)
    , ui(new Ui::AddFamilyMemberDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->birthdayEdit->setDate(QDate::currentDate().addYears(-20));
    ui->incomeScaleCombo->addItem(tr("Weekly"), 52.0);
    ui->incomeScaleCombo->addItem(tr("Monthly"), 12.0);
    ui->incomeScaleCombo->addItem(tr("Yearly"), 1.0);
    ui->incomeScaleCombo->setCurrentIndex(2);
    connect(ui->nameEdit, &QLineEdit::textChanged, this, &AddFamilyMemberDialog::checkOkEnabled);
    connect(ui->birthdayEdit, &QDateEdit::dateChanged, this, &AddFamilyMemberDialog::checkOkEnabled);
    connect(ui->incomeSpin, &QDoubleSpinBox::valueChanged, this, &AddFamilyMemberDialog::checkOkEnabled);
    connect(ui->incomeScaleCombo, &QComboBox::currentIndexChanged, this, &AddFamilyMemberDialog::checkOkEnabled);
    connect(ui->incomeCurrencyCombo, &QComboBox::currentIndexChanged, this, &AddFamilyMemberDialog::checkOkEnabled);
}

AddFamilyMemberDialog::~AddFamilyMemberDialog()
{
    delete ui;
}

void AddFamilyMemberDialog::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    ui->incomeCurrencyCombo->setModel(m_object ? m_object->currenciesModel() : nullptr);
    ui->incomeCurrencyCombo->setModelColumn(MainObject::ccCurrency);
    if (m_object)
        ui->incomeCurrencyCombo->setCurrentIndex(ui->incomeCurrencyCombo->findText(m_object->baseCurrency()));
}

QString AddFamilyMemberDialog::name() const
{
    return ui->nameEdit->text();
}

QDate AddFamilyMemberDialog::birthday() const
{
    return ui->birthdayEdit->date();
}

double AddFamilyMemberDialog::annualIncome() const
{
    return ui->incomeSpin->value() * ui->incomeScaleCombo->currentData().toDouble();
}

int AddFamilyMemberDialog::incomeCurrency() const
{
    if (m_object)
        return m_object->currenciesModel()->index(ui->incomeCurrencyCombo->currentIndex(), MainObject::ccId).data().toInt();
    return 0;
}

void AddFamilyMemberDialog::checkOkEnabled()
{
    const bool result = ui->birthdayEdit->date().isValid() && !ui->nameEdit->text().isEmpty() && !ui->incomeCurrencyCombo->currentText().isEmpty()
            && !ui->incomeScaleCombo->currentText().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(result);
}
