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
#include "offlinesqlitetable.h"
#include "globals.h"
#include <QSqlDriver>
#include <QSqlRecord>
#ifdef QT_DEBUG
#    include <QSqlError>
#endif
OfflineSqliteTable::OfflineSqliteTable(QObject *parent)
    : QAbstractTableModel(parent)
    , m_colCount(0)
    , m_rowCount(0)
    , m_needTableInfo(true)
    , m_sortColumn(-1)
    , m_sortOrder(Qt::AscendingOrder)
{ }

bool OfflineSqliteTable::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row + count - 1 >= m_rowCount)
        return false;
    const bool hasPk = hasPrimaryKey();
    QSqlDatabase db = openDb();
    if (!db.isValid() || !db.isOpen())
        return false;
    QString removeQueryString =
            QLatin1String("DELETE FROM ") + db.driver()->escapeIdentifier(m_tableName, QSqlDriver::TableName) + QLatin1String(" WHERE ");
    const int colCount = m_colCount;
    bool firstRow = true;
    for (int h = 0; h < count; ++h) {
        if (!firstRow)
            removeQueryString += QLatin1String(" OR ");
        firstRow = false;
        bool firstField = true;
        removeQueryString += QLatin1Char('(');
        for (int i = 0; i < colCount; ++i) {
            if (hasPk && !m_fields.at(i).isPrimaryKey)
                continue;
            if (!firstField)
                removeQueryString += QLatin1String("AND ");
            firstField = false;
            removeQueryString += db.driver()->escapeIdentifier(m_fields.at(i).fieldName, QSqlDriver::FieldName);
            if (index(row + h, i).data().isValid())
                removeQueryString += QLatin1String("=? ");
            else
                removeQueryString += QLatin1String(" IS NULL ");
        }
        removeQueryString += QLatin1Char(')');
    }
    QSqlQuery removeQuery(db);
    removeQuery.prepare(removeQueryString);
    for (int h = 0; h < count; ++h) {
        for (int i = 0; i < colCount; ++i) {
            if (hasPk && !m_fields.at(i).isPrimaryKey)
                continue;
            const QVariant currData = index(row + h, i).data();
            if (currData.isValid())
                removeQuery.addBindValue(currData);
        }
    }
    if (!db.transaction()) // dry run to make sure query runs before calling beginRemoveRows
        return false;
    if (!removeQuery.exec()) {
#ifdef QT_DEBUG
        qDebug().noquote() << removeQuery.executedQuery() << removeQuery.lastError().text();
#endif
        return false;
    }
    CHECK_TRUE(db.rollback());
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    CHECK_TRUE(removeQuery.exec());
    endRemoveRows();
    return true;
}

bool OfflineSqliteTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid))
        return false;
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return false;
    QSqlDatabase db = openDb();
    if (!db.isValid() || !db.isOpen())
        return false;
    QString updateQueryString = QLatin1String("UPDATE ") + db.driver()->escapeIdentifier(m_tableName, QSqlDriver::TableName) + QLatin1String(" SET ")
            + db.driver()->escapeIdentifier(m_fields.at(index.column()).fieldName, QSqlDriver::FieldName) + QLatin1String("=");
    if (value.isValid())
        updateQueryString += QLatin1Char('?');
    else
        updateQueryString += QLatin1String("NULL");
    updateQueryString += QLatin1String(" WHERE ");
    const bool hasPk = hasPrimaryKey();
    bool firstField = true;
    const int colCount = m_colCount;
    for (int i = 0; i < colCount; ++i) {
        if (hasPk && !m_fields.at(i).isPrimaryKey)
            continue;
        if (!firstField)
            updateQueryString += QLatin1String("AND ");
        firstField = false;
        updateQueryString += db.driver()->escapeIdentifier(m_fields.at(i).fieldName, QSqlDriver::FieldName);
        if (data(index.sibling(index.row(), i)).isValid())
            updateQueryString += QLatin1String("=? ");
        else
            updateQueryString += QLatin1String(" IS NULL ");
    }
    QSqlQuery updateQuery(db);
    updateQuery.prepare(updateQueryString);
    if (value.isValid()) {
        if (value.typeId() == m_fields.at(index.column()).fieldType) {
            updateQuery.addBindValue(value);
        } else {
            QVariant valueCopy = value;
            if (!valueCopy.convert(QMetaType(m_fields.at(index.column()).fieldType)))
                return false;
            updateQuery.addBindValue(valueCopy);
        }
    } else {
        if (!m_fields.at(index.column()).allowNull)
            return false;
    }
    for (int i = 0; i < colCount; ++i) {
        if (hasPk && !m_fields.at(i).isPrimaryKey)
            continue;
        const QVariant currData = data(index.sibling(index.row(), i));
        if (currData.isValid())
            updateQuery.addBindValue(currData);
    }
    if (updateQuery.exec()) {
        setInternalData(index, value);
        return true;
    }
#ifdef QT_DEBUG
    qDebug().noquote() << updateQuery.executedQuery() << updateQuery.lastError().text();
#endif
    return false;
}

Qt::ItemFlags OfflineSqliteTable::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QString OfflineSqliteTable::tableName() const
{
    return m_tableName;
}

QString OfflineSqliteTable::filter() const
{
    return m_filter;
}

QSqlQuery OfflineSqliteTable::createQuery() const
{
    QSqlDatabase db = openDb();
    if (!db.isValid() || !db.isOpen())
        return QSqlQuery();
    QString queryString = QLatin1String("SELECT * FROM ") + db.driver()->escapeIdentifier(m_tableName, QSqlDriver::TableName);
    if (!m_filter.isEmpty())
        queryString += QLatin1String(" WHERE ") + m_filter;
    if (m_sortColumn >= 0 && m_sortColumn < m_colCount) {
        queryString += QLatin1String(" ORDER BY ") + db.driver()->escapeIdentifier(m_fields.at(m_sortColumn).fieldName, QSqlDriver::FieldName);
        if (m_sortOrder == Qt::AscendingOrder)
            queryString += QLatin1String(" ASC");
        else
            queryString += QLatin1String(" DESC");
    }
    QSqlQuery selectQuery(db);
    selectQuery.prepare(queryString);
    return selectQuery;
}

void OfflineSqliteTable::setTable(const QString &tableName)
{
    const bool refreshStructure = m_tableName != tableName;
    m_tableName = tableName;
    if (refreshStructure || m_needTableInfo)
        getTableStructure();
    setQuery(createQuery());
}

void OfflineSqliteTable::setFilter(const QString &filter)
{
    m_filter = filter;
    setQuery(createQuery());
}

void OfflineSqliteTable::sort(int column, Qt::SortOrder order)
{
    m_sortColumn = column;
    m_sortOrder = order;
    setQuery(createQuery());
}

bool OfflineSqliteTable::moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent,
                                     int destinationChild)
{
    return false;
}

bool OfflineSqliteTable::insertColumns(int column, int count, const QModelIndex &parent)
{
    return false;
}

bool OfflineSqliteTable::removeColumns(int column, int count, const QModelIndex &parent)
{
    return false;
}

bool OfflineSqliteTable::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent,
                                  int destinationChild)
{
    return false;
}

bool OfflineSqliteTable::insertRows(int row, int count, const QModelIndex &parent)
{
    return false;
}

QVariant OfflineSqliteTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    if (section < 0 || section >= m_headers.size())
        return QVariant();
    return m_headers.at(section);
}

bool OfflineSqliteTable::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role != Qt::DisplayRole || orientation == Qt::Vertical)
        return false;
    if (section < 0 || section >= m_headers.size())
        return false;
    m_headers[section] = value;
    Q_EMIT headerDataChanged(orientation, section, section);
    return true;
}

bool OfflineSqliteTable::setInternalData(const QModelIndex &index, const QVariant &value)
{
    if (!index.isValid())
        return false;
    const int dataIndex = (index.row() * m_colCount) + index.column();
    m_data[dataIndex] = value;
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

QMetaType::Type OfflineSqliteTable::convertSqliteType(const QString &typ) const
{
    if (typ.compare(QStringLiteral("INTEGER"), Qt::CaseInsensitive) == 0 || typ.compare(QStringLiteral("INT"), Qt::CaseInsensitive) == 0)
        return QMetaType::Int; // TODO maybe 64 bits?
    if (typ.compare(QStringLiteral("REAL"), Qt::CaseInsensitive) == 0)
        return QMetaType::Double;
    if (typ.compare(QStringLiteral("TEXT"), Qt::CaseInsensitive) == 0)
        return QMetaType::QString;
    if (typ.compare(QStringLiteral("BLOB"), Qt::CaseInsensitive) == 0)
        return QMetaType::QByteArray;
    return QMetaType::UnknownType;
}

bool OfflineSqliteTable::hasPrimaryKey() const
{
    return std::any_of(m_fields.cbegin(), m_fields.cend(), [](const FiledInfo &a) -> bool { return a.isPrimaryKey; });
}

QString OfflineSqliteTable::fieldName(int index) const
{
    return m_fields.value(index, FiledInfo()).fieldName;
}

bool OfflineSqliteTable::getTableStructure()
{
    beginResetModel();
    m_headers.clear();
    m_data.clear();
    m_fields.clear();
    m_colCount = 0;
    m_rowCount = 0;
    if (m_tableName.isEmpty())
        return true;
    QSqlDatabase db = openDb();
    if (!db.isValid() || !db.isOpen())
        return false;
    QSqlQuery structureQuery(db);
    structureQuery.prepare(QLatin1String("PRAGMA table_info(") + db.driver()->escapeIdentifier(m_tableName, QSqlDriver::TableName)
                           + QLatin1Char(')'));
    if (!structureQuery.exec()) {
#ifdef QT_DEBUG
        qDebug() << m_query.lastQuery() << m_query.lastError().text();
#endif
        return false;
    }
    m_colCount = std::max(0, structureQuery.size());
    m_headers.reserve(m_colCount);
    m_fields.reserve(m_colCount);
    int newColCount;
    for (newColCount = 0; structureQuery.next(); ++newColCount) {
        const QString currFieldName = structureQuery.value(1).toString();
        m_headers.append(currFieldName);
        m_fields.append(FiledInfo(currFieldName, convertSqliteType(structureQuery.value(2).toString()), structureQuery.value(3).toInt() == 0,
                                  structureQuery.value(5).toInt() > 0));
    }
    if (m_colCount == 0)
        m_colCount = newColCount;
    Q_ASSERT(newColCount == m_colCount);
    endResetModel();
    m_needTableInfo = false;
    return true;
}

QVariant OfflineSqliteTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const int dataIndex = (index.row() * m_colCount) + index.column();
        return m_data.at(dataIndex);
    }
    return QVariant();
}

bool OfflineSqliteTable::select()
{
    beginResetModel();
    m_rowCount = 0;
    m_data.clear();
    if (!m_query.exec()) {
#ifdef QT_DEBUG
        qDebug() << m_query.executedQuery() << m_query.lastError().text();
#endif
        endResetModel();
        return false;
    }
    int newRowCount = 0;
    for (; m_query.next(); ++newRowCount) {
        if (newRowCount == 0) {
            m_rowCount = std::max(0, m_query.size());
            m_data.reserve(std::max(m_colCount, m_colCount * m_rowCount));
        }
        for (int i = 0; i < m_colCount; ++i) {
            const QVariant tempValue = m_query.value(i); // needs to call value before isNull
            if (m_query.isNull(i))
                m_data.append(QVariant());
            else
                m_data.append(tempValue);
        }
    }
    if (m_rowCount == 0)
        m_rowCount = newRowCount;
    m_query.finish();
    Q_ASSERT(m_rowCount == newRowCount);
    Q_ASSERT(m_rowCount * m_colCount == m_data.size());
    endResetModel();
    return true;
}

void OfflineSqliteTable::setQuery(const QString &query)
{
    QSqlDatabase db = openDb();
    if (!db.isValid() || !db.isOpen())
        return;
    QSqlQuery newQuery(db);
    newQuery.prepare(query);
    setQuery(std::move(newQuery));
}

void OfflineSqliteTable::setQuery(QSqlQuery &&query)
{
    m_query = std::move(query);
    select();
}

int OfflineSqliteTable::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_colCount;
}

int OfflineSqliteTable::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rowCount;
}
