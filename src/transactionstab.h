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
#ifndef TRANSACTIONSTAB_H
#define TRANSACTIONSTAB_H

#include <QWidget>
#include <mainobject.h>
namespace Ui {
class TransactionsTab;
}
class RelationalDelegate;
class FilteredRelationalDelegate;
class OrFilterProxy;
class BlankRowProxy;
class DecimalDelegate;
class IsoDateDelegate;
class QSortFilterProxyModel;
class TransactionsTab : public QWidget
{
    Q_OBJECT

public:
    explicit TransactionsTab(QWidget *parent = nullptr);
    ~TransactionsTab();
    void setMainObject(MainObject *mainObj);

private:
    void onShowWIPChanged();
    void onFilterChanged();
    void importStatement(MainObject::ImportFormats format);
    void onRemoveTransactions();
    void refreshLastUpdate();
    MainObject *m_object;
    OrFilterProxy *m_filterProxy;
    RelationalDelegate *m_currencyDelegate;
    RelationalDelegate *m_accountDelegate;
    RelationalDelegate *m_categoryDelegate;
    FilteredRelationalDelegate *m_subcategoryDelegate;
    RelationalDelegate *m_movementTypeDelegate;
    DecimalDelegate *m_amountDelegate;
    IsoDateDelegate *m_opDateDelegate;
    BlankRowProxy *m_currencyProxy;
    BlankRowProxy *m_accountProxy;

    Ui::TransactionsTab *ui;
};

#endif // TRANSACTIONSTAB_H
