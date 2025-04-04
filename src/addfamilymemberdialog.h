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
#ifndef ADDFAMILYMEMBERDIALOG_H
#define ADDFAMILYMEMBERDIALOG_H

#include <QDialog>

namespace Ui {
class AddFamilyMemberDialog;
}
class MainObject;
class AddFamilyMemberDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFamilyMemberDialog(QWidget *parent = nullptr);
    ~AddFamilyMemberDialog();
    void setMainObject(MainObject *mainObj);
    QString name() const;
    QDate birthday() const;
    double annualIncome() const;
    int incomeCurrency() const;
    int retirementAge() const;

private:
    void checkOkEnabled();
    MainObject *m_object;
    Ui::AddFamilyMemberDialog *ui;
};

#endif // ADDFAMILYMEMBERDIALOG_H
