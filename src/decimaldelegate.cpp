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
#include "decimaldelegate.h"
DecimalDelegate::DecimalDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{ }

QString DecimalDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    QLocale localeCopy = locale;
    localeCopy.setNumberOptions(localeCopy.numberOptions() & (~QLocale::OmitGroupSeparator));
    return localeCopy.toString(value.toDouble(), 'f', 2);
}

void DecimalDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->displayAlignment = Qt::AlignVCenter | Qt::AlignRight;
}
