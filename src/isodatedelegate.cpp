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
#include "isodatedelegate.h"
#include <QDate>
#include <QDateEdit>
IsoDateDelegate::IsoDateDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{ }

QString IsoDateDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    const QDate result = QDate::fromString(value.toString(), Qt::ISODate);
    if (result.isValid())
        return QStyledItemDelegate::displayText(result, locale);
    return QStyledItemDelegate::displayText(value, locale);
}

QWidget *IsoDateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return new QDateEdit(parent);
}

void IsoDateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDateEdit *dateEditor = qobject_cast<QDateEdit *>(editor);
    if (dateEditor)
        dateEditor->setDate(QDate::fromString(index.data().toString(), Qt::ISODate));
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

void IsoDateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDateEdit *dateEditor = qobject_cast<QDateEdit *>(editor);
    if (dateEditor)
        model->setData(index, dateEditor->date().toString(Qt::ISODate));
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}
