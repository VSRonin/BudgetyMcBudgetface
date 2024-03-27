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
#include "multichoicecombo.h"
#include <RoleMaskProxyModel>
#include <QStandardItemModel>
#include <QListView>
#include <QEvent>
class CheckableRoleMaskProxyModel : public RoleMaskProxyModel
{
    Q_DISABLE_COPY_MOVE(CheckableRoleMaskProxyModel)
public:
    using RoleMaskProxyModel::RoleMaskProxyModel;
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return (RoleMaskProxyModel::flags(index) & (~Qt::ItemIsSelectable)) | Qt::ItemIsUserCheckable;
    }
};

MultichoiceCombo::MultichoiceCombo(QWidget *parent)
    : QComboBox(parent)
    , m_baseModel(new QStandardItemModel(0, 1, this))
    , m_choiceMask(new CheckableRoleMaskProxyModel(this))
{
    m_choiceMask->setTransparentIfEmpty(false);
    m_choiceMask->setMaskedRoles({Qt::CheckStateRole});
    m_choiceMask->setMaskedRoleDefaultValue(Qt::CheckStateRole, Qt::Unchecked);
    m_choiceMask->setSourceModel(m_baseModel);
    QComboBox::setModel(m_choiceMask);
    connect(m_choiceMask, &QAbstractItemModel::modelReset, this, &MultichoiceCombo::updateChosenText);
    connect(m_choiceMask, &QAbstractItemModel::rowsInserted, this, &MultichoiceCombo::updateChosenText);
    connect(m_choiceMask, &QAbstractItemModel::rowsRemoved, this, &MultichoiceCombo::updateChosenText);
    connect(m_choiceMask, &QAbstractItemModel::columnsInserted, this, &MultichoiceCombo::updateChosenText);
    connect(m_choiceMask, &QAbstractItemModel::columnsRemoved, this, &MultichoiceCombo::updateChosenText);
    connect(m_choiceMask, &QAbstractItemModel::rowsMoved, this, &MultichoiceCombo::updateChosenText);
    connect(m_choiceMask, &QAbstractItemModel::columnsMoved, this, &MultichoiceCombo::updateChosenText);
    connect(m_choiceMask, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QList<int> &roles) {
        if (roles.isEmpty() || roles.contains(Qt::CheckStateRole))
            updateChosenText();
    });
}

void MultichoiceCombo::setModel(QAbstractItemModel *model)
{
    Q_ASSERT(model);
    if (m_choiceMask->sourceModel() == m_baseModel && model != m_baseModel)
        m_baseModel->deleteLater();
    m_choiceMask->setSourceModel(model);
}

const QString &MultichoiceCombo::chosenText() const
{
    return m_chosenText;
}

QList<int> MultichoiceCombo::checkedIndexes() const
{
    QList<int> result;
    for (int i = 0, maxI = m_choiceMask->rowCount(); i < maxI; ++i) {
        if (m_choiceMask->index(i, modelColumn()).data(Qt::CheckStateRole).toInt() == Qt::Checked)
            result.append(i);
    }
    return result;
}

void MultichoiceCombo::setCheckedIndexes(const QList<int> &idx)
{
    for (int i = 0, maxI = m_choiceMask->rowCount(); i < maxI; ++i)
        m_choiceMask->setData(m_choiceMask->index(i, modelColumn()), idx.contains(i) ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}

void MultichoiceCombo::initStyleOption(QStyleOptionComboBox *option) const
{
    QComboBox::initStyleOption(option);
    option->currentText = m_chosenText;
}

void MultichoiceCombo::updateChosenText()
{
    QString result;
    const QString separator = tr(", ");
    for (int i = 0, maxI = m_choiceMask->rowCount(); i < maxI; ++i) {
        if (m_choiceMask->index(i, modelColumn()).data(Qt::CheckStateRole).toInt() == Qt::Checked)
            result += m_choiceMask->index(i, modelColumn()).data().toString() + separator;
    }
    result.chop(separator.size());
    if (result != m_chosenText) {
        m_chosenText = result;
        chosenTextChanged(m_chosenText);
    }
}
