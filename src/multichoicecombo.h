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
#ifndef MULTICHOICECOMBO_H
#define MULTICHOICECOMBO_H

#include <QComboBox>
class RoleMaskProxyModel;
class QStandardItemModel;
class MultichoiceCombo : public QComboBox
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MultichoiceCombo)
public:
    MultichoiceCombo(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model) override;
    const QString &chosenText() const;
    QList<int> checkedIndexes() const;
    void setCheckedIndexes(const QList<int> &idx);

protected:
    void initStyleOption(QStyleOptionComboBox *option) const override;
signals:
    void chosenTextChanged(const QString &newText);

private:
    QStandardItemModel *m_baseModel;
    RoleMaskProxyModel *m_choiceMask;
    QString m_chosenText;
    void updateChosenText();
};

#endif // MULTICHOICECOMBO_H
