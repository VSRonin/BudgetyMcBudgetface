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
#ifndef MULTIPLEFILTERPROXY_H
#define MULTIPLEFILTERPROXY_H

#include <QSortFilterProxyModel>
#include <QPair>
#include <QHash>
#include <QDate>
#include <QRegularExpression>
class MultipleFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MultipleFilterProxy)
    void setRegExpFilter(qint32 col, const QRegularExpression &matcher, qint32 role, bool match);

public:
    explicit MultipleFilterProxy(QObject *parent = nullptr);
    ~MultipleFilterProxy();
    virtual void setDateRangeFilter(qint32 col, const QDate &minDate, const QDate &maxDate, qint32 role = Qt::DisplayRole);
    virtual void setRegExpFilter(qint32 col, const QRegularExpression &matcher, qint32 role = Qt::DisplayRole);
    virtual void setRegExpFilter(qint32 col, const QString &matcher, qint32 role = Qt::DisplayRole);
    virtual void setNegativeRegExpFilter(qint32 col, const QRegularExpression &matcher, qint32 role = Qt::DisplayRole);
    virtual void setNegativeRegExpFilter(qint32 col, const QString &matcher, qint32 role = Qt::DisplayRole);
    virtual void setBoolFilter(qint32 col, bool showWhat, qint32 role = Qt::DisplayRole);
    virtual void clearFilters();
    virtual void removeFilterFromColumn(qint32 col, qint32 role);
    virtual void removeFilterFromColumn(qint32 col);
    void setSourceModel(QAbstractItemModel *mdl) override;

private:
    void onColumnsInserted(const QModelIndex &parent, int first, int last);
    void onModelReset(QAbstractItemModel *mdl);

protected:
    bool hasAnyFilter() const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override = 0;
    template<class T>
    void removeFilterFromColumn(QList<QHash<qint32, T>> &filter, qint32 col)
    {
        if (col < 0 || col >= filter.size())
            return;
        filter[col].clear();
    }
    QList<QHash<qint32, std::pair<QDate, QDate>>> m_dateRangeFilter;
    QList<QHash<qint32, std::pair<QRegularExpression, bool>>> m_regExpFilter;
    QList<QHash<qint32, bool>> m_boolFilter;
    QList<QMetaObject::Connection> m_sourceConnections;
};

class AndFilterProxy : public MultipleFilterProxy
{
    Q_DISABLE_COPY_MOVE(AndFilterProxy)
public:
    using MultipleFilterProxy::MultipleFilterProxy;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
class OrFilterProxy : public MultipleFilterProxy
{
    Q_DISABLE_COPY_MOVE(OrFilterProxy)
public:
    using MultipleFilterProxy::MultipleFilterProxy;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // MULTIPLEFILTERPROXY_H
