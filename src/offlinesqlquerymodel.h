/****************************************************************************\
   Copyright 2021 Luca Beldi
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

#ifndef OFFLINESQLQUERYMODEL_H
#define OFFLINESQLQUERYMODEL_H
#include <QAbstractTableModel>
#include <QVector>
#include <QVariant>
#include <QSqlQuery>
class OfflineSqlQueryModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(OfflineSqlQueryModel)
public:
    explicit OfflineSqlQueryModel(QObject *parent = nullptr);
    virtual void setQuery(const QString &query);
    virtual void setQuery(QSqlQuery &&query);
    virtual bool select();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    virtual bool setInternalData(const QModelIndex &index, const QVariant &value);
    virtual QString fieldName(int index) const;
    QSqlQuery *query();
    const QSqlQuery *query() const;

private:
    QSqlQuery m_query;
    QVector<QVariant> m_data;
    QStringList m_headers;
    int m_colCount;
    int m_rowCount;
};

#endif
