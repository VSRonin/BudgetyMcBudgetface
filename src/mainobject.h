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

#ifndef MAINOBJECT_H
#define MAINOBJECT_H
#include <QObject>
#include <QDate>
class OfflineSqlTable;
class QAbstractItemModel;
class QFile;
class MainObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainObject)
public:
    enum AccountsModelColumn { acId, acName, acOwner, acCurrency, acAccountType };
    enum TransactionModelColumn {
        tcId,
        tcAccount,
        tcOpDate,
        tcCurrency,
        tcAmount,
        tcPaymentType,
        tcDescription,
        tcCategory,
        tcSubcategory,
        tcMovementType,
        tcDestinationAccount,
        tcExchangeRate
    };
    enum CurrencyModelColumn { ccId, ccCurrency };
    enum AccountTypeModelColumn { atcId, atcName };
    enum ImportFormats { ifBarclays, ifNatwest, ifRevolut };
    enum FamilyModelColumn { fcId, fcName, fcBirthday, fcIncome, fcIncomeCurrency };

    explicit MainObject(QObject *parent = nullptr);
    ~MainObject();
    bool createBlankBudget();
    QAbstractItemModel *transactionsModel() const;
    QAbstractItemModel *accountsModel() const;
    QAbstractItemModel *categoriesModel() const;
    QAbstractItemModel *currenciesModel() const;
    QAbstractItemModel *accountTypesModel() const;
    QAbstractItemModel *familyModel() const;
    bool addFamilyMember(const QString &name, const QDate &birthday, double income, int incomeCurr);
    bool removeFamilyMembers(const QList<int> &ids);
    bool addAccount(const QString &name, const QString &owner, int curr, int typ);
    bool removeAccounts(const QList<int> &ids);
    bool addTransaction(int account, const QDate &opDt, int curr, double amount, const QString &payType, const QString &desc, int categ, int subcateg,
                        int movementType, int destination, double exchangeRate);
    bool removeTransactions(const QList<int> &ids);
    bool isDirty() const;
    bool saveBudget(const QString &path);
    bool loadBudget(const QString &path);
    bool importStatement(const QString &path, ImportFormats format);
    bool importBarclaysStatement(QFile *source);
    QDate lastTransactionDate() const;
    const QString &baseCurrency() const;
    bool setBaseCurrency(const QString &crncy);
    double exchangeRate(const QString &fromCrncy, const QString &toCrncy) const;
public slots:
    void newBudget();
signals:
    void dirtyChanged(bool dirty);
    void lastUpdateChanged();
    void baseCurrencyChanged();

private:
    void setDirty(bool dirty);
    void reselectModels();
    bool removeAccounts(const QList<int> &ids, bool transaction);
    OfflineSqlTable *m_transactionsModel;
    OfflineSqlTable *m_accountsModel;
    OfflineSqlTable *m_categoriesModel;
    OfflineSqlTable *m_currenciesModel;
    OfflineSqlTable *m_accountTypesModel;
    OfflineSqlTable *m_familyModel;
    bool m_dirty;
    QString m_baseCurrency;
};

#endif
