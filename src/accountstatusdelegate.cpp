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
#include "accountstatusdelegate.h"
#include <QComboBox>
AccountStatusDelegate::AccountStatusDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QString AccountStatusDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if(value.toInt()==1)
        return tr("Open");
    return tr("Closed");
}

QWidget *AccountStatusDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QComboBox *result = new QComboBox(parent);
    result->addItem(tr("Open"),1);
    result->addItem(tr("Closed"),0);
    return result;
}

void AccountStatusDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *result = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(result);
    result->setCurrentIndex(result->findData(index.data().toInt()));
}

void AccountStatusDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *result = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(result);
    model->setData(index,result->currentData().toInt());
}
