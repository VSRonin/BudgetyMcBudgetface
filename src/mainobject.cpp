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
#include "mainobject.h"
#include "globals.h"
#include "offlinesqltable.h"
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSaveFile>
#include <QFile>

MainObject::MainObject(QObject *parent)
    : QObject(parent)
    , m_transactionsModel(new OfflineSqlTable(this))
    , m_accountsModel(new OfflineSqlTable(this))
    , m_categoriesModel(new OfflineSqlTable(this))
    , m_currenciesModel(new QStandardItemModel(this))
    , m_accountTypesModel(new QStandardItemModel(this))
    , m_dirty(false)
{
    m_transactionsModel->setTable(QStringLiteral("Transactions"));
    m_accountsModel->setTable(QStringLiteral("Accounts"));
    m_categoriesModel->setTable(QStringLiteral("Categories"));
    Q_ASSUME(m_currenciesModel->insertColumn(0));
    Q_ASSUME(m_accountTypesModel->insertColumn(0));
    Q_ASSUME(m_currenciesModel->setHeaderData(0, Qt::Horizontal, tr("Currency")));
    Q_ASSUME(m_currenciesModel->setHeaderData(0, Qt::Horizontal, tr("Account Type")));
}

MainObject::~MainObject() { }

QAbstractItemModel *MainObject::transactionsModel() const
{
    return m_transactionsModel;
}

QAbstractItemModel *MainObject::accountsModel() const
{
    return m_accountsModel;
}

QAbstractItemModel *MainObject::categoriesModel() const
{
    return m_categoriesModel;
}

QAbstractItemModel *MainObject::currenciesModel() const
{
    return m_currenciesModel;
}

QAbstractItemModel *MainObject::accountTypesModel() const
{
    return m_accountTypesModel;
}

bool MainObject::addAccount(const QString &name, const QString &owner, int curr, int typ)
{
    if (name.isEmpty() || owner.isEmpty())
        return false;
    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return false;
    int newID = 0;
    for (int i = 0, maxI = m_accountsModel->rowCount(); i < maxI; ++i)
        newID = std::max(newID, m_accountsModel->index(i, 0).data().toInt());
    QSqlQuery addAccountQuery(db);
    addAccountQuery.prepare(QStringLiteral("INSERT INTO Accounts (Id, Name, Owner, Currency, AccountType) VALUES (?,?,?,?,?)"));
    addAccountQuery.addBindValue(++newID);
    addAccountQuery.addBindValue(name);
    addAccountQuery.addBindValue(owner);
    addAccountQuery.addBindValue(curr);
    addAccountQuery.addBindValue(typ);
    if (!addAccountQuery.exec()) {
        qDebug() << addAccountQuery.lastError().text();
        return false;
    }
    m_accountsModel->select();
    setDirty(true);
    return true;
}

bool MainObject::isDirty() const
{
    return m_dirty;
}

void MainObject::newBudget()
{
    discardDbFile();
    createDbFile();
    reselectModels();
    setDirty(false);
}

bool MainObject::saveBudget(const QString &path)
{
    if (path.isEmpty())
        return false;
    QFile source(dbFilePath());
    if (!source.open(QFile::ReadOnly))
        return false;
    QSaveFile destination(path);
    if (!destination.open(QSaveFile::WriteOnly))
        return false;
    destination.write(BUDGET_FILE_VERSION);
    while (!source.atEnd())
        destination.write(source.read(1024));
    if (!destination.commit())
        return false;
    setDirty(false);
    return true;
}

bool MainObject::loadBudget(const QString &path)
{
    if (path.isEmpty())
        return false;
    discardDbFile();
    QFile source(path);
    if (!source.open(QFile::ReadOnly))
        return false;
    QSaveFile destination(dbFilePath());
    if (!destination.open(QSaveFile::WriteOnly))
        return false;
    const QByteArray fileVersion = source.read(BUDGET_FILE_VERSION.size());
    if (BUDGET_FILE_VERSION != fileVersion)
        return false;
    while (!source.atEnd())
        destination.write(source.read(1024));
    if (!destination.commit())
        return false;
    reselectModels();
    setDirty(false);
    return true;
}

void MainObject::setDirty(bool dirty)
{
    if (dirty == m_dirty)
        return;
    m_dirty = dirty;
    dirtyChanged(m_dirty);
}

void MainObject::reselectModels()
{
    for (OfflineSqlTable *model : {m_transactionsModel, m_accountsModel, m_categoriesModel})
        model->setTable(model->tableName());

    m_currenciesModel->removeRows(0, m_currenciesModel->rowCount());
    m_accountTypesModel->removeRows(0, m_currenciesModel->rowCount());
    QSqlDatabase db = openDb();
    if (db.isOpen()) {
        {
            QSqlQuery currencyQuery(db);
            currencyQuery.prepare(QStringLiteral("SELECT Id, Currency FROM Currencies"));
            if (currencyQuery.exec()) {
                while (currencyQuery.next()) {
                    const int newRow = m_currenciesModel->rowCount();
                    Q_ASSUME(m_currenciesModel->insertRow(newRow));
                    const QModelIndex currIdx = m_currenciesModel->index(newRow, 0);
                    Q_ASSUME(m_currenciesModel->setData(currIdx, currencyQuery.value(1)));
                    Q_ASSUME(m_currenciesModel->setData(currIdx, currencyQuery.value(0), Qt::UserRole));
                }
            }
        }
        {
            QSqlQuery accountTypeQuery(db);
            accountTypeQuery.prepare(QStringLiteral("SELECT Id, Name FROM AccountTypes"));
            if (accountTypeQuery.exec()) {
                while (accountTypeQuery.next()) {
                    const int newRow = m_accountTypesModel->rowCount();
                    Q_ASSUME(m_accountTypesModel->insertRow(newRow));
                    const QModelIndex currIdx = m_accountTypesModel->index(newRow, 0);
                    Q_ASSUME(m_accountTypesModel->setData(currIdx, accountTypeQuery.value(1)));
                    Q_ASSUME(m_accountTypesModel->setData(currIdx, accountTypeQuery.value(0), Qt::UserRole));
                }
            }
        }
    }
}
