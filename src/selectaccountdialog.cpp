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
#include "selectaccountdialog.h"
#include "ui_selectaccountdialog.h"
#include <mainobject.h>
SelectAccountDialog::SelectAccountDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectAccountDialog)
{
    ui->setupUi(this);
}

SelectAccountDialog::~SelectAccountDialog()
{
    delete ui;
}

void SelectAccountDialog::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    if (m_object)
        ui->accountCombo->setModel(m_object->filteredAccountsModel());
    ui->accountCombo->setModelColumn(MainObject::acName);
}

int SelectAccountDialog::selectedAccountId() const
{
    if (!m_object)
        return -1;
    return m_object->filteredAccountsModel()->index(ui->accountCombo->currentIndex(), MainObject::acId).data().toInt();
}
