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
#include "ownerdelegate.h"
#include <mainobject.h>
#include <QStringList>
#include "multichoicecombo.h"
OwnerDelegate::OwnerDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_object(nullptr)
{ }

void OwnerDelegate::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
}

QString OwnerDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (!m_object)
        return QStyledItemDelegate::displayText(value, locale);
    QStringList resultList = value.toString().split(QLatin1Char(','));
    for (int i = 0, maxI = m_object->familyModel()->rowCount(); i < maxI; ++i) {
        const int idxToReplace = resultList.indexOf(QString::number(m_object->familyModel()->index(i, MainObject::fcId).data().toInt()));
        if (idxToReplace >= 0)
            resultList[idxToReplace] = m_object->familyModel()->index(i, MainObject::fcName).data().toString();
    }
    return resultList.join(tr(", "));
}

QWidget *OwnerDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!m_object)
        return QStyledItemDelegate::createEditor(parent, option, index);
    MultichoiceCombo *result = new MultichoiceCombo(parent);
    result->setModel(m_object->familyModel());
    result->setModelColumn(MainObject::fcName);
    return result;
}

void OwnerDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    MultichoiceCombo *comboEditor = qobject_cast<MultichoiceCombo *>(editor);
    if (!comboEditor || !m_object)
        return QStyledItemDelegate::setEditorData(editor, index);
    QList<int> indexesToCheck;
    QStringList resultList = index.data().toString().split(QLatin1Char(','));
    for (int i = 0, maxI = m_object->familyModel()->rowCount(); i < maxI; ++i) {
        if (resultList.contains(QString::number(m_object->familyModel()->index(i, MainObject::fcId).data().toInt())))
            indexesToCheck.append(i);
    }
    comboEditor->setCheckedIndexes(indexesToCheck);
}

void OwnerDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    MultichoiceCombo *comboEditor = qobject_cast<MultichoiceCombo *>(editor);
    if (!comboEditor || !m_object)
        return QStyledItemDelegate::setModelData(editor, model, index);
    const QList<int> selectedIndexes = comboEditor->checkedIndexes();
    QString result;
    for (int i : selectedIndexes)
        result += QString::number(m_object->familyModel()->index(i, MainObject::fcId).data().toInt()) + QLatin1Char(',');
    result.chop(1);
    model->setData(index, result);
}
