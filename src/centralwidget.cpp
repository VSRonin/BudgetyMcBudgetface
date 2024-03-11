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

#include "centralwidget.h"
#include "ui_centralwidget.h"
#include <globals.h>

CentralWidget::CentralWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CentralWidget)
{
    ui->setupUi(this);
    m_object = new MainObject(this);
}

CentralWidget::~CentralWidget()
{
    delete ui;
}
