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
#ifndef RELATIONALDELEGATE_H
#define RELATIONALDELEGATE_H

#include <QStyledItemDelegate >
class QAbstractItemModel;
class QSortFilterProxyModel;
class RelationalDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RelationalDelegate)
public:
    RelationalDelegate(QObject *parent = nullptr);
    virtual void setRelationModel(QAbstractItemModel *model, int keyCol, int relationCol, int keyRole = Qt::DisplayRole, int relationRole = Qt::DisplayRole);
    QString displayText(const QVariant &value, const QLocale &locale) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

protected:
    QAbstractItemModel *m_relationModel;
    int m_keyCol;
    int m_keyRole;
    int m_relationCol;
    int m_relationRole;
};

class FilteredRelationalDelegate : public RelationalDelegate{
    Q_DISABLE_COPY_MOVE(FilteredRelationalDelegate)
public:
    explicit FilteredRelationalDelegate(QObject* parent = nullptr);
    void setRelationModel(QAbstractItemModel *model, int keyCol, int relationCol, int keyRole = Qt::DisplayRole, int relationRole = Qt::DisplayRole) override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    int relationFilterColumn() const;
    void setRelationFilterColumn(int col);
    int filterKeyColumn() const;
    void setFilterKeyColumn(int newFilterKeyColumn);
    int filterKeyRole() const;
    void setFilterKeyRole(int newFilterKeyRole);
    int relationFilterRole() const;
    void setRelationFilterRole(int newRelationFilterRole);

private:
    int m_filterKeyColumn;
    int m_filterKeyRole;
    int m_relationFilterColumn;
    int m_relationFilterRole;
    QSortFilterProxyModel* m_relationFilterProxy;
};

#endif // RELATIONALDELEGATE_H
