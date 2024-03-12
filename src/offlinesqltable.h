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

#ifndef OFFLINESQLTABLE_H
#define OFFLINESQLTABLE_H
#include "offlinesqlquerymodel.h"
#include <QVector>
#include <QVariant>
class OfflineSqlTable : public OfflineSqlQueryModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(OfflineSqlTable)
public:
    explicit OfflineSqlTable(QObject *parent = nullptr);
    virtual void setTable(const QString &tableName);
    virtual void setFilter(const QString &filter);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QSqlQuery createQuery() const;
    QString m_tableName;
    QString m_filter;
};

#endif
