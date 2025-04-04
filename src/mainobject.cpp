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
#include <QQueue>
#include <QRegularExpression>
#ifdef QT_DEBUG
#include <QSortFilterProxyModel>
#    include <QSqlError>
#endif

class TransactionModel : public OfflineSqliteTable
{
    Q_DISABLE_COPY_MOVE(TransactionModel)
public:
    explicit TransactionModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int baseCurrency() const;
    void setBaseCurrency(int newBaseCurrency);

private:
    int m_baseCurrency;
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

MainObject::MainObject(QObject *parent)
    : QObject(parent)
    , m_transactionsModel(new TransactionModel(this))
    , m_accountsModel(new OfflineSqliteTable(this))
    , m_openAccountFilter(new QSortFilterProxyModel(this))
    , m_categoriesModel(new OfflineSqliteTable(this))
    , m_subcategoriesModel(new OfflineSqliteTable(this))
    , m_currenciesModel(new OfflineSqliteTable(this))
    , m_movementTypesModel(new OfflineSqliteTable(this))
    , m_accountTypesModel(new OfflineSqliteTable(this))
    , m_familyModel(new OfflineSqliteTable(this))
    , m_dirty(false)
    , m_baseCurrency(1)
{
    m_transactionsModel->setTable(QStringLiteral("Transactions"));
    m_transactionsModel->sort(tcOpDate, Qt::DescendingOrder);
    m_accountsModel->setTable(QStringLiteral("Accounts"));
    m_accountsModel->sort(acName);
    m_openAccountFilter->setSourceModel(m_accountsModel);
    m_openAccountFilter->setFilterKeyColumn(acAccountStatus);
    m_openAccountFilter->setFilterFixedString(QStringLiteral("1"));
    m_categoriesModel->setTable(QStringLiteral("Categories"));
    m_categoriesModel->sort(cacName);
    m_subcategoriesModel->setTable(QStringLiteral("Subcategories"));
    m_subcategoriesModel->sort(sccName);
    m_currenciesModel->setTable(QStringLiteral("Currencies"));
    m_movementTypesModel->setTable(QStringLiteral("MovementTypes"));
    m_accountTypesModel->setTable(QStringLiteral("AccountTypes"));
    m_familyModel->setTable(QStringLiteral("Family"));
    m_familyModel->sort(fcBirthday);
    connect(m_transactionsModel, &QAbstractItemModel::dataChanged, this, &MainObject::onTransactionCategoryChanged);
    connect(m_transactionsModel, &QAbstractItemModel::dataChanged, this, &MainObject::onTransactionCurrencyChanged);
    for (OfflineSqliteTable *model : {static_cast<OfflineSqliteTable *>(m_transactionsModel), m_accountsModel, m_categoriesModel,
                                      m_subcategoriesModel, m_currenciesModel, m_accountTypesModel, m_familyModel, m_movementTypesModel})
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

QAbstractItemModel *MainObject::filteredAccountsModel() const
{
    return m_openAccountFilter;
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

QAbstractItemModel *MainObject::subcategoriesModel() const
{
    return m_subcategoriesModel;
}

QAbstractItemModel *MainObject::movementTypesModel() const
{
    return m_movementTypesModel;
}

bool MainObject::addFamilyMember(const QString &name, const QDate &birthday, double income, int incomeCurr, int retirementAge)
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
    addFamilyMemberQuery.prepare(QStringLiteral("INSERT INTO Family (Id, Name, Birthday, TaxableIncome, IncomeCurrency, RetirementAge) VALUES (?,?,?,?,?,?)"));
    addFamilyMemberQuery.addBindValue(++newID);
    addFamilyMemberQuery.addBindValue(name);
    addFamilyMemberQuery.addBindValue(birthday.toString(Qt::ISODate));
    addFamilyMemberQuery.addBindValue(income);
    addFamilyMemberQuery.addBindValue(incomeCurr);
    addFamilyMemberQuery.addBindValue(retirementAge);
    if (!addFamilyMemberQuery.exec()) {
#ifdef QT_DEBUG
        qDebug() << addFamilyMemberQuery.executedQuery() << addFamilyMemberQuery.lastError().text();
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
            qDebug() << impactedAccountsQuery.executedQuery() << impactedAccountsQuery.lastError().text();
#endif
            CHECK_TRUE(db.rollback());
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
            qDebug() << updateAccountQuery.executedQuery() << updateAccountQuery.lastError().text();
#endif
            CHECK_TRUE(db.rollback());
            return false;
        }
    }
    if (!accountsToRemove.isEmpty()) {
        if (!removeAccounts(accountsToRemove, false)) {
            CHECK_TRUE(db.rollback());
            return false;
        }
    }
    {
        QSqlQuery removeFamilyQuery(db);
        removeFamilyQuery.prepare(QStringLiteral("DELETE FROM Family WHERE Id IN (") + filterString + QLatin1Char(')'));
        if (!removeFamilyQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << removeFamilyQuery.executedQuery() << removeFamilyQuery.lastError().text();
#endif
            CHECK_TRUE(db.rollback());
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
        qDebug() << addAccountQuery.executedQuery() << addAccountQuery.lastError().text();
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
            qDebug() << removeTransactionsQuery.executedQuery() << removeTransactionsQuery.lastError().text();
#endif
            CHECK_TRUE(db.rollback());
            return false;
        }
    }
    {
        QSqlQuery removeAccountQuery(db);
        removeAccountQuery.prepare(QStringLiteral("DELETE FROM Accounts WHERE Id IN (") + filterString + QLatin1Char(')'));
        if (!removeAccountQuery.exec()) {
#ifdef QT_DEBUG
            qDebug() << removeAccountQuery.executedQuery() << removeAccountQuery.lastError().text();
#endif
            CHECK_TRUE(db.rollback());
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

void MainObject::onTransactionCategoryChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.column() > tcCategory || bottomRight.column() < tcCategory)
        return;
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        const QVariant catData = topLeft.sibling(i, tcCategory).data();
        if (!catData.isValid() || !isInternalTransferCategory(catData.toInt())) {
            if (topLeft.sibling(i, tcDestinationAccount).data().isValid())
                m_transactionsModel->setData(topLeft.sibling(i, tcDestinationAccount), QVariant());
        } else if (isInternalTransferCategory(catData.toInt())) {
            m_transactionsModel->setData(topLeft.sibling(i, tcMovementType),
                                         movementTypeForInternalTransfer(catData.toInt(), topLeft.sibling(i, tcAmount).data().toDouble()));
        }
        const int forcedSub = forcedSubcategory(catData.toInt());
        if (forcedSub >= 0)
            m_transactionsModel->setData(topLeft.sibling(i, tcSubcategory), forcedSub);
        else if (topLeft.sibling(i, tcSubcategory).data().isValid()
                 && (!catData.isValid() || !validSubcategory(catData.toInt(), topLeft.sibling(i, tcSubcategory).data().toInt())))
            m_transactionsModel->setData(topLeft.sibling(i, tcSubcategory), QVariant());
    }
}

double MainObject::getExchangeRate(int fromCurrencyID, int toCurrencyID, double defaultVal) const{
    QSqlDatabase db = openDb();
    if (db.isOpen()) {
        QSqlQuery getExchangeRateQuery(db);
        getExchangeRateQuery.prepare(
                QStringLiteral("SELECT Intermediate.ExchangeRate, Intermediate.FromCurrencyID, Currencies.Id as ToCurrencyID FROM (SELECT "
                               "Currencies.Id as FromCurrencyID, ExchangeRates.ToCurrency, ExchangeRates.ExchangeRate from ExchangeRates LEFT "
                               "JOIN Currencies ON Currencies.Currency=ExchangeRates.FromCurrency) as Intermediate LEFT JOIN Currencies ON "
                               "Currencies.Currency=Intermediate.ToCurrency WHERE FromCurrencyID =? AND ToCurrencyID=?"));
        getExchangeRateQuery.addBindValue(fromCurrencyID);
        getExchangeRateQuery.addBindValue(toCurrencyID);
        if (getExchangeRateQuery.exec()) {
            if (getExchangeRateQuery.next())
                return getExchangeRateQuery.value(0).toDouble();
        }
    }
    return defaultVal;
}

void MainObject::onTransactionCurrencyChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.column() > tcCurrency || bottomRight.column() < tcCurrency)
        return;
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        const QVariant curData = topLeft.sibling(i, tcCurrency).data();
        if (curData.toInt() == m_baseCurrency)
            m_transactionsModel->setData(topLeft.sibling(i, tcExchangeRate), QVariant());
        else
            m_transactionsModel->setData(topLeft.sibling(i, tcExchangeRate), getExchangeRate(curData.toInt(),m_baseCurrency));
    }
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
        qDebug() << removeTransactionsQuery.executedQuery() << removeTransactionsQuery.lastError().text();
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
                           QList<double>(), true);
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
        qDebug() << lastTransactionQuery.executedQuery() << lastTransactionQuery.lastError().text();
#endif
        return QDate();
    }
    if (!lastTransactionQuery.next())
        return QDate();
    return QDate::fromString(lastTransactionQuery.value(0).toString(), Qt::ISODate);
}

int MainObject::baseCurrency() const
{
    return m_baseCurrency;
}

bool MainObject::setBaseCurrency(int crncy)
{
    if (m_baseCurrency == crncy)
        return true;
    for (int i = 0, maxI = m_currenciesModel->rowCount(); i < maxI; ++i) {
        if (crncy == m_currenciesModel->index(i, ccId).data().toInt()) {
            m_baseCurrency = crncy;
            m_transactionsModel->setBaseCurrency(crncy);
            baseCurrencyChanged();
            return true;
        }
    }
    return false;
}

bool MainObject::setBaseCurrency(const QString &crncy)
{
    return setBaseCurrency(idForCurrency(crncy));
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

bool MainObject::validSubcategory(int category, int subcategory) const
{
    QSqlDatabase db = openDb();
    if (db.isOpen()) {
        QSqlQuery subcategoryQuery(db);
        subcategoryQuery.prepare(QStringLiteral("SELECT Id from Subcategories WHERE Id=? AND Category=?"));
        subcategoryQuery.addBindValue(subcategory);
        subcategoryQuery.addBindValue(category);
        if (subcategoryQuery.exec())
            return subcategoryQuery.next();
    }
    // backup
    for (int i = 0, maxI = m_subcategoriesModel->rowCount(); i < maxI; ++i) {
        if (m_subcategoriesModel->index(i, sccCategoryId).data().toInt() == category
            && m_subcategoriesModel->index(i, sccId).data().toInt() == subcategory)
            return true;
    }
    return false;
}

constexpr bool MainObject::isInternalTransferCategory(int category)
{
    return category == 0 // internal transfer
            || category == 18 // investment
            || category == 19 // debt
            ;
}

int MainObject::forcedSubcategory(int category) const
{
    int result = -1;
    QSqlDatabase db = openDb();
    if (db.isOpen()) {
        QSqlQuery subcategoryQuery(db);
        subcategoryQuery.prepare(QStringLiteral("SELECT Id from Subcategories WHERE Category=?"));
        subcategoryQuery.addBindValue(category);
        if (subcategoryQuery.exec()) {
            if (subcategoryQuery.next()) {
                result = subcategoryQuery.value(0).toInt();
                if (subcategoryQuery.next())
                    return -1;
                return result;
            }
        }
    }
    // backup
    for (int i = 0, maxI = m_subcategoriesModel->rowCount(); i < maxI; ++i) {
        if (m_subcategoriesModel->index(i, sccCategoryId).data().toInt() == category) {
            if (result >= 0)
                return -1;
            result = m_subcategoriesModel->index(i, sccId).data().toInt();
        }
    }
    return result;
}

int MainObject::movementTypeForInternalTransfer(int category, double amount) const
{
    Q_ASSERT(isInternalTransferCategory(category));
    if (amount > 0)
        return 6; // Withdrawal
    if (category == 0 || category == 18) // internal transfer
        return 4; // Deposit
    if (category == 19) // debt
        return 5; // Repayment
    Q_UNREACHABLE();
    return 0;
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
    for (OfflineSqliteTable *model : {static_cast<OfflineSqliteTable *>(m_transactionsModel), m_accountsModel, m_categoriesModel,
                                      m_subcategoriesModel, m_currenciesModel, m_movementTypesModel, m_accountTypesModel, m_familyModel})
        model->setTable(model->tableName());
}

bool MainObject::addTransactions(int account, const QList<QDate> &opDt, const QList<int> &curr, const QList<double> &amount,
                                 const QList<QString> &payType, const QList<QString> &desc, const QList<int> &categ, const QList<int> &subcateg,
                                 const QList<int> &movementType, const QList<int> &destination, const QList<double> &exchangeRate,
                                 bool checkDuplicates)
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
    QQueue<int> iToSkip;
    if (checkDuplicates) {
        for (decltype(maxI) i = 0; i < maxI; ++i) {
            QSqlQuery duplicateQuery(db);
            duplicateQuery.prepare(QStringLiteral("SELECT Id FROM Transactions WHERE Account=? AND OperationDate=? AND Currency=? AND Amount=? AND "
                                                  "PaymentType=? AND Description=?"));
            duplicateQuery.addBindValue(account);
            duplicateQuery.addBindValue((opDt.size() > 1 ? opDt.at(i) : opDt.first()).toString(Qt::ISODate));
            duplicateQuery.addBindValue(curr.size() > 1 ? curr.at(i) : curr.first());
            duplicateQuery.addBindValue(amount.size() > 1 ? amount.at(i) : amount.first());
            if (payType.isEmpty())
                duplicateQuery.addBindValue(QVariant(QMetaType::fromType<QString>()));
            else
                duplicateQuery.addBindValue(payType.size() > 1 ? payType.at(i) : payType.first());
            if (desc.isEmpty())
                duplicateQuery.addBindValue(QVariant(QMetaType::fromType<QString>()));
            else
                duplicateQuery.addBindValue(desc.size() > 1 ? desc.at(i) : desc.first());
            if (!duplicateQuery.exec()) {
#ifdef QT_DEBUG
                qDebug() << duplicateQuery.executedQuery() << duplicateQuery.lastError().text();
#endif
                CHECK_TRUE(db.rollback());
                return false;
            }
            if (duplicateQuery.next())
                iToSkip.enqueue(i);
        }
    }
    const int duplicateSkipped = iToSkip.size();
    for (decltype(maxI) i = 0; i < maxI; ++i) {
        if (!iToSkip.isEmpty()) {
            if (iToSkip.head() == i) {
                iToSkip.dequeue();
                continue;
            }
        }
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
            qDebug() << addTransactionQuery.executedQuery() << addTransactionQuery.lastError().text();
#endif
            CHECK_TRUE(db.rollback());
            return false;
        }
    }
    if (!db.commit())
        return false;
    m_transactionsModel->select();
    if (duplicateSkipped > 0)
        Q_EMIT addTransactionSkippedDuplicates(duplicateSkipped);
    setDirty(true);
    return true;
}

TransactionModel::TransactionModel(QObject *parent)
    : OfflineSqliteTable(parent)
    , m_baseCurrency(1)
{
    connect(this, &TransactionModel::dataChanged, this, &TransactionModel::onDataChanged);
}

Qt::ItemFlags TransactionModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
    case MainObject::tcSubcategory:
        if (index.sibling(index.row(), MainObject::tcCategory).data().isValid())
            return OfflineSqliteTable::flags(index);
        return Qt::ItemNeverHasChildren;
    case MainObject::tcDestinationAccount: {
        const QVariant catData = index.sibling(index.row(), MainObject::tcCategory).data();
        if (catData.isValid() && MainObject::isInternalTransferCategory(catData.toInt()))
            return OfflineSqliteTable::flags(index);
        return Qt::ItemNeverHasChildren;
    }
    case MainObject::tcExchangeRate:
        if (index.sibling(index.row(), MainObject::tcCurrency).data().toInt() != m_baseCurrency)
            return OfflineSqliteTable::flags(index);
        return Qt::ItemNeverHasChildren;
    default:
        return OfflineSqliteTable::flags(index);
    }
}

int TransactionModel::baseCurrency() const
{
    return m_baseCurrency;
}

void TransactionModel::setBaseCurrency(int newBaseCurrency)
{
    if (m_baseCurrency == newBaseCurrency)
        return;
    m_baseCurrency = newBaseCurrency;
    if (rowCount() > 0 && columnCount() > 0)
        Q_EMIT dataChanged(index(0, MainObject::tcExchangeRate), index(rowCount() - 1, MainObject::tcExchangeRate), {Qt::DisplayRole, Qt::EditRole});
}

void TransactionModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.column() <= MainObject::tcCurrency && bottomRight.column() >= MainObject::tcCurrency) {
        Q_EMIT dataChanged(index(topLeft.row(), MainObject::tcExchangeRate), index(bottomRight.row(), MainObject::tcExchangeRate));
    }
    if (topLeft.column() <= MainObject::tcCategory && bottomRight.column() >= MainObject::tcCategory) {
        Q_EMIT dataChanged(index(topLeft.row(), MainObject::tcDestinationAccount), index(bottomRight.row(), MainObject::tcDestinationAccount));
        Q_EMIT dataChanged(index(topLeft.row(), MainObject::tcSubcategory), index(bottomRight.row(), MainObject::tcSubcategory));
    }
}
