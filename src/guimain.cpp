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
#include <QApplication>
#include <mainwindow.h>
#include <globals.h>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(backendresources);
    Q_INIT_RESOURCE(uiresources);
    app.setStyle(QStringLiteral("fusion"));
    MainWindow w;
    w.show();
    QObject::connect(&app, &QApplication::lastWindowClosed, []() { discardDbFile(); });
    return app.exec();
}
