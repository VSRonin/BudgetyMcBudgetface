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

#ifndef OFFLINESQLITETABLE_H
#define OFFLINESQLITETABLE_H
#include <QAbstractTableModel>
#include <QVector>
#include <QVariant>
#include <QSqlQuery>

struct FiledInfo
{
    FiledInfo()
        : fieldType(QMetaType::UnknownType)
        , allowNull(false)
        , isPrimaryKey(false)
    { }
    FiledInfo(const QString &fName, QMetaType::Type fTyp, bool canNull, bool pk)
        : fieldName(fName)
        , fieldType(fTyp)
        , allowNull(canNull)
        , isPrimaryKey(pk)
    { }
    FiledInfo(const FiledInfo &) = default;
    FiledInfo(FiledInfo &&) = default;
    FiledInfo &operator=(const FiledInfo &) = default;
    FiledInfo &operator=(FiledInfo &&) = default;
    QString fieldName;
    QMetaType::Type fieldType;
    bool allowNull;
    bool isPrimaryKey;
};

class OfflineSqliteTable : public QAbstractTableModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(OfflineSqliteTable)
public:
    explicit OfflineSqliteTable(QObject *parent = nullptr);
    virtual void setTable(const QString &tableName);
    virtual void setFilter(const QString &filter);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QString tableName() const;
    QString filter() const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    virtual QString fieldName(int index) const;
    virtual bool select();
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    bool moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent,
                     int destinationChild) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

protected:
    virtual bool getTableStructure();
    virtual void setQuery(const QString &query);
    virtual void setQuery(QSqlQuery &&query);
    virtual bool setInternalData(const QModelIndex &index, const QVariant &value);

private:
    QMetaType::Type convertSqliteType(const QString &typ) const;
    bool hasPrimaryKey() const;
    QSqlQuery createQuery() const;
    QString m_tableName;
    QString m_filter;
    QSqlQuery m_query;
    QVariantList m_data;
    QVariantList m_headers;
    QList<FiledInfo> m_fields;
    int m_colCount;
    int m_rowCount;
    bool m_needTableInfo;
    int m_sortColumn;
    Qt::SortOrder m_sortOrder;
};

#endif
