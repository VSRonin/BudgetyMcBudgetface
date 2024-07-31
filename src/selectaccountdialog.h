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
#ifndef SELECTACCOUNTDIALOG_H
#define SELECTACCOUNTDIALOG_H

#include <QDialog>

namespace Ui {
class SelectAccountDialog;
}
class MainObject;
class SelectAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectAccountDialog(QWidget *parent = nullptr);
    ~SelectAccountDialog();
    void setMainObject(MainObject *mainObj);
    int selectedAccountId() const;

private:
    MainObject *m_object;
    Ui::SelectAccountDialog *ui;
};

#endif // SELECTACCOUNTDIALOG_H
