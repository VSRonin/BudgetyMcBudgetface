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
#include <QTranslator>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "centralwidget.h"
#include "settingsdialog.h"
#include <mainobject.h>
#include <QMessageBox>
#include <QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_object(new MainObject(this))
    , m_settingsDialog(new SettingsDialog(this))
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralwidget->setMainObject(m_object);
    m_settingsDialog->setMainObject(m_object);
    m_settingsDialog->setModal(true);
    m_settingsDialog->hide();
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onFileNew);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onFileSave);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::onFileSaveAs);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onFileLoad);
    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &SettingsDialog::show);
    connect(m_object, &MainObject::dirtyChanged, this, &MainWindow::setWindowModified);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::onFileNew()
{
    Q_ASSERT(m_object);
    if (m_object->isDirty()) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
                this, tr("Do you want to save?"), tr("There are unsaved changes to your budget. Do you want to save them before continuing?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (answer == QMessageBox::Cancel)
            return;
        if (answer == QMessageBox::Save)
            onFileSave();
    }
    m_object->newBudget();
    m_lastSavedPath.clear();
}

bool MainWindow::onFileSave()
{
    if (m_lastSavedPath.isEmpty())
        return onFileSaveAs();
    Q_ASSERT(m_object);
    if (!m_object->saveBudget(m_lastSavedPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Error while saving the budget. Try again later"));
        return false;
    }
    return true;
}

bool MainWindow::onFileSaveAs()
{
    Q_ASSERT(m_object);
    QString path = QFileDialog::getSaveFileName(this, tr("Save Budget"), m_lastSavedPath, tr("Budget Files (*.buddb)"));
    if (path.isEmpty())
        return false;
    if (!m_object->saveBudget(path)) {
        QMessageBox::critical(this, tr("Error"), tr("Error while saving the budget. Try again later"));
        return false;
    }
    m_lastSavedPath = path;
    return true;
}

bool MainWindow::onFileLoad()
{
    Q_ASSERT(m_object);
    QString path = QFileDialog::getOpenFileName(this, tr("Open Budget"), m_lastSavedPath, tr("Budget Files (*.buddb)"));
    if (path.isEmpty())
        return false;
    if (!m_object->loadBudget(path)) {
        QMessageBox::critical(this, tr("Error"), tr("Error while loading the budget. The file might be currupted"));
        return false;
    }
    m_lastSavedPath = path;
    return true;
}
