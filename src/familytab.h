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
#ifndef FAMILYTAB_H
#define FAMILYTAB_H

#include <QWidget>

namespace Ui {
class FamilyTab;
}
class MainObject;
class RelationalDelegate;
class DecimalDelegate;
class IsoDateDelegate;
class FamilyTab : public QWidget
{
    Q_OBJECT

public:
    explicit FamilyTab(QWidget *parent = nullptr);
    ~FamilyTab();
    void setMainObject(MainObject *mainObj);

private:
    void onAddFamily();
    void onRemoveFamily();
    MainObject *m_object;
    Ui::FamilyTab *ui;
    RelationalDelegate *m_currencyDelegate;
    DecimalDelegate *m_incomeDelegate;
    IsoDateDelegate *m_birthdayDelegate;
};

#endif // FAMILYTAB_H
