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
#include "blankrowproxy.h"
#include "decimaldelegate.h"
#include "isodatedelegate.h"
#include "multiplefilterproxy.h"
#include "relationaldelegate.h"
#include "selectaccountdialog.h"
#include "transactionstab.h"
#include "ui_transactionstab.h"
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>



TransactionsTab::TransactionsTab(QWidget *parent)
    : QWidget(parent)
    , m_object(nullptr)
    , m_filterProxy(new OrFilterProxy(this))
    , m_currencyDelegate(new RelationalDelegate(this))
    , m_accountDelegate(new RelationalDelegate(this))
    , m_categoryDelegate(new RelationalDelegate(this))
    , m_subcategoryDelegate(new FilteredRelationalDelegate(this))
    , m_movementTypeDelegate(new RelationalDelegate(this))
    , m_amountDelegate(new DecimalDelegate(this))
    , m_opDateDelegate(new IsoDateDelegate(this))
    , m_currencyProxy(new BlankRowProxy(this))
    , m_accountProxy(new BlankRowProxy(this))
    , ui(new Ui::TransactionsTab)

{
    ui->setupUi(this);
    ui->lastUpdateLabel->hide();
    ui->transactionView->setModel(m_filterProxy);
    ui->currencyFilterCombo->setModel(m_currencyProxy);
    ui->accountFilterCombo->setModel(m_accountProxy);
    QMenu *importStatementsMenu = new QMenu(this);
    importStatementsMenu->addAction(ui->actionImport_Barclays);
    importStatementsMenu->addAction(ui->actionImport_Natwest);
    importStatementsMenu->addAction(ui->actionImport_Revolut);
    ui->importStatementButton->setMenu(importStatementsMenu);
    connect(ui->actionImport_Barclays, &QAction::triggered, this, std::bind(&TransactionsTab::importStatement, this, MainObject::ifBarclays));
    connect(ui->actionImport_Natwest, &QAction::triggered, this, std::bind(&TransactionsTab::importStatement, this, MainObject::ifNatwest));
    connect(ui->actionImport_Revolut, &QAction::triggered, this, std::bind(&TransactionsTab::importStatement, this, MainObject::ifRevolut));
    connect(ui->removeTransactionButton, &QPushButton::clicked, this, &TransactionsTab::onRemoveTransactions);
    connect(ui->currencyFilterCombo, &QComboBox::currentIndexChanged, this, &TransactionsTab::onFilterChanged);
    connect(ui->accountFilterCombo, &QComboBox::currentIndexChanged, this, &TransactionsTab::onFilterChanged);
    connect(ui->fromDateEdit, &QDateEdit::dateChanged, this, &TransactionsTab::onFilterChanged);
    connect(ui->toDateEdit, &QDateEdit::dateChanged, this, &TransactionsTab::onFilterChanged);
    connect(ui->descriptionFilterEdit, &QLineEdit::textChanged, this, &TransactionsTab::onFilterChanged);
    connect(ui->paymentTypeFilterEdit, &QLineEdit::textChanged, this, &TransactionsTab::onFilterChanged);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(ui->showUncategorisedCheck, &QCheckBox::stateChanged, this, &TransactionsTab::onShowWIPChanged);
#else
    connect(ui->showUncategorisedCheck, &QCheckBox::checkStateChanged, this, &TransactionsTab::onShowWIPChanged);
#endif
    connect(ui->transactionView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this]() { ui->removeTransactionButton->setEnabled(!ui->transactionView->selectionModel()->selectedIndexes().isEmpty()); });
}

TransactionsTab::~TransactionsTab()
{
    delete ui;
}

void TransactionsTab::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    if (m_object) {
        m_filterProxy->setSourceModel(m_object->transactionsModel());
        m_currencyProxy->setSourceModel(m_object->currenciesModel());
        m_accountProxy->setSourceModel(m_object->accountsModel());
        const auto setupView = [this]() {
            ui->transactionView->setColumnHidden(MainObject::tcId, true);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcCurrency, m_currencyDelegate);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcAccount, m_accountDelegate);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcDestinationAccount, m_accountDelegate);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcCategory, m_categoryDelegate);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcSubcategory, m_subcategoryDelegate);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcMovementType, m_movementTypeDelegate);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcAmount, m_amountDelegate);
            ui->transactionView->setItemDelegateForColumn(MainObject::tcOpDate, m_opDateDelegate);
            ui->currencyFilterCombo->setModelColumn(MainObject::ccCurrency);
            ui->accountFilterCombo->setModelColumn(MainObject::acName);
            refreshLastUpdate();
        };
        connect(m_object->transactionsModel(), &QAbstractItemModel::rowsInserted, this, setupView);
        connect(m_object->transactionsModel(), &QAbstractItemModel::modelReset, this, setupView);
        setupView();
    } else
        ui->removeTransactionButton->setEnabled(false);
    m_currencyDelegate->setRelationModel(m_object ? m_object->currenciesModel() : nullptr, MainObject::ccId, MainObject::ccCurrency);
    m_accountDelegate->setRelationModel(m_object ? m_object->accountsModel() : nullptr, MainObject::acId, MainObject::acName);
    m_categoryDelegate->setRelationModel(m_object ? m_object->categoriesModel() : nullptr, MainObject::cacId, MainObject::cacName);
    m_subcategoryDelegate->setRelationModel(m_object ? m_object->subcategoriesModel() : nullptr, MainObject::sccId, MainObject::sccName);
    m_subcategoryDelegate->setRelationFilterColumn(MainObject::sccCategoryId);
    m_subcategoryDelegate->setFilterKeyColumn(MainObject::tcCategory);
    m_movementTypeDelegate->setRelationModel(m_object ? m_object->movementTypesModel() : nullptr, MainObject::mtcId, MainObject::mtcName);
}

void TransactionsTab::onShowWIPChanged()
{
    if (ui->showUncategorisedCheck->checkState() == Qt::Checked) {
        m_filterProxy->setNegativeRegExpFilter(MainObject::tcCategory, QStringLiteral(".+"));
        m_filterProxy->setNegativeRegExpFilter(MainObject::tcSubcategory, QStringLiteral(".+"));
        m_filterProxy->setNegativeRegExpFilter(MainObject::tcMovementType, QStringLiteral(".+"));
    } else {
        for (int col : {MainObject::tcCategory, MainObject::tcSubcategory, MainObject::tcMovementType})
            m_filterProxy->removeFilterFromColumn(col);
    }
}

void TransactionsTab::importStatement(MainObject::ImportFormats format)
{
    Q_ASSERT(m_object);
    SelectAccountDialog selectAccountDialog;
    selectAccountDialog.setMainObject(m_object);
    if (!selectAccountDialog.exec())
        return;
    Q_ASSERT(!QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).isEmpty());
    QString path = QFileDialog::getOpenFileName(
            this, tr("Open Statement"), QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first(), tr("Statement Files (*.csv)"));
    if (path.isEmpty())
        return;
    if (!m_object->importStatement(selectAccountDialog.selectedAccountId(), path, format)) {
        QMessageBox::critical(this, tr("Error"), tr("Error while importing the statement. The file might be currupted or in an unexpected format"));
        return;
    }
    refreshLastUpdate();
}

void TransactionsTab::onRemoveTransactions()
{
    QList<int> idsToRemove;
    const auto seletctedIndexes = ui->transactionView->selectionModel()->selectedIndexes();
    for (auto &&idx : seletctedIndexes)
        idsToRemove.append(idx.sibling(idx.row(), MainObject::tcId).data().toInt());
    std::sort(idsToRemove.begin(), idsToRemove.end());
    idsToRemove.erase(std::unique(idsToRemove.begin(), idsToRemove.end()), idsToRemove.end());
    if (QMessageBox::question(this, tr("Are you sure?"), tr("Are you sure you want to remove the selected transaction(s)?", "", idsToRemove.size()),
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::No)
        return;
    Q_ASSERT(m_object);
    if (!m_object->removeTransactions(idsToRemove))
        QMessageBox::critical(this, tr("Error"), tr("Failed to remove transaction(s), try again later", "", idsToRemove.size()));
}

void TransactionsTab::refreshLastUpdate()
{
    if (!m_object)
        return ui->lastUpdateLabel->hide();
    const QDate lastUpdDt = m_object->lastTransactionDate();
    ui->lastUpdateLabel->setVisible(lastUpdDt.isValid());
    if (lastUpdDt.isValid())
        ui->lastUpdateLabel->setText(tr("Last Update: %1").arg(locale().toString(lastUpdDt)));
}

void TransactionsTab::onFilterChanged()
{
    QList<MainObject::TransactionModelColumn> cols;
    QStringList filters;
    if (ui->currencyFilterCombo->currentIndex() > 0) {
        cols.append(MainObject::tcCurrency);
        filters.append(
                QLatin1Char('=')
                + QString::number(m_object->currenciesModel()->index(ui->currencyFilterCombo->currentIndex() - 1, MainObject::ccId).data().toInt()));
    }
    if (ui->accountFilterCombo->currentIndex() > 0) {
        cols.append(MainObject::tcAccount);
        filters.append(
                QLatin1Char('=')
                + QString::number(m_object->accountsModel()->index(ui->accountFilterCombo->currentIndex() - 1, MainObject::acId).data().toInt()));
    }
    if (ui->fromDateEdit->date() != ui->fromDateEdit->minimumDate()) {
        cols.append(MainObject::tcOpDate);
        filters.append(QLatin1String(">='") + ui->fromDateEdit->date().toString(Qt::ISODate) + QLatin1Char('\''));
    }
    if (ui->toDateEdit->date() != ui->toDateEdit->minimumDate()) {
        cols.append(MainObject::tcOpDate);
        filters.append(QLatin1String("<='") + ui->toDateEdit->date().toString(Qt::ISODate) + QLatin1Char('\''));
    }
    if (!ui->descriptionFilterEdit->text().isEmpty()) {
        cols.append(MainObject::tcDescription);
        filters.append(QLatin1String(" LIKE '%") + ui->descriptionFilterEdit->text() + QLatin1String("%'")); // TODO: escape identifier
    }
    if (!ui->paymentTypeFilterEdit->text().isEmpty()) {
        cols.append(MainObject::tcPaymentType);
        filters.append(QLatin1String(" LIKE '%") + ui->paymentTypeFilterEdit->text() + QLatin1String("%'")); // TODO: escape identifier
    }
    m_object->setTransactionsFilter(cols, filters);
}
