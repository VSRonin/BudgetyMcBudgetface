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
#include "transactionstab.h"
#include "ui_transactionstab.h"
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
TransactionsTab::TransactionsTab(QWidget *parent)
    : QWidget(parent)
    , m_object(nullptr)
    , ui(new Ui::TransactionsTab)

{
    ui->setupUi(this);
    ui->lastUpdateLabel->hide();
    QMenu *importStatementsMenu = new QMenu(this);
    importStatementsMenu->addAction(ui->actionImport_Barclays);
    importStatementsMenu->addAction(ui->actionImport_Natwest);
    importStatementsMenu->addAction(ui->actionImport_Revolut);
    ui->importStatementButton->setMenu(importStatementsMenu);
    connect(ui->actionImport_Barclays, &QAction::triggered, this, std::bind(&TransactionsTab::importStatement, this, MainObject::ifBarclays));
    connect(ui->actionImport_Natwest, &QAction::triggered, this, std::bind(&TransactionsTab::importStatement, this, MainObject::ifNatwest));
    connect(ui->actionImport_Revolut, &QAction::triggered, this, std::bind(&TransactionsTab::importStatement, this, MainObject::ifRevolut));
    connect(ui->removeTransactionButton, &QPushButton::clicked, this, &TransactionsTab::onRemoveTransactions);
}

TransactionsTab::~TransactionsTab()
{
    delete ui;
}

void TransactionsTab::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    ui->transactionView->setModel(m_object ? m_object->transactionsModel() : nullptr);
    connect(ui->transactionView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this]() { ui->removeTransactionButton->setEnabled(!ui->transactionView->selectionModel()->selectedIndexes().isEmpty()); });
    refreshLastUpdate();
}

void TransactionsTab::importStatement(MainObject::ImportFormats format)
{
    Q_ASSERT(!QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).isEmpty());
    QString path = QFileDialog::getOpenFileName(
            this, tr("Open Statement"), QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first(), tr("Statement Files (*.csv)"));
    if (path.isEmpty())
        return;
    Q_ASSERT(m_object);
    if (!m_object->importStatement(path, format)) {
        QMessageBox::critical(this, tr("Error"), tr("Error while importing the statement. The file might be currupted or in an unexpected format"));
        return;
    }
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
