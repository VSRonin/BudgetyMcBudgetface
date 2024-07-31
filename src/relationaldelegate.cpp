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
#include <QSortFilterProxyModel>
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
    result->setEditable(true);
    result->setInsertPolicy(QComboBox::NoInsert);
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

FilteredRelationalDelegate::FilteredRelationalDelegate(QObject *parent)
    : RelationalDelegate(parent)
    , m_filterKeyColumn(-1)
    , m_filterKeyRole(Qt::DisplayRole)
    , m_relationFilterColumn(-1)
    , m_relationFilterRole(Qt::DisplayRole)
    , m_relationFilterProxy(new QSortFilterProxyModel(this))
{ }
void FilteredRelationalDelegate::setRelationModel(QAbstractItemModel *model, int keyCol, int relationCol, int keyRole, int relationRole)
{
    RelationalDelegate::setRelationModel(model, keyCol, relationCol, keyRole, relationRole);
    m_relationFilterProxy->setSourceModel(model);
}

QWidget *FilteredRelationalDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    QWidget *result = RelationalDelegate::createEditor(parent, option, index);
    if (m_filterKeyColumn < 0 || m_relationFilterColumn < 0)
        return result;
    QComboBox *resultCombo = qobject_cast<QComboBox *>(result);
    if (!resultCombo)
        return result;
    resultCombo->setModel(m_relationFilterProxy);
    // TODO handle m_relationRole
    return resultCombo;
}

void FilteredRelationalDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (m_filterKeyColumn >= 0 || m_relationFilterColumn >= 0) {
        m_relationFilterProxy->setFilterKeyColumn(m_relationFilterColumn);
        m_relationFilterProxy->setFilterRole(m_relationFilterRole);
        m_relationFilterProxy->setFilterRegularExpression(
                QLatin1Char('^') + index.sibling(index.row(), m_filterKeyColumn).data(m_filterKeyRole).toString() + QLatin1Char('$'));
    }
    RelationalDelegate::setEditorData(editor, index);
}

void FilteredRelationalDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!m_relationModel || m_keyCol == m_relationCol)
        return QStyledItemDelegate::setModelData(editor, model, index);
    if (m_filterKeyColumn < 0 || m_relationFilterColumn < 0)
        return RelationalDelegate::setModelData(editor, model, index);
    QComboBox *result = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(result);
    model->setData(index, m_relationFilterProxy->index(result->currentIndex(), m_keyCol).data(m_keyRole));
}

int FilteredRelationalDelegate::relationFilterColumn() const
{
    return m_relationFilterColumn;
}

void FilteredRelationalDelegate::setRelationFilterColumn(int col)
{
    m_relationFilterColumn = col;
    m_relationFilterProxy->setFilterKeyColumn(m_relationFilterColumn);
}

int FilteredRelationalDelegate::filterKeyColumn() const
{
    return m_filterKeyColumn;
}

void FilteredRelationalDelegate::setFilterKeyColumn(int col)
{
    m_filterKeyColumn = col;
}

int FilteredRelationalDelegate::filterKeyRole() const
{
    return m_filterKeyRole;
}

void FilteredRelationalDelegate::setFilterKeyRole(int role)
{
    m_filterKeyRole = role;
}

int FilteredRelationalDelegate::relationFilterRole() const
{
    return m_relationFilterRole;
}

void FilteredRelationalDelegate::setRelationFilterRole(int role)
{
    m_relationFilterRole = role;
    m_relationFilterProxy->setFilterRole(m_relationFilterRole);
}
