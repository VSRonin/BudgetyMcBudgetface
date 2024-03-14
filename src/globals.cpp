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
#include "globals.h"
#include <QStandardPaths>
#include <QDir>
#define DATABASE_NAME QStringLiteral("BudgetDB")
QString makeStandardLocation(QStandardPaths::StandardLocation loc)
{
    const QString stdLocation = QStandardPaths::writableLocation(loc);
    Q_ASSERT(!stdLocation.isEmpty());
    Q_ASSUME(QDir().mkpath(stdLocation));
    return stdLocation;
}

QString appDataPath()
{
    return makeStandardLocation(QStandardPaths::AppLocalDataLocation);
}

QString appSettingsPath()
{
    return makeStandardLocation(QStandardPaths::AppConfigLocation);
}

QString dbFilePath()
{
    return appDataPath() + QDir::separator() + QLatin1String("currentbudget.sqlite");
}

QSqlDatabase openDb()
{
    QSqlDatabase db = QSqlDatabase::database(DATABASE_NAME, false);
    Q_ASSERT(db.isValid());
    bool DbOpen = db.isOpen();
    if (!DbOpen)
        DbOpen = db.open();
    Q_ASSERT(DbOpen);
    return db;
}

void discardDbFile()
{
    QSqlDatabase db = QSqlDatabase::database(DATABASE_NAME, false);
    if (db.isValid()) {
        if (db.isOpen())
            db.close();
        QSqlDatabase::removeDatabase(DATABASE_NAME);
    }
    const QString dbFileName = dbFilePath();
    if (QFile::exists(dbFileName))
        QFile::remove(dbFileName);
}

void createDbFile()
{
    const QString destinationDB = dbFilePath();
    QFile::copy(QStringLiteral(":/db/defaultdb.sqlite"), destinationDB);
    QSqlDatabase db = QSqlDatabase::database(DATABASE_NAME, false);
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), DATABASE_NAME);
        db.setDatabaseName(destinationDB);
    }
}
