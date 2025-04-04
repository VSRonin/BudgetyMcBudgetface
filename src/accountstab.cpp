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
#include "accountstab.h"
#include "ui_accountstab.h"
#include "addaccountdialog.h"
#include "relationaldelegate.h"
#include <QMessageBox>
#include <QHeaderView>
#include <mainobject.h>
#include "ownerdelegate.h"
#include "multiplefilterproxy.h"
#include "accountstatusdelegate.h"
#include <blankrowproxy.h>
class OwnerSorter : public AndFilterProxy
{
    Q_DISABLE_COPY_MOVE(OwnerSorter)
public:
    using AndFilterProxy::AndFilterProxy;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

bool OwnerSorter::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (source_left.column() == MainObject::acOwner && source_right.column() == MainObject::acOwner) {
        const int l = source_left.data(sortRole()).toString().size();
        const int r = source_right.data(sortRole()).toString().size();
        if (l < r)
            return true;
        if (l > r)
            return false;
    }
    return AndFilterProxy::lessThan(source_left, source_right);
}

AccountsTab::AccountsTab(QWidget *parent)
    : QWidget(parent)
    , m_object(nullptr)
    , ui(new Ui::AccountsTab)
    , m_currencyDelegate(new RelationalDelegate(this))
    , m_accountTypeDelagate(new RelationalDelegate(this))
    , m_ownerDelegate(new OwnerDelegate(this))
    , m_accountStatusDelegate(new AccountStatusDelegate(this))
    , m_filterProxy(new OwnerSorter(this))
    , m_currencyProxy(new BlankRowProxy(this))
    , m_accountTypeProxy(new BlankRowProxy(this))
    , m_ownerProxy(new BlankRowProxy(this))
{
    ui->setupUi(this);
    ui->accountsView->setModel(m_filterProxy);
    ui->accountsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->currencyFilterCombo->setModel(m_currencyProxy);
    ui->accountTypeFilterCombo->setModel(m_accountTypeProxy);
    ui->ownerFilterCombo->setModel(m_ownerProxy);
    connect(ui->nameFilterEdit, &QLineEdit::textChanged, this, &AccountsTab::onNameFilterChanged);
    connect(ui->currencyFilterCombo, &QComboBox::currentIndexChanged, this, &AccountsTab::onCurrencyFilterChanged);
    connect(ui->accountTypeFilterCombo, &QComboBox::currentIndexChanged, this, &AccountsTab::onAccountTypeFilterChanged);
    connect(ui->ownerFilterCombo, &QComboBox::currentIndexChanged, this, &AccountsTab::onOwnerFilterChanged);
    connect(ui->addAccountButton, &QPushButton::clicked, this, &AccountsTab::onAddAccount);
    connect(ui->removeAccountButton, &QPushButton::clicked, this, &AccountsTab::onRemoveAccount);
    connect(ui->accountsView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this]() { ui->removeAccountButton->setEnabled(!ui->accountsView->selectionModel()->selectedIndexes().isEmpty()); });
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(ui->openAccountCheck, &QCheckBox::stateChanged, this, &AccountsTab::onOpenFilterChanged);
#else
    connect(ui->openAccountCheck, &QCheckBox::checkStateChanged, this, &AccountsTab::onOpenFilterChanged);
#endif
}

AccountsTab::~AccountsTab()
{
    delete ui;
}

void AccountsTab::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    if (m_object) {
        m_filterProxy->setSourceModel(m_object->accountsModel());
        m_currencyProxy->setSourceModel(m_object->currenciesModel());
        m_accountTypeProxy->setSourceModel(m_object->accountTypesModel());
        m_ownerProxy->setSourceModel(m_object->familyModel());
        const auto setupView = [this]() {
            ui->accountsView->setColumnHidden(MainObject::acId, true);
            ui->accountsView->setItemDelegateForColumn(MainObject::acCurrency, m_currencyDelegate);
            ui->accountsView->setItemDelegateForColumn(MainObject::acAccountType, m_accountTypeDelagate);
            ui->accountsView->setItemDelegateForColumn(MainObject::acOwner, m_ownerDelegate);
            ui->accountsView->setItemDelegateForColumn(MainObject::acAccountStatus, m_accountStatusDelegate);
            ui->currencyFilterCombo->setModelColumn(MainObject::ccCurrency);
            ui->accountTypeFilterCombo->setModelColumn(MainObject::atcName);
            ui->ownerFilterCombo->setModelColumn(MainObject::fcName);
        };
        connect(m_object->accountsModel(), &QAbstractItemModel::rowsInserted, this, setupView);
        connect(m_object->accountsModel(), &QAbstractItemModel::modelReset, this, setupView);
        setupView();
    } else
        ui->removeAccountButton->setEnabled(false);
    m_ownerDelegate->setMainObject(m_object);
    m_currencyDelegate->setRelationModel(m_object ? m_object->currenciesModel() : nullptr, MainObject::ccId, MainObject::ccCurrency);
    m_accountTypeDelagate->setRelationModel(m_object ? m_object->accountTypesModel() : nullptr, MainObject::atcId, MainObject::atcName);
}

void AccountsTab::onAddAccount()
{
    Q_ASSERT(m_object);
    AddAccountDialog addAccountDialog(this);
    addAccountDialog.setMainObject(m_object);
    while (addAccountDialog.exec()) {
        if (m_object->addAccount(addAccountDialog.name(), addAccountDialog.owner(), addAccountDialog.curr(), addAccountDialog.typ()))
            break;
        QMessageBox::critical(this, tr("Error"), tr("Failed to add a new account, try again or check your input"));
    }
}

void AccountsTab::onRemoveAccount()
{
    QList<int> idsToRemove;
    const auto seletctedIndexes = ui->accountsView->selectionModel()->selectedIndexes();
    for (auto &&idx : seletctedIndexes)
        idsToRemove.append(idx.sibling(idx.row(), MainObject::acId).data().toInt());
    std::sort(idsToRemove.begin(), idsToRemove.end());
    idsToRemove.erase(std::unique(idsToRemove.begin(), idsToRemove.end()), idsToRemove.end());
    if (QMessageBox::question(this, tr("Are you sure?"),
                              tr("Are you sure you want to remove the selected account(s) and all related transactions?", "", idsToRemove.size()),
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::No)
        return;
    Q_ASSERT(m_object);
    if (!m_object->removeAccounts(idsToRemove))
        QMessageBox::critical(this, tr("Error"), tr("Failed to remove account(s), try again later", "", idsToRemove.size()));
}

void AccountsTab::onNameFilterChanged(const QString &text)
{
    if (text.isEmpty())
        return m_filterProxy->removeFilterFromColumn(MainObject::acName);
    m_filterProxy->setRegExpFilter(MainObject::acName, QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
}

void AccountsTab::onCurrencyFilterChanged(int newIndex)
{
    if (newIndex == 0)
        return m_filterProxy->removeFilterFromColumn(MainObject::acCurrency);
    m_filterProxy->setRegExpFilter(
            MainObject::acCurrency,
            QRegularExpression(QString::number(m_object->currenciesModel()->index(newIndex - 1, MainObject::ccId).data().toInt())));
}

void AccountsTab::onAccountTypeFilterChanged(int newIndex)
{
    if (newIndex == 0)
        return m_filterProxy->removeFilterFromColumn(MainObject::acAccountType);
    m_filterProxy->setRegExpFilter(
            MainObject::acAccountType,
            QRegularExpression(QString::number(m_object->accountTypesModel()->index(newIndex - 1, MainObject::atcId).data().toInt())));
}

void AccountsTab::onOwnerFilterChanged(int newIndex)
{
    if (newIndex == 0)
        return m_filterProxy->removeFilterFromColumn(MainObject::acOwner);
    m_filterProxy->setRegExpFilter(
            MainObject::acOwner, QRegularExpression(QString::number(m_object->familyModel()->index(newIndex - 1, MainObject::fcId).data().toInt())));
}

void AccountsTab::onOpenFilterChanged()
{
    if (ui->openAccountCheck->checkState() == Qt::Checked)
        m_filterProxy->setRegExpFilter(MainObject::acAccountStatus, QRegularExpression(QStringLiteral("1")));
    else
        m_filterProxy->removeFilterFromColumn(MainObject::acAccountStatus);
}
