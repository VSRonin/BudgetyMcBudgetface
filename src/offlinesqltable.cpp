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
#include "offlinesqltable.h"
#include "globals.h"
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>
#ifdef QT_DEBUG
#    include <QSqlError>
#endif
OfflineSqlTable::OfflineSqlTable(QObject *parent)
    : OfflineSqlQueryModel(parent)
{ }

bool OfflineSqlTable::setData(const QModelIndex &index, const QVariant &value, int role)
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

Qt::ItemFlags OfflineSqlTable::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return OfflineSqlQueryModel::flags(index);
    return OfflineSqlQueryModel::flags(index) | Qt::ItemIsEditable;
}

QString OfflineSqlTable::tableName() const
{
    return m_tableName;
}

QString OfflineSqlTable::filter() const
{
    return m_filter;
}

QSqlQuery OfflineSqlTable::createQuery() const
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

void OfflineSqlTable::setTable(const QString &tableName)
{
    m_tableName = tableName;
    setQuery(createQuery());
}

void OfflineSqlTable::setFilter(const QString &filter)
{
    m_filter = filter;
    setQuery(createQuery());
}
