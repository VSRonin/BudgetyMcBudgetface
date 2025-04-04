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
#include "blankrowproxy.h"
#include "globals.h"
#include <QSize>
BlankRowProxy::BlankRowProxy(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_blankRows(1)
{ }

QModelIndex BlankRowProxy::buddy(const QModelIndex &index) const
{
    return index;
}

bool BlankRowProxy::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    if (!sourceModel())
        return false;
    if (!parent.isValid() && row < m_blankRows)
        return false;
    return sourceModel()->canDropMimeData(data, action, parent.isValid() ? row : (row - m_blankRows), column, parent);
}

bool BlankRowProxy::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.isValid() && row < m_blankRows)
        return false;
    return sourceModel()->dropMimeData(data, action, parent.isValid() ? row : (row - m_blankRows), column, parent);
}

bool BlankRowProxy::canFetchMore(const QModelIndex &parent) const
{
    if (!sourceModel())
        return false;
    return sourceModel()->canFetchMore(mapToSource(parent));
}

void BlankRowProxy::fetchMore(const QModelIndex &parent)
{
    if (!sourceModel())
        return;
    sourceModel()->fetchMore(mapToSource(parent));
}

bool BlankRowProxy::clearItemData(const QModelIndex &index)
{
    if (!sourceModel())
        return false;
    if (!index.parent().isValid() && index.row() < m_blankRows)
        return false;
    return sourceModel()->clearItemData(mapToSource(index));
}

QVariant BlankRowProxy::data(const QModelIndex &proxyIndex, int role) const
{
    if (!sourceModel())
        return QVariant();
    if (!proxyIndex.parent().isValid() && proxyIndex.row() < m_blankRows)
        return dataForBlankRow(proxyIndex.row(), proxyIndex.column(), role);
    return sourceModel()->data(mapToSource(proxyIndex), role);
}

void BlankRowProxy::multiData(const QModelIndex &index, QModelRoleDataSpan roleDataSpan) const
{
    if (!sourceModel() || !index.isValid() || (!index.parent().isValid() && index.row() < m_blankRows)) {
        for (QModelRoleData &roleData : roleDataSpan)
            roleData.setData(QVariant());
        return;
    }
    return sourceModel()->multiData(mapToSource(index), roleDataSpan);
}

bool BlankRowProxy::hasChildren(const QModelIndex &parent) const
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() < m_blankRows)
        return false;
    return sourceModel()->hasChildren(mapToSource(parent));
}

QVariant BlankRowProxy::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!sourceModel())
        return QVariant();
    return sourceModel()->headerData(orientation == Qt::Vertical ? (section - m_blankRows) : section, orientation, role);
}

QMap<int, QVariant> BlankRowProxy::itemData(const QModelIndex &proxyIndex) const
{
    if (!sourceModel())
        return QMap<int, QVariant>();
    if (!proxyIndex.parent().isValid() && proxyIndex.row() < m_blankRows)
        return QMap<int, QVariant>();
    return sourceModel()->itemData(mapToSource(proxyIndex));
}

bool BlankRowProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!sourceModel())
        return false;
    if (!index.parent().isValid() && index.row() < m_blankRows)
        return false;
    return sourceModel()->setData(mapToSource(index), value, role);
}

bool BlankRowProxy::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (!sourceModel())
        return false;
    if (section < m_blankRows && orientation == Qt::Vertical)
        return false;
    return sourceModel()->setHeaderData(orientation == Qt::Vertical ? (section - m_blankRows) : section, orientation, value, role);
}

bool BlankRowProxy::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    if (!sourceModel())
        return false;
    if (!index.parent().isValid() && index.row() < m_blankRows)
        return false;
    return sourceModel()->setItemData(mapToSource(index), roles);
}

int BlankRowProxy::rowCount(const QModelIndex &parent) const
{
    if (!sourceModel())
        return 0;
    return sourceModel()->rowCount(mapToSource(parent)) + (parent.isValid() ? 0 : m_blankRows);
}

int BlankRowProxy::columnCount(const QModelIndex &parent) const
{
    if (!sourceModel())
        return 0;
    return sourceModel()->columnCount(mapToSource(parent));
}

QModelIndex BlankRowProxy::index(int row, int column, const QModelIndex &parent) const
{
    if (!sourceModel())
        return QModelIndex();
    if (!parent.isValid())
        return createIndex(row, column);
    return mapFromSource(sourceModel()->index(row, column, mapToSource(parent)));
}

QModelIndex BlankRowProxy::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid())
        return QModelIndex();
    Q_ASSERT(sourceIndex.model() == sourceModel());
    if (sourceIndex.parent().isValid())
        return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
    return createIndex(sourceIndex.row() + m_blankRows, sourceIndex.column());
}

QModelIndex BlankRowProxy::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
        return QModelIndex();
    Q_ASSERT(proxyIndex.model() == this);
    if (proxyIndex.internalPointer())
        return createSourceIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());
    if (proxyIndex.row() < m_blankRows)
        return QModelIndex();
    return sourceModel()->index(proxyIndex.row() - m_blankRows, proxyIndex.column());
}

void BlankRowProxy::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel())
        return;
    beginResetModel();
    for (const auto &conn : std::as_const(m_sourceConnections))
        QObject::disconnect(conn);
    m_sourceConnections.clear();
    QAbstractProxyModel::setSourceModel(model);
    if (model) {
        m_sourceConnections << connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &BlankRowProxy::beginResetModel)
                            << connect(model, &QAbstractItemModel::modelReset, this, &BlankRowProxy::endResetModel)
                            << connect(model, &QAbstractItemModel::dataChanged, this, &BlankRowProxy::onDataChanged)
                            << connect(model, &QAbstractItemModel::headerDataChanged, this, &BlankRowProxy::onHeaderDataChanged)
                            << connect(model, &QAbstractItemModel::columnsAboutToBeInserted, this, &BlankRowProxy::onColumnsAboutToBeInserted)
                            << connect(model, &QAbstractItemModel::columnsAboutToBeMoved, this, &BlankRowProxy::onColumnsAboutToBeMoved)
                            << connect(model, &QAbstractItemModel::columnsAboutToBeRemoved, this, &BlankRowProxy::onColumnsAboutToBeRemoved)
                            << connect(model, &QAbstractItemModel::columnsInserted, this, &BlankRowProxy::onColumnsInserted)
                            << connect(model, &QAbstractItemModel::columnsMoved, this, &BlankRowProxy::onColumnsMoved)
                            << connect(model, &QAbstractItemModel::columnsRemoved, this, &BlankRowProxy::onColumnsRemoved)
                            << connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &BlankRowProxy::onRowsAboutToBeInserted)
                            << connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, &BlankRowProxy::onRowsAboutToBeMoved)
                            << connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &BlankRowProxy::onRowsAboutToBeRemoved)
                            << connect(model, &QAbstractItemModel::rowsInserted, this, &BlankRowProxy::onRowsInserted)
                            << connect(model, &QAbstractItemModel::rowsMoved, this, &BlankRowProxy::onRowsMoved)
                            << connect(model, &QAbstractItemModel::rowsRemoved, this, &BlankRowProxy::onRowsRemoved)
                            << connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, &BlankRowProxy::onLayoutAboutToBeChanged)
                            << connect(model, &QAbstractItemModel::layoutChanged, this, &BlankRowProxy::onLayoutChanged);
    }
    endResetModel();
}

Qt::ItemFlags BlankRowProxy::flags(const QModelIndex &index) const
{
    if (!sourceModel())
        return Qt::NoItemFlags;
    if (index.parent().isValid() || index.row() >= m_blankRows)
        return sourceModel()->flags(mapToSource(index));
    return flagsForBlankRow(index.row(), index.column());
}

QModelIndex BlankRowProxy::parent(const QModelIndex &index) const
{
    if (!sourceModel())
        return QModelIndex();
    return mapFromSource(sourceModel()->parent(mapToSource(index)));
}

bool BlankRowProxy::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() < m_blankRows)
        return false;
    return sourceModel()->insertColumns(column, count, mapToSource(parent));
}

bool BlankRowProxy::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() < m_blankRows)
        return false;
    return sourceModel()->insertRows(parent.isValid() ? row : std::max(0, row - m_blankRows), count, mapToSource(parent));
}

bool BlankRowProxy::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() < m_blankRows)
        return false;
    return sourceModel()->removeColumns(column, count, mapToSource(parent));
}

bool BlankRowProxy::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() < m_blankRows)
        return false;
    if (!parent.isValid() && row < m_blankRows)
        return false;
    return sourceModel()->removeRows(parent.isValid() ? row : (row - m_blankRows), count, mapToSource(parent));
}

bool BlankRowProxy::moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent,
                                int destinationChild)
{
    if (!sourceModel())
        return false;
    if (!sourceParent.parent().isValid() && sourceParent.row() < m_blankRows)
        return false;
    if (!destinationParent.parent().isValid() && destinationParent.row() < m_blankRows)
        return false;
    return sourceModel()->moveColumns(mapToSource(sourceParent), sourceColumn, count, mapToSource(destinationParent), destinationChild);
}

bool BlankRowProxy::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (!sourceModel())
        return false;
    if (!sourceParent.parent().isValid() && sourceParent.row() < m_blankRows)
        return false;
    if (!destinationParent.parent().isValid() && destinationParent.row() < m_blankRows)
        return false;
    if (!sourceParent.isValid() && sourceRow < m_blankRows)
        return false;
    return sourceModel()->moveRows(mapToSource(sourceParent), sourceParent.isValid() ? sourceRow : (sourceRow - m_blankRows), count,
                                   mapToSource(destinationParent),
                                   destinationParent.isValid() ? destinationChild : std::max(0, destinationChild - m_blankRows));
}

QModelIndexList BlankRowProxy::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    if (!sourceModel())
        return QModelIndexList();
    if (start.isValid() && !start.parent().isValid() && start.row() < m_blankRows)
        return sourceModel()->match(sourceModel()->index(0, start.column()), role, value, hits, flags);
    return sourceModel()->match(start, role, value, hits, flags);
}

QMimeData *BlankRowProxy::mimeData(const QModelIndexList &indexes) const
{
    if (!sourceModel())
        return nullptr;
    QModelIndexList mappedIndexes;
    for (auto &idx : indexes) {
        const auto mappedIdx = mapToSource(idx);
        if (mappedIdx.isValid() || !idx.isValid())
            mappedIndexes.append(mappedIdx);
    }
    return sourceModel()->mimeData(mappedIndexes);
}

QStringList BlankRowProxy::mimeTypes() const
{
    if (!sourceModel())
        return QStringList();
    return sourceModel()->mimeTypes();
}

QHash<int, QByteArray> BlankRowProxy::roleNames() const
{
    if (!sourceModel())
        return QHash<int, QByteArray>();
    return sourceModel()->roleNames();
}

QModelIndex BlankRowProxy::sibling(int row, int column, const QModelIndex &index) const
{
    return this->index(row, column, index.parent());
}

void BlankRowProxy::sort(int column, Qt::SortOrder order)
{
    if (sourceModel())
        sourceModel()->sort(column, order);
}

QSize BlankRowProxy::span(const QModelIndex &index) const
{
    if (!sourceModel() || !index.isValid())
        return QSize(1, 1);
    if (!index.parent().isValid() && index.row() < m_blankRows)
        return QSize(1, 1);
    return sourceModel()->span(mapToSource(index));
}

Qt::DropActions BlankRowProxy::supportedDragActions() const
{
    if (!sourceModel())
        return Qt::IgnoreAction;
    return sourceModel()->supportedDragActions();
}

Qt::DropActions BlankRowProxy::supportedDropActions() const
{
    if (!sourceModel())
        return Qt::IgnoreAction;
    return sourceModel()->supportedDragActions();
}

void BlankRowProxy::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}

void BlankRowProxy::onHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    const int shift = orientation == Qt::Vertical ? m_blankRows : 0;
    Q_EMIT headerDataChanged(orientation, first + shift, last + shift);
}

void BlankRowProxy::onColumnsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    beginInsertColumns(mapFromSource(parent), first, last);
}

void BlankRowProxy::onColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent,
                                            int destinationColumn)
{
    CHECK_TRUE(beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destinationParent), destinationColumn));
}

void BlankRowProxy::onColumnsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    beginRemoveColumns(mapFromSource(parent), first, last);
}

void BlankRowProxy::onColumnsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    endInsertColumns();
}

void BlankRowProxy::onColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent,
                                   int destinationColumn)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationColumn)
    endMoveColumns();
}

void BlankRowProxy::onColumnsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    endRemoveColumns();
}

void BlankRowProxy::onRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    const int shift = parent.isValid() ? 0 : m_blankRows;
    beginInsertRows(mapFromSource(parent), start + shift, end + shift);
}

void BlankRowProxy::onRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent,
                                         int destinationRow)
{
    const int sourceShift = sourceParent.isValid() ? 0 : m_blankRows;
    const int destinationShift = destinationParent.isValid() ? 0 : m_blankRows;
    beginMoveRows(mapFromSource(sourceParent), sourceStart + sourceShift, sourceShift + sourceEnd, mapFromSource(destinationParent),
                  destinationShift + destinationRow);
}

void BlankRowProxy::onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    const int shift = parent.isValid() ? 0 : m_blankRows;
    beginRemoveRows(mapFromSource(parent), first + shift, last + shift);
}

void BlankRowProxy::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    endInsertRows();
}

void BlankRowProxy::onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent,
                                int destinationRow)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)
    endMoveRows();
}

void BlankRowProxy::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    endRemoveRows();
}

void BlankRowProxy::onLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> mappedParents;
    for (const auto &par : parents)
        mappedParents.append(mapFromSource(par));
    Q_EMIT layoutAboutToBeChanged(mappedParents, hint);
}

void BlankRowProxy::onLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> mappedParents;
    for (const auto &par : parents)
        mappedParents.append(mapFromSource(par));
    Q_EMIT layoutChanged(mappedParents, hint);
}

int BlankRowProxy::blankRows() const
{
    return m_blankRows;
}

void BlankRowProxy::setBlankRows(int newBlankRows)
{
    if (m_blankRows == newBlankRows || newBlankRows < 0)
        return;
    if (newBlankRows > m_blankRows)
        beginInsertRows(QModelIndex(), 0, newBlankRows - m_blankRows);
    else
        beginRemoveRows(QModelIndex(), 0, newBlankRows - m_blankRows);
    m_blankRows = newBlankRows;
    if (newBlankRows > m_blankRows)
        endInsertRows();
    else
        endRemoveRows();
}

Qt::ItemFlags BlankRowProxy::flagsForBlankRow(int row, int column) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QVariant BlankRowProxy::dataForBlankRow(int row, int column, int role) const
{
    return QVariant();
}

void BlankRowProxy::revert()
{
    if (sourceModel())
        sourceModel()->revert();
}

bool BlankRowProxy::submit()
{
    if (sourceModel())
        return sourceModel()->submit();
    return false;
}
