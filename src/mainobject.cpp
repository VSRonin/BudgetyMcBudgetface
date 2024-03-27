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
#include <QSaveFile>
#include <QFile>
#include <QMap>
#include <QRegularExpression>
#ifdef QT_DEBUG
#    include <QSqlError>
#endif
MainObject::MainObject(QObject *parent)
    : QObject(parent)
    , m_transactionsModel(new OfflineSqlTable(this))
    , m_accountsModel(new OfflineSqlTable(this))
    , m_categoriesModel(new OfflineSqlTable(this))
    , m_currenciesModel(new OfflineSqlTable(this))
    , m_accountTypesModel(new OfflineSqlTable(this))
    , m_familyModel(new OfflineSqlTable(this))
    , m_dirty(false)
    , m_baseCurrency(QStringLiteral("GBP"))
{
    m_transactionsModel->setTable(QStringLiteral("Transactions"));
    m_accountsModel->setTable(QStringLiteral("Accounts"));
    m_categoriesModel->setTable(QStringLiteral("Categories"));
    m_currenciesModel->setTable(QStringLiteral("Currencies"));
    m_accountTypesModel->setTable(QStringLiteral("AccountTypes"));
    m_familyModel->setTable(QStringLiteral("Family"));
    for (OfflineSqlTable *model : {m_transactionsModel, m_accountsModel, m_categoriesModel, m_currenciesModel, m_accountTypesModel, m_familyModel})
        connect(model, &QAbstractItemModel::dataChanged, this, std::bind(&MainObject::setDirty, this, true));
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

QAbstractItemModel *MainObject::familyModel() const
{
    return m_familyModel;
}

bool MainObject::addFamilyMember(const QString &name, const QDate &birthday, double income, int incomeCurr)
{
    if (name.isEmpty())
        return false;
    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return false;
    int newID = 0;
    for (int i = 0, maxI = m_familyModel->rowCount(); i < maxI; ++i)
        newID = std::max(newID, m_familyModel->index(i, fcId).data().toInt());
    QSqlQuery addFamilyMemberQuery(db);
    addFamilyMemberQuery.prepare(QStringLiteral("INSERT INTO Family (Id, Name, Birthday, TaxableIncome, IncomeCurrency) VALUES (?,?,?,?,?)"));
    addFamilyMemberQuery.addBindValue(++newID);
    addFamilyMemberQuery.addBindValue(name);
    addFamilyMemberQuery.addBindValue(birthday.toString(Qt::ISODate));
    addFamilyMemberQuery.addBindValue(income);
    addFamilyMemberQuery.addBindValue(incomeCurr);
    if (!addFamilyMemberQuery.exec()) {
#ifdef QT_DEBUG
        qDebug() << addFamilyMemberQuery.lastQuery() << addFamilyMemberQuery.lastError().text();
#endif
        return false;
    }
    m_familyModel->select();
    setDirty(true);
    return true;
}

bool MainObject::removeFamilyMembers(const QList<int> &ids)
{
    if (ids.isEmpty())
        return false;
    QString filterString;
    for (int id : ids)
        filterString += QString::number(id) + QLatin1Char(',');
    filterString.chop(1);
    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return false;
    if (!db.transaction())
        return false;
    QList<int> accountsToRemove;
    QMap<int, QString> accountsToAmend;
    for (int id : ids) {
        QSqlQuery impactedAccountsQuery(db);
        impactedAccountsQuery.prepare(QStringLiteral("SELECT Id, Owner FROM Accounts WHERE Owner LIKE '%") + QString::number(id)
                                      + QStringLiteral("%'"));
        if (!impactedAccountsQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << impactedAccountsQuery.lastQuery() << impactedAccountsQuery.lastError().text();
#endif
            Q_ASSUME(db.rollback());
            return false;
        }
        while (impactedAccountsQuery.next()) {
            const QStringList owners = impactedAccountsQuery.value(1).toString().split(QLatin1Char(','), Qt::SkipEmptyParts);
            if (std::all_of(owners.cbegin(), owners.cend(), [&ids](const QString &own) -> bool { return ids.contains(own.toInt()); }))
                accountsToRemove.append(impactedAccountsQuery.value(0).toInt());
            else
                accountsToAmend[impactedAccountsQuery.value(0).toInt()] = impactedAccountsQuery.value(1).toString();
        }
    }
    for (auto i = accountsToAmend.cbegin(), iEnd = accountsToAmend.cend(); i != iEnd; ++i) {
        QString newOwner = i.value();
        QString ownersToRemoveRegExpPattern = QStringLiteral("(?:");
        for (int id : ids)
            ownersToRemoveRegExpPattern += QString::number(id) + QLatin1Char('|');
        ownersToRemoveRegExpPattern.chop(1);
        ownersToRemoveRegExpPattern += QStringLiteral("),?");
        newOwner.replace(QRegularExpression(ownersToRemoveRegExpPattern), QString());
        if (newOwner.at(newOwner.size() - 1) == QLatin1Char(','))
            newOwner.chop(1);
        QSqlQuery updateAccountQuery(db);
        updateAccountQuery.prepare(QStringLiteral("UPDATE Accounts SET Owner = ? WHERE Id = ?"));
        updateAccountQuery.addBindValue(newOwner);
        updateAccountQuery.addBindValue(i.key());
        if (!updateAccountQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << updateAccountQuery.lastQuery() << updateAccountQuery.lastError().text();
#endif
            Q_ASSUME(db.rollback());
            return false;
        }
    }
    if (!accountsToRemove.isEmpty()) {
        if (!removeAccounts(accountsToRemove, false)) {
            Q_ASSUME(db.rollback());
            return false;
        }
    }
    {
        QSqlQuery removeFamilyQuery(db);
        removeFamilyQuery.prepare(QStringLiteral("DELETE FROM Family WHERE Id IN (") + filterString + QLatin1Char(')'));
        if (!removeFamilyQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << removeFamilyQuery.lastQuery() << removeFamilyQuery.lastError().text();
#endif
            Q_ASSUME(db.rollback());
            return false;
        }
    }
    if (!db.commit())
        return false;
    m_familyModel->select();
    if (!accountsToAmend.isEmpty())
        m_accountsModel->select();
    setDirty(true);
    return true;
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
        newID = std::max(newID, m_accountsModel->index(i, acId).data().toInt());
    QSqlQuery addAccountQuery(db);
    addAccountQuery.prepare(QStringLiteral("INSERT INTO Accounts (Id, Name, Owner, Currency, AccountType) VALUES (?,?,?,?,?)"));
    addAccountQuery.addBindValue(++newID);
    addAccountQuery.addBindValue(name);
    addAccountQuery.addBindValue(owner);
    addAccountQuery.addBindValue(curr);
    addAccountQuery.addBindValue(typ);
    if (!addAccountQuery.exec()) {
#ifdef QT_DEBUG
        qDebug() << addAccountQuery.lastQuery() << addAccountQuery.lastError().text();
#endif
        return false;
    }
    m_accountsModel->select();
    setDirty(true);
    return true;
}

bool MainObject::removeAccounts(const QList<int> &ids)
{
    return removeAccounts(ids, true);
}

bool MainObject::removeAccounts(const QList<int> &ids, bool transaction)
{
    if (ids.isEmpty())
        return false;
    QString filterString;
    for (int id : ids)
        filterString += QString::number(id) + QLatin1Char(',');
    filterString.chop(1);
    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return false;
    if (transaction) {
        if (!db.transaction())
            return false;
    }
    {
        QSqlQuery removeTransactionsQuery(db);
        removeTransactionsQuery.prepare(QStringLiteral("DELETE FROM Transactions WHERE Account IN (") + filterString + QLatin1Char(')'));
        if (!removeTransactionsQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << removeTransactionsQuery.lastQuery() << removeTransactionsQuery.lastError().text();
#endif
            Q_ASSUME(db.rollback());
            return false;
        }
    }
    {
        QSqlQuery removeAccountQuery(db);
        removeAccountQuery.prepare(QStringLiteral("DELETE FROM Accounts WHERE Id IN (") + filterString + QLatin1Char(')'));
        if (!removeAccountQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << removeAccountQuery.lastQuery() << removeAccountQuery.lastError().text();
#endif
            Q_ASSUME(db.rollback());
            return false;
        }
    }
    if (transaction) {
        if (!db.commit())
            return false;
    }
    m_accountsModel->select();
    m_transactionsModel->select();
    setDirty(true);
    return true;
}

bool MainObject::removeTransactions(const QList<int> &ids)
{
    if (ids.isEmpty())
        return false;
    QString filterString;
    for (int id : ids)
        filterString += QString::number(id) + QLatin1Char(',');
    filterString.chop(1);
    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return false;
    QSqlQuery removeTransactionsQuery(db);
    removeTransactionsQuery.prepare(QStringLiteral("DELETE FROM Transactions WHERE Id IN (") + filterString + QLatin1Char(')'));
    if (!removeTransactionsQuery.exec()) {
#ifdef QT_DEBUG
        qDebug() << removeTransactionsQuery.lastQuery() << removeTransactionsQuery.lastError().text();
#endif
        return false;
    }
    m_transactionsModel->select();
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

bool MainObject::importStatement(const QString &path, ImportFormats format)
{
    // TODO
    return false;
}

QDate MainObject::lastTransactionDate() const
{
    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return QDate();
    QSqlQuery lastTransactionQuery(db);
    lastTransactionQuery.prepare(QStringLiteral("SELECT TOP 1 OperationDate FROM Transactions ORDER BY OperationDate DESC"));
    if (!lastTransactionQuery.exec())
        return QDate();
    if (!lastTransactionQuery.next())
        return QDate();
    return QDate::fromString(lastTransactionQuery.value(0).toString(), Qt::ISODate);
}

const QString &MainObject::baseCurrency() const
{
    return m_baseCurrency;
}

bool MainObject::setBaseCurrency(const QString &crncy)
{
    if (m_baseCurrency == crncy)
        return true;
    for (int i = 0, maxI = m_currenciesModel->rowCount(); i < maxI; ++i) {
        if (crncy == m_currenciesModel->index(i, ccCurrency).data().toString()) {
            m_baseCurrency = crncy;
            baseCurrencyChanged();
            return true;
        }
    }
    return false;
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
    for (OfflineSqlTable *model : {m_transactionsModel, m_accountsModel, m_categoriesModel, m_currenciesModel, m_accountTypesModel, m_familyModel})
        model->setTable(model->tableName());
}
