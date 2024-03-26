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
#include "relationaldelegate.h"
#include <QAbstractItemModel>
#include <QComboBox>
RelationalDelegate::RelationalDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_relationModel(nullptr)
    , m_keyCol(0)
    , m_keyRole(Qt::DisplayRole)
    , m_relationCol(0)
    , m_relationRole(Qt::DisplayRole)
{ }

void RelationalDelegate::setRelationModel(QAbstractItemModel *model, int keyCol, int relationCol, int keyRole, int relationRole)
{
    m_relationModel = model;
    m_keyCol = keyCol;
    m_keyRole = keyRole;
    m_relationCol = relationCol;
    m_relationRole = relationRole;
}

QString RelationalDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (!m_relationModel || m_keyCol == m_relationCol)
        return QStyledItemDelegate::displayText(value, locale);
    for (int i = 0, maxI = m_relationModel->rowCount(); i < maxI; ++i) {
        if (m_relationModel->index(i, m_keyCol).data(m_keyRole) == value)
            return QStyledItemDelegate::displayText(m_relationModel->index(i, m_relationCol).data(m_relationRole), locale);
    }
    return QStyledItemDelegate::displayText(value, locale);
}

QWidget *RelationalDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!m_relationModel || m_keyCol == m_relationCol)
        return QStyledItemDelegate::createEditor(parent, option, index);
    QComboBox *result = new QComboBox(parent);
    result->setModel(m_relationModel);
    result->setModelColumn(m_relationCol);
    // TODO handle m_relationRole
    return result;
}

void RelationalDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!m_relationModel || m_keyCol == m_relationCol)
        return QStyledItemDelegate::setEditorData(editor, index);
    QComboBox *result = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(result);
    for (int i = 0, maxI = m_relationModel->rowCount(); i < maxI; ++i) {
        if (m_relationModel->index(i, m_keyCol).data(m_keyRole) == index.data())
            return result->setCurrentIndex(i);
    }
}

void RelationalDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!m_relationModel || m_keyCol == m_relationCol)
        return QStyledItemDelegate::setModelData(editor, model, index);
    QComboBox *result = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(result);
    model->setData(index, m_relationModel->index(result->currentIndex(), m_keyCol).data(m_keyRole));
}
