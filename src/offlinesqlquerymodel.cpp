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
#include "offlinesqlquerymodel.h"
#include "globals.h"
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlResult>
OfflineSqlQueryModel::OfflineSqlQueryModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_colCount(0)
    , m_rowCount(0)
{ }

void OfflineSqlQueryModel::setQuery(const QString &query)
{
    QSqlDatabase db = openDb();
    if (!db.isValid() || !db.isOpen())
        return;
    QSqlQuery newQuery(db);
    newQuery.prepare(query);
    setQuery(std::move(newQuery));
}

void OfflineSqlQueryModel::setQuery(QSqlQuery &&query)
{
    m_query = std::move(query);
    select();
}

int OfflineSqlQueryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_colCount;
}

int OfflineSqlQueryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rowCount;
}
bool OfflineSqlQueryModel::setData(const QModelIndex &, const QVariant &, int)
{
    return false;
}

QVariant OfflineSqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    if (section < 0 || section >= m_headers.size())
        return QVariant();
    return m_headers.at(section);
}

bool OfflineSqlQueryModel::setInternalData(const QModelIndex &index, const QVariant &value)
{
    if (!index.isValid())
        return false;
    const int dataIndex = (index.row() * m_colCount) + index.column();
    m_data[dataIndex] = value;
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

QString OfflineSqlQueryModel::fieldName(int index) const
{
    return m_headers.value(index, QString());
}

QSqlQuery *OfflineSqlQueryModel::query()
{
    return &m_query;
}

const QSqlQuery *OfflineSqlQueryModel::query() const
{
    return &m_query;
}
QVariant OfflineSqlQueryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const int dataIndex = (index.row() * m_colCount) + index.column();
        return m_data.at(dataIndex);
    }
    return QVariant();
}

bool OfflineSqlQueryModel::select()
{
    beginResetModel();
    m_colCount = m_rowCount = 0;
    m_data.clear();
    m_headers.clear();
    if (!m_query.exec()) {
        endResetModel();
        return false;
    }
    int newRowCount = 0;
    while (m_query.next()) {
        if (m_colCount == 0) {
            const QSqlRecord selectRecord = m_query.record();
            m_colCount = selectRecord.count();
            m_rowCount = std::max(0, m_query.size());
            m_headers.reserve(m_colCount);
            for (int i = 0; i < m_colCount; ++i)
                m_headers.append(selectRecord.fieldName(i));
            m_data.reserve(std::max(m_colCount, m_colCount * m_rowCount));
        }
        for (int i = 0; i < m_colCount; ++i) {
            const QVariant tempValue = m_query.value(i); // needs to call value before isNull
            if (m_query.isNull(i))
                m_data.append(QVariant());
            else
                m_data.append(tempValue);
        }
        ++newRowCount;
    }
    if (m_rowCount == 0)
        m_rowCount = newRowCount;
    m_query.finish();
    Q_ASSERT(m_rowCount == newRowCount);
    Q_ASSERT(m_rowCount * m_colCount == m_data.size());
    endResetModel();
    return true;
}
