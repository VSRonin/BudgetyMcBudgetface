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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
namespace Ui {
class MainWindow;
}
class MainObject;
class SettingsDialog;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onAboutQt();
    void onFileNew();
    bool onFileSave();
    bool onFileSaveAs();
    bool onFileLoad();
    void onFileExit();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QString m_lastSavedPath;
    MainObject *m_object;
    SettingsDialog *m_settingsDialog;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
