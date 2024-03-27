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
#include "familytab.h"
#include "ui_familytab.h"
#include <mainobject.h>
#include "decimaldelegate.h"
#include "relationaldelegate.h"
#include <QMessageBox>
#include "addfamilymemberdialog.h"
#include "isodatedelegate.h"
#include <QHeaderView>
FamilyTab::FamilyTab(QWidget *parent)
    : QWidget(parent)
    , m_object(nullptr)
    , ui(new Ui::FamilyTab)
    , m_currencyDelegate(new RelationalDelegate(this))
    , m_incomeDelegate(new DecimalDelegate(this))
    , m_birthdayDelegate(new IsoDateDelegate(this))
{
    ui->setupUi(this);
    ui->familyView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(ui->addFamilyButton, &QPushButton::clicked, this, &FamilyTab::onAddFamily);
    connect(ui->removeFamilyButton, &QPushButton::clicked, this, &FamilyTab::onRemoveFamily);
}

FamilyTab::~FamilyTab()
{
    delete ui;
}

void FamilyTab::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    if (m_object) {
        ui->familyView->setModel(m_object->familyModel());
        const auto setupView = [this]() {
            ui->familyView->setColumnHidden(MainObject::fcId, true);
            ui->familyView->setItemDelegateForColumn(MainObject::fcIncomeCurrency, m_currencyDelegate);
            ui->familyView->setItemDelegateForColumn(MainObject::fcIncome, m_incomeDelegate);
            ui->familyView->setItemDelegateForColumn(MainObject::fcBirthday, m_birthdayDelegate);
        };
        connect(m_object->familyModel(), &QAbstractItemModel::rowsInserted, this, setupView);
        connect(m_object->familyModel(), &QAbstractItemModel::modelReset, this, setupView);
        setupView();
    } else
        ui->removeFamilyButton->setEnabled(false);
    m_currencyDelegate->setRelationModel(m_object ? m_object->currenciesModel() : nullptr, MainObject::ccId, MainObject::ccCurrency);
    connect(ui->familyView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this]() { ui->removeFamilyButton->setEnabled(!ui->familyView->selectionModel()->selectedIndexes().isEmpty()); });
}

void FamilyTab::onAddFamily()
{
    Q_ASSERT(m_object);
    AddFamilyMemberDialog addFamilyDialog(this);
    addFamilyDialog.setMainObject(m_object);
    while (addFamilyDialog.exec()) {
        if (m_object->addFamilyMember(addFamilyDialog.name(), addFamilyDialog.birthday(), addFamilyDialog.annualIncome(),
                                      addFamilyDialog.incomeCurrency()))
            break;
        QMessageBox::critical(this, tr("Error"), tr("Failed to add a new family member, try again or check your input"));
    }
}

void FamilyTab::onRemoveFamily()
{
    QList<int> idsToRemove;
    const auto seletctedIndexes = ui->familyView->selectionModel()->selectedIndexes();
    for (auto &&idx : seletctedIndexes)
        idsToRemove.append(idx.sibling(idx.row(), MainObject::fcId).data().toInt());
    std::sort(idsToRemove.begin(), idsToRemove.end());
    idsToRemove.erase(std::unique(idsToRemove.begin(), idsToRemove.end()), idsToRemove.end());
    if (QMessageBox::question(this, tr("Are you sure?"),
                              tr("Are you sure you want to remove the selected family member(s) and their connected accounts and transactions?", "",
                                 idsToRemove.size()),
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::No)
        return;
    Q_ASSERT(m_object);
    if (!m_object->removeFamilyMembers(idsToRemove))
        QMessageBox::critical(this, tr("Error"), tr("Failed to remove family member(s), try again later", "", idsToRemove.size()));
}
