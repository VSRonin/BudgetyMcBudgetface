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
#include "multiplefilterproxy.h"

MultipleFilterProxy::MultipleFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(false);
    setSortLocaleAware(true);
}

MultipleFilterProxy::~MultipleFilterProxy() = default;

void MultipleFilterProxy::setSourceModel(QAbstractItemModel *mdl)
{
    if (sourceModel() == mdl)
        return;
    for (const auto &conn : qAsConst(m_sourceConnections))
        QObject::disconnect(conn);
    m_sourceConnections.clear();
    if (mdl) {
        m_sourceConnections << connect(mdl, &QAbstractItemModel::columnsInserted, this, &MultipleFilterProxy::onColumnsInserted)
                            << connect(mdl, &QAbstractItemModel::modelReset, this, std::bind(&MultipleFilterProxy::onModelReset, this, mdl));
    }
    onModelReset(mdl);
    QSortFilterProxyModel::setSourceModel(mdl);
}

void MultipleFilterProxy::onColumnsInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;
    for (; first <= last; --last) {
        m_dateRangeFilter.insert(first, QHash<qint32, std::pair<QDate, QDate>>());
        m_regExpFilter.insert(first, QHash<qint32, std::pair<QRegularExpression, bool>>());
        m_boolFilter.insert(first, QHash<qint32, bool>());
    }
}

void MultipleFilterProxy::onModelReset(QAbstractItemModel *mdl)
{
    if (!mdl)
        return;
    m_dateRangeFilter.clear();
    m_regExpFilter.clear();
    m_boolFilter.clear();
    const int sourceCols = mdl->columnCount();
    m_dateRangeFilter.reserve(sourceCols);
    m_regExpFilter.reserve(sourceCols);
    m_boolFilter.reserve(sourceCols);
    for (int i = 0; i < sourceCols; ++i) {
        m_dateRangeFilter.append(QHash<qint32, std::pair<QDate, QDate>>());
        m_regExpFilter.append(QHash<qint32, std::pair<QRegularExpression, bool>>());
        m_boolFilter.append(QHash<qint32, bool>());
    }
}

bool MultipleFilterProxy::hasAnyFilter() const
{
    return
    std::any_of(m_dateRangeFilter.cbegin(),m_dateRangeFilter.cend(),[](const QHash<qint32, std::pair<QDate, QDate>>& filter)->bool{return !filter.isEmpty();})
 ||std::any_of(m_regExpFilter.cbegin(),m_regExpFilter.cend(),[](const QHash<qint32, std::pair<QRegularExpression, bool>>& filter)->bool{return !filter.isEmpty();})
  ||          std::any_of(m_boolFilter.cbegin(),m_boolFilter.cend(),[](const QHash<qint32, bool>& filter)->bool{return !filter.isEmpty();})
    ;
}

void MultipleFilterProxy::setDateRangeFilter(qint32 col, const QDate &minDate, const QDate &maxDate, qint32 role)
{
    if (col < 0 || col >= m_dateRangeFilter.size())
        return;
    if (minDate.isNull() && maxDate.isNull())
        return removeFilterFromColumn(col, role);
    m_regExpFilter[col].remove(role);
    m_boolFilter[col].remove(role);
    m_dateRangeFilter[col][role] = std::make_pair(minDate, maxDate);
    invalidateFilter();
}

void MultipleFilterProxy::setRegExpFilter(qint32 col, const QRegularExpression &matcher, qint32 role)
{
    setRegExpFilter(col, matcher, role, true);
}

void MultipleFilterProxy::setRegExpFilter(qint32 col, const QString &matcher, qint32 role)
{
    if (matcher.isEmpty())
        return removeFilterFromColumn(col, role);
    return setRegExpFilter(col, QRegularExpression(matcher), role);
}

void MultipleFilterProxy::setRegExpFilter(qint32 col, const QRegularExpression &matcher, qint32 role, bool match)
{
    if (col < 0 || col >= m_dateRangeFilter.size())
        return;
    if (matcher.pattern().isEmpty() || !matcher.isValid())
        return removeFilterFromColumn(col, role);
    m_dateRangeFilter[col].remove(role);
    m_boolFilter[col].remove(role);
    m_regExpFilter[col][role] = std::make_pair(matcher, match);
    invalidateFilter();
}

void MultipleFilterProxy::setNegativeRegExpFilter(qint32 col, const QRegularExpression &matcher, qint32 role)
{
    setRegExpFilter(col, matcher, role, false);
}

void MultipleFilterProxy::setNegativeRegExpFilter(qint32 col, const QString &matcher, qint32 role)
{
    if (matcher.isEmpty())
        return removeFilterFromColumn(col, role);
    return setNegativeRegExpFilter(col, QRegularExpression(matcher), role);
}

void MultipleFilterProxy::setBoolFilter(qint32 col, bool showWhat, qint32 role)
{
    if (col < 0 || col >= m_dateRangeFilter.size())
        return;
    m_regExpFilter[col].remove(role);
    m_dateRangeFilter[col].remove(role);
    m_boolFilter[col][role] = showWhat;
    invalidateFilter();
}

void MultipleFilterProxy::clearFilters()
{
    for (int i = 0; i < m_dateRangeFilter.size(); ++i) {
        m_dateRangeFilter[i].clear();
        m_regExpFilter[i].clear();
        m_boolFilter[i].clear();
    }
    invalidateFilter();
}

void MultipleFilterProxy::removeFilterFromColumn(qint32 col)
{
    removeFilterFromColumn(m_dateRangeFilter, col);
    removeFilterFromColumn(m_regExpFilter, col);
    removeFilterFromColumn(m_boolFilter, col);
    invalidateFilter();
}

void MultipleFilterProxy::removeFilterFromColumn(qint32 col, qint32 role)
{
    if (col < 0 || col >= m_dateRangeFilter.size())
        return;
    m_dateRangeFilter[col].remove(role);
    m_regExpFilter[col].remove(role);
    m_boolFilter[col].remove(role);
    invalidateFilter();
}

bool AndFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex currntIndex = sourceModel()->index(source_row, 0, source_parent);
    if (sourceModel()->hasChildren(currntIndex)) {
        bool result = false;
        for (int i = 0; i < sourceModel()->rowCount(currntIndex) && !result; ++i) {
            result = result || filterAcceptsRow(i, currntIndex);
        }
        return result;
    } else {
        for (int i = 0; i < sourceModel()->columnCount(source_parent); ++i) {
            currntIndex = sourceModel()->index(source_row, i, source_parent);
            if (currntIndex.data().isNull() && currntIndex.parent().isValid()) {
                currntIndex = sourceModel()->index(source_parent.row(), i, source_parent.parent());
            }
            for (auto dateRngIter = m_dateRangeFilter.at(i).constBegin(); dateRngIter != m_dateRangeFilter.at(i).constEnd(); ++dateRngIter) {
                const QVariant testDateVariant = currntIndex.data(dateRngIter.key());
                const QDate testDate =
                        testDateVariant.canConvert<QDate>() ? testDateVariant.toDate() : QDate::fromString(testDateVariant.toString(), Qt::ISODate);
                if (!((testDate >= dateRngIter.value().first || dateRngIter.value().first.isNull())
                      && (testDate <= dateRngIter.value().second || dateRngIter.value().second.isNull())))
                    return false;
            }
            for (auto boolIter = m_boolFilter.at(i).constBegin(); boolIter != m_boolFilter.at(i).constEnd(); ++boolIter) {
                const bool testBool = currntIndex.data(boolIter.key()).toBool();
                if (testBool != boolIter.value())
                    return false;
            }
            for (auto regExpIter = m_regExpFilter.at(i).constBegin(); regExpIter != m_regExpFilter.at(i).constEnd(); ++regExpIter) {
                if (regExpIter.value().first.match(currntIndex.data(regExpIter.key()).toString()).hasMatch() != regExpIter.value().second)
                    return false;
            }
        }
    }
    return true;
}

bool OrFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex currntIndex = sourceModel()->index(source_row, 0, source_parent);
    if (sourceModel()->hasChildren(currntIndex)) {
        bool result = false;
        for (int i = 0; i < sourceModel()->rowCount(currntIndex) && !result; ++i) {
            result = result || filterAcceptsRow(i, currntIndex);
        }
        return result;
    } else {
        if(!hasAnyFilter())
            return true;
        for (int i = 0; i < sourceModel()->columnCount(source_parent); ++i) {
            currntIndex = sourceModel()->index(source_row, i, source_parent);
            if (currntIndex.data().isNull() && currntIndex.parent().isValid()) {
                currntIndex = sourceModel()->index(source_parent.row(), i, source_parent.parent());
            }
            for (auto dateRngIter = m_dateRangeFilter.at(i).constBegin(); dateRngIter != m_dateRangeFilter.at(i).constEnd(); ++dateRngIter) {
                const QDate testDate = currntIndex.data(dateRngIter.key()).toDate();
                if (((testDate >= dateRngIter.value().first || dateRngIter.value().first.isNull())
                     && (testDate <= dateRngIter.value().second || dateRngIter.value().second.isNull())))
                    return true;
            }
            for (auto boolIter = m_boolFilter.at(i).constBegin(); boolIter != m_boolFilter.at(i).constEnd(); ++boolIter) {
                const bool testBool = currntIndex.data(boolIter.key()).toBool();
                if (testBool == boolIter.value())
                    return true;
            }
            for (auto regExpIter = m_regExpFilter.at(i).constBegin(); regExpIter != m_regExpFilter.at(i).constEnd(); ++regExpIter) {
                if (regExpIter.value().first.match(currntIndex.data(regExpIter.key()).toString()).hasMatch() == regExpIter.value().second)
                    return true;
            }
        }
    }
    return false;
}
