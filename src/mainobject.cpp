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
#include "offlinesqlitetable.h"
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
    , m_transactionsModel(new OfflineSqliteTable(this))
    , m_accountsModel(new OfflineSqliteTable(this))
    , m_categoriesModel(new OfflineSqliteTable(this))
    , m_currenciesModel(new OfflineSqliteTable(this))
    , m_movementTypesModel(new OfflineSqliteTable(this))
    , m_accountTypesModel(new OfflineSqliteTable(this))
    , m_familyModel(new OfflineSqliteTable(this))
    , m_dirty(false)
    , m_baseCurrency(QStringLiteral("GBP"))
{
    m_transactionsModel->setTable(QStringLiteral("Transactions"));
    m_accountsModel->setTable(QStringLiteral("Accounts"));
    m_categoriesModel->setTable(QStringLiteral("Categories"));
    m_currenciesModel->setTable(QStringLiteral("Currencies"));
    m_movementTypesModel->setTable(QStringLiteral("MovementTypes"));
    m_accountTypesModel->setTable(QStringLiteral("AccountTypes"));
    m_familyModel->setTable(QStringLiteral("Family"));
    for (OfflineSqliteTable *model :
         {m_transactionsModel, m_accountsModel, m_categoriesModel, m_currenciesModel, m_accountTypesModel, m_familyModel, m_movementTypesModel})
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

QAbstractItemModel *MainObject::movementTypesModel() const
{
    return m_movementTypesModel;
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

bool MainObject::importStatement(int account, const QString &path, ImportFormats format)
{
    QFile source(path);
    if (!source.open(QFile::ReadOnly | QFile::Text))
        return false;
    switch (format) {
    case MainObject::ifBarclays:
        return importBarclaysStatement(account, &source);
    case MainObject::ifNatwest:
        break;
    case MainObject::ifRevolut:
        break;
    }
    Q_UNREACHABLE();
    return false;
}

int MainObject::idForCurrency(const QString &curr) const
{
    for (int i = 0; i < m_currenciesModel->rowCount(); ++i) {
        if (m_currenciesModel->index(i, ccCurrency).data().toString().trimmed().compare(curr, Qt::CaseInsensitive) == 0)
            return m_currenciesModel->index(i, ccId).data().toInt();
    }
    return -1;
}

int MainObject::idForMovementType(const QString &mov) const
{
    for (int i = 0; i < m_movementTypesModel->rowCount(); ++i) {
        if (m_movementTypesModel->index(i, mtcName).data().toString().trimmed().compare(mov, Qt::CaseInsensitive) == 0)
            return m_movementTypesModel->index(i, mtcId).data().toInt();
    }
    return -1;
}

bool MainObject::importBarclaysStatement(int account, QFile *source)
{
    QTextStream stream(source);
    QString line;
    bool needCheckFirstLine = true;
    const int gbpID = idForCurrency(QStringLiteral("GBP"));
    if (gbpID < 0)
        return false;
    QList<QDate> opDates;
    QList<double> amounts;
    QList<QString> payTypes;
    QList<QString> descriptions;
    QList<int> movTypes;
    while (stream.readLineInto(&line)) {
        auto parts = line.split(QLatin1Char(','), Qt::KeepEmptyParts);
        if (line.trimmed().isEmpty())
            continue;
        while (parts.size() > 6)
            parts[parts.size() - 2] += parts.takeLast();
        if (parts.size() != 6)
            return false;
        if (needCheckFirstLine) {
            const QString expectedHeaders[6] = {QStringLiteral("Number"), QStringLiteral("Date"),        QStringLiteral("Account"),
                                                QStringLiteral("Amount"), QStringLiteral("Subcategory"), QStringLiteral("Memo")};
            for (int i = 0; i < 6; ++i) {
                if (parts.at(i).trimmed().compare(expectedHeaders[i], Qt::CaseInsensitive) != 0)
                    return false;
            }
            needCheckFirstLine = false;
        } else {
            bool amountCheck = false;
            const double amnt = parts.at(3).trimmed().toDouble(&amountCheck);
            if (!amountCheck)
                return false;
            if (qFuzzyIsNull(amnt))
                continue;
            amounts.append(amnt);
            opDates.append(QDate::fromString(parts.at(1).trimmed(), QStringLiteral("dd/MM/yyyy")));
            payTypes.append(parts.at(4).trimmed());
            descriptions.append(parts.at(5).trimmed());
            if (amnt < 0)
                movTypes.append(idForMovementType(QStringLiteral("Expense")));
            else
                movTypes.append(idForMovementType(QStringLiteral("Income")));
        }
    }
    return addTransactions(account, opDates, {gbpID}, amounts, payTypes, descriptions, QList<int>(), QList<int>(), movTypes, QList<int>(),
                           QList<double>());
}

QDate MainObject::lastTransactionDate() const
{
    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return QDate();
    QSqlQuery lastTransactionQuery(db);
    lastTransactionQuery.prepare(QStringLiteral("SELECT OperationDate FROM Transactions ORDER BY OperationDate DESC LIMIT 1"));
    if (!lastTransactionQuery.exec()) {
#ifdef QT_DEBUG
        qDebug() << lastTransactionQuery.lastQuery() << lastTransactionQuery.lastError().text();
#endif
        return QDate();
    }
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

void MainObject::setTransactionsFilter(const QList<TransactionModelColumn> &col, const QStringList &filter)
{
    Q_ASSERT(col.size() == filter.size());
    QString filterString;
    for (int i = 0, maxI = col.size(); i < maxI; ++i) {
        if (i > 0)
            filterString += QStringLiteral(" AND ");
        filterString += m_transactionsModel->fieldName(col.at(i)) + filter.at(i);
    }
    m_transactionsModel->setFilter(filterString);
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
    for (OfflineSqliteTable *model :
         {m_transactionsModel, m_accountsModel, m_categoriesModel, m_currenciesModel, m_movementTypesModel, m_accountTypesModel, m_familyModel})
        model->setTable(model->tableName());
}

bool MainObject::addTransactions(int account, const QList<QDate> &opDt, const QList<int> &curr, const QList<double> &amount,
                                 const QList<QString> &payType, const QList<QString> &desc, const QList<int> &categ, const QList<int> &subcateg,
                                 const QList<int> &movementType, const QList<int> &destination, const QList<double> &exchangeRate)
{

    QSqlDatabase db = openDb();
    if (!db.isOpen())
        return false;
    if (!db.transaction())
        return false;
    if (account < 0 || opDt.isEmpty() || curr.isEmpty() || amount.isEmpty())
        return false;
    int newID = 0;
    for (int i = 0, maxI = m_transactionsModel->rowCount(); i < maxI; ++i)
        newID = std::max(newID, m_transactionsModel->index(i, tcId).data().toInt());
    auto maxI = opDt.size();
    if (maxI > 1 && curr.size() > 1 && curr.size() != maxI)
        return false;
    maxI = std::max(maxI, curr.size());
    if (maxI > 1 && amount.size() > 1 && amount.size() != maxI)
        return false;
    maxI = std::max(maxI, amount.size());
    if (maxI > 1 && payType.size() > 1 && payType.size() != maxI)
        return false;
    maxI = std::max(maxI, payType.size());
    if (maxI > 1 && desc.size() > 1 && desc.size() != maxI)
        return false;
    maxI = std::max(maxI, desc.size());
    if (maxI > 1 && categ.size() > 1 && categ.size() != maxI)
        return false;
    maxI = std::max(maxI, categ.size());
    if (maxI > 1 && subcateg.size() > 1 && subcateg.size() != maxI)
        return false;
    maxI = std::max(maxI, subcateg.size());
    if (maxI > 1 && movementType.size() > 1 && movementType.size() != maxI)
        return false;
    maxI = std::max(maxI, movementType.size());
    if (maxI > 1 && destination.size() > 1 && destination.size() != maxI)
        return false;
    maxI = std::max(maxI, destination.size());
    if (maxI > 1 && exchangeRate.size() > 1 && exchangeRate.size() != maxI)
        return false;
    maxI = std::max(maxI, exchangeRate.size());
    for (decltype(maxI) i = 0; i < maxI; ++i) {
        QSqlQuery addTransactionQuery(db);
        addTransactionQuery.prepare(
                QStringLiteral("INSERT INTO Transactions (Id, Account, OperationDate, Currency, Amount, PaymentType, Description, Category, "
                               "Subcategory, MovementType, DestinationAccount, ExchangeRate) VALUES (?,?,?,?,?,?,?,?,?,?,?,?)"));
        addTransactionQuery.addBindValue(++newID);
        addTransactionQuery.addBindValue(account);
        addTransactionQuery.addBindValue((opDt.size() > 1 ? opDt.at(i) : opDt.first()).toString(Qt::ISODate));
        addTransactionQuery.addBindValue(curr.size() > 1 ? curr.at(i) : curr.first());
        addTransactionQuery.addBindValue(amount.size() > 1 ? amount.at(i) : amount.first());
        if (payType.isEmpty())
            addTransactionQuery.addBindValue(QVariant(QMetaType::fromType<QString>()));
        else
            addTransactionQuery.addBindValue(payType.size() > 1 ? payType.at(i) : payType.first());
        if (desc.isEmpty())
            addTransactionQuery.addBindValue(QVariant(QMetaType::fromType<QString>()));
        else
            addTransactionQuery.addBindValue(desc.size() > 1 ? desc.at(i) : desc.first());
        if (categ.isEmpty())
            addTransactionQuery.addBindValue(QVariant(QMetaType::fromType<int>()));
        else
            addTransactionQuery.addBindValue(categ.size() > 1 ? categ.at(i) : categ.first());
        if (subcateg.isEmpty())
            addTransactionQuery.addBindValue(QVariant(QMetaType::fromType<int>()));
        else
            addTransactionQuery.addBindValue(subcateg.size() > 1 ? subcateg.at(i) : subcateg.first());
        if (movementType.isEmpty())
            addTransactionQuery.addBindValue(QVariant(QMetaType::fromType<int>()));
        else
            addTransactionQuery.addBindValue(movementType.size() > 1 ? movementType.at(i) : movementType.first());
        if (destination.isEmpty())
            addTransactionQuery.addBindValue(QVariant(QMetaType::fromType<int>()));
        else
            addTransactionQuery.addBindValue(destination.size() > 1 ? destination.at(i) : destination.first());
        if (exchangeRate.isEmpty())
            addTransactionQuery.addBindValue(QVariant(QMetaType::fromType<double>()));
        else
            addTransactionQuery.addBindValue(exchangeRate.size() > 1 ? exchangeRate.at(i) : movementType.first());
        if (!addTransactionQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << addTransactionQuery.lastQuery() << addTransactionQuery.lastError().text();
#endif
            Q_ASSUME(db.rollback());
            return false;
        }
    }
    if (!db.commit())
        return false;
    m_transactionsModel->select();
    setDirty(true);
    return true;
}
