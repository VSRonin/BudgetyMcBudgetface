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
{ }

bool OfflineSqliteTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return false;
    QSqlDatabase db = openDb();
    QString selectQueryString = QLatin1String("UPDATE ") + db.driver()->escapeIdentifier(m_tableName, QSqlDriver::TableName) + QLatin1String(" SET ")
            + db.driver()->escapeIdentifier(fieldName(index.column()), QSqlDriver::FieldName) + QLatin1String("=");
    if (value.isValid())
        selectQueryString += QLatin1Char('?');
    else
        selectQueryString += QLatin1String("NULL");
    selectQueryString += QLatin1String(" WHERE ");
    const int colCount = columnCount();
    for (int i = 0; i < colCount; ++i) {
        if (i > 0)
            selectQueryString += QLatin1String("AND ");
        selectQueryString += db.driver()->escapeIdentifier(fieldName(i), QSqlDriver::FieldName);
        if (data(index.sibling(index.row(), i)).isValid())
            selectQueryString += QLatin1String("=? ");
        else
            selectQueryString += QLatin1String(" IS NULL ");
    }
    QSqlQuery selectQuery(db);
    selectQuery.prepare(selectQueryString);
    if (value.isValid())
        selectQuery.addBindValue(value);
    for (int i = 0; i < colCount; ++i) {
        const QVariant currData = data(index.sibling(index.row(), i));
        if (currData.isValid())
            selectQuery.addBindValue(currData);
    }
    if (selectQuery.exec()) {
        setInternalData(index, value);
        return true;
    }
#ifdef QT_DEBUG
    qDebug().noquote() << selectQuery.lastQuery() << selectQuery.lastError().text();
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

QString OfflineSqliteTable::fieldName(int index) const
{
    return m_fields.value(index, QString());
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
        m_fields.append(currFieldName);
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
        qDebug() << m_query.lastQuery() << m_query.lastError().text();
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
