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
#ifndef ACCOUNTSTAB_H
#define ACCOUNTSTAB_H

#include <QWidget>

namespace Ui {
class AccountsTab;
}
class RelationalDelegate;
class MainObject;
class OwnerDelegate;
class AccountsTab : public QWidget
{
    Q_OBJECT

public:
    explicit AccountsTab(QWidget *parent = nullptr);
    ~AccountsTab();
    void setMainObject(MainObject *mainObj);

private:
    void onAddAccount();
    void onRemoveAccount();
    MainObject *m_object;
    Ui::AccountsTab *ui;
    RelationalDelegate *m_currencyDelegate;
    RelationalDelegate *m_accountTypeDelagate;
    OwnerDelegate *m_ownerDelegate;
};

#endif // ACCOUNTSTAB_H
