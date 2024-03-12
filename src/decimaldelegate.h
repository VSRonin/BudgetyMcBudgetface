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

#ifndef DECIMALDELEGATE_H
#define DECIMALDELEGATE_H
#include <QStyledItemDelegate>
class DecimalDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DecimalDelegate)
public:
    explicit DecimalDelegate(QObject *parent = nullptr);
    QString displayText(const QVariant &value, const QLocale &locale) const override;
};

#endif
