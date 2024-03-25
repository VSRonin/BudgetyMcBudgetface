/****************************************************************************\
   Copyright 2021 Luca Beldi
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
class OfflineSqlTable;
class QAbstractItemModel;
class QStandardItemModel;
class MainObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainObject)
public:
    explicit MainObject(QObject *parent = nullptr);
    ~MainObject();
    bool createBlankBudget();
    QAbstractItemModel *transactionsModel() const;
    QAbstractItemModel *accountsModel() const;
    QAbstractItemModel *categoriesModel() const;
    QAbstractItemModel *currenciesModel() const;
    QAbstractItemModel *accountTypesModel() const;
    bool addAccount(const QString &name, const QString &owner, int curr, int typ);
    bool isDirty() const;
public slots:
    void newBudget();
    bool saveBudget(const QString &path);
    bool loadBudget(const QString &path);
signals:
    void dirtyChanged(bool dirty);

private:
    void setDirty(bool dirty);
    void reselectModels();
    OfflineSqlTable *m_transactionsModel;
    OfflineSqlTable *m_accountsModel;
    OfflineSqlTable *m_categoriesModel;
    QStandardItemModel *m_currenciesModel;
    QStandardItemModel *m_accountTypesModel;
    bool m_dirty;
};

#endif
