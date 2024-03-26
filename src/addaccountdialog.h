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
#ifndef ADDACCOUNTDIALOG_H
#define ADDACCOUNTDIALOG_H

#include <QDialog>
class QAbstractItemModel;
namespace Ui {
class AddAccountDialog;
}

class AddAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAccountDialog(QWidget *parent = nullptr);
    ~AddAccountDialog();
    QString name() const;
    QString owner() const;
    int curr() const;
    int typ() const;
    void loadCombos(QAbstractItemModel *currenciesModel, QAbstractItemModel *accountTypesModel, int currModelKeyCol, int currModelValCol,
                    int AccTypModelKeyCol, int AccTypModelValCol);

private:
    int m_currKeyCol;
    int m_accTypKeyCol;
    Ui::AddAccountDialog *ui;
};

#endif // ADDACCOUNTDIALOG_H
