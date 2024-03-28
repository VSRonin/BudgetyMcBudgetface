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

BlankRowProxy::BlankRowProxy(QObject *parent)
    : QAbstractProxyModel(parent)
{ }

QModelIndex BlankRowProxy::buddy(const QModelIndex &index) const
{
    return index;
}

bool BlankRowProxy::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    if (!sourceModel())
        return false;
    if (!parent.isValid() && row == 0)
        return false;
    return sourceModel()->canDropMimeData(data, action, parent.isValid() ? row : (row - 1), column, parent);
}

bool BlankRowProxy::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.isValid() && row == 0)
        return false;
    return sourceModel()->dropMimeData(data, action, parent.isValid() ? row : (row - 1), column, parent);
}

bool BlankRowProxy::clearItemData(const QModelIndex &index)
{
    if (!sourceModel())
        return false;
    if (!index.parent().isValid() && index.row() == 0)
        return false;
    return sourceModel()->clearItemData(mapToSource(index));
}

QVariant BlankRowProxy::data(const QModelIndex &proxyIndex, int role) const
{
    if (!sourceModel())
        return QVariant();
    if (!proxyIndex.parent().isValid() && proxyIndex.row() == 0)
        return QVariant();
    return sourceModel()->data(mapToSource(proxyIndex), role);
}

void BlankRowProxy::multiData(const QModelIndex &index, QModelRoleDataSpan roleDataSpan) const
{
    if (!sourceModel() || !index.isValid() || (!index.parent().isValid() && index.row() == 0)) {
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
    if (!parent.parent().isValid() && parent.row() == 0)
        return false;
    return sourceModel()->hasChildren(mapToSource(parent));
}

QVariant BlankRowProxy::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!sourceModel())
        return QVariant();
    return sourceModel()->headerData(orientation == Qt::Vertical ? (section - 1) : section, orientation, role);
}

QMap<int, QVariant> BlankRowProxy::itemData(const QModelIndex &proxyIndex) const
{
    if (!sourceModel())
        return QMap<int, QVariant>();
    if (!proxyIndex.parent().isValid() && proxyIndex.row() == 0)
        return QMap<int, QVariant>();
    return sourceModel()->itemData(mapToSource(proxyIndex));
}

bool BlankRowProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!sourceModel())
        return false;
    if (!index.parent().isValid() && index.row() == 0)
        return false;
    return sourceModel()->setData(mapToSource(index), value, role);
}

bool BlankRowProxy::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (!sourceModel())
        return false;
    if (section == 0 && orientation == Qt::Vertical)
        return false;
    return sourceModel()->setHeaderData(orientation == Qt::Vertical ? (section - 1) : section, orientation, value, role);
}

bool BlankRowProxy::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    if (!sourceModel())
        return false;
    if (!index.parent().isValid() && index.row() == 0)
        return false;
    return sourceModel()->setItemData(mapToSource(index), roles);
}

int BlankRowProxy::rowCount(const QModelIndex &parent) const
{
    if (!sourceModel())
        return 0;
    return sourceModel()->rowCount(parent) + (parent.isValid() ? 0 : 1);
}

int BlankRowProxy::columnCount(const QModelIndex &parent) const
{
    if (!sourceModel())
        return 0;
    return sourceModel()->columnCount(parent);
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
    if (!sourceModel())
        return QModelIndex();
    if (sourceIndex.parent().isValid())
        return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
    return createIndex(sourceIndex.row() + 1, sourceIndex.column());
}

QModelIndex BlankRowProxy::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel())
        return QModelIndex();
    if (proxyIndex.internalPointer())
        return createSourceIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());
    if (proxyIndex.row() == 0)
        return QModelIndex();
    return sourceModel()->index(proxyIndex.row() - 1, proxyIndex.column());
}

void BlankRowProxy::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel())
        return;
    beginResetModel();
    for (const auto &conn : qAsConst(m_sourceConnections))
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
    if (index.parent().isValid() || index.row() > 0)
        return sourceModel()->flags(mapToSource(index));
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
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
    if (!parent.parent().isValid() && parent.row() == 0)
        return false;
    return sourceModel()->insertColumns(column, count, mapToSource(parent));
}

bool BlankRowProxy::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() == 0)
        return false;
    return sourceModel()->insertRows(parent.isValid() ? row : std::max(0, row - 1), count, mapToSource(parent));
}

bool BlankRowProxy::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() == 0)
        return false;
    return sourceModel()->removeColumns(column, count, mapToSource(parent));
}

bool BlankRowProxy::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!sourceModel())
        return false;
    if (!parent.parent().isValid() && parent.row() == 0)
        return false;
    if (!parent.isValid() && row == 0)
        return false;
    return sourceModel()->removeRows(parent.isValid() ? row : (row - 1), count, mapToSource(parent));
}

bool BlankRowProxy::moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent,
                                int destinationChild)
{
    if (!sourceModel())
        return false;
    if (!sourceParent.parent().isValid() && sourceParent.row() == 0)
        return false;
    if (!destinationParent.parent().isValid() && destinationParent.row() == 0)
        return false;
    return sourceModel()->moveColumns(mapToSource(sourceParent), sourceColumn, count, mapToSource(destinationParent), destinationChild);
}

bool BlankRowProxy::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (!sourceModel())
        return false;
    if (!sourceParent.parent().isValid() && sourceParent.row() == 0)
        return false;
    if (!destinationParent.parent().isValid() && destinationParent.row() == 0)
        return false;
    if (!sourceParent.isValid() && sourceRow == 0)
        return false;
    return sourceModel()->moveRows(mapToSource(sourceParent), sourceParent.isValid() ? sourceRow : (sourceRow - 1), count,
                                   mapToSource(destinationParent),
                                   destinationParent.isValid() ? destinationChild : std::max(0, destinationChild - 1));
}

void BlankRowProxy::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}

void BlankRowProxy::onHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    const int shift = orientation == Qt::Vertical ? 1 : 0;
    Q_EMIT headerDataChanged(orientation, first + shift, last + shift);
}

void BlankRowProxy::onColumnsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    beginInsertColumns(mapFromSource(parent), first, last);
}

void BlankRowProxy::onColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent,
                                            int destinationColumn)
{
    Q_ASSUME(beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destinationParent), destinationColumn));
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
    const int shift = parent.isValid() ? 0 : 1;
    beginInsertRows(mapFromSource(parent), start + shift, end + shift);
}

void BlankRowProxy::onRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent,
                                         int destinationRow)
{
    const int sourceShift = sourceParent.isValid() ? 0 : 1;
    const int destinationShift = destinationParent.isValid() ? 0 : 1;
    beginMoveRows(mapFromSource(sourceParent), sourceStart + sourceShift, sourceShift + sourceEnd, mapFromSource(destinationParent),
                  destinationShift + destinationRow);
}

void BlankRowProxy::onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    const int shift = parent.isValid() ? 0 : 1;
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
