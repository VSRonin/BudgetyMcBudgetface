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

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H
#include <QList>
#include <QWidget>
class MainObject;
namespace Ui {
class CentralWidget;
}

class CentralWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CentralWidget)
public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();
    void setMainObject(MainObject *mainObj);

private:
    MainObject *m_object;
    Ui::CentralWidget *ui;
};
#endif
