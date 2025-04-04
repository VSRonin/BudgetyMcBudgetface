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

#ifndef GLOBALS_H
#define GLOBALS_H
#define BUDGET_FILE_VERSION QByteArrayLiteral("1.0.0")
#include <QObject>
#include <QString>
#include <QSqlDatabase>
inline bool check_true_helper(bool cond) noexcept
{
    return cond;
}
#define CHECK_TRUE(Expr)                                                                                                                             \
    [](bool valueOfExpression) {                                                                                                                     \
        Q_ASSERT_X(valueOfExpression, "CHECK_TRUE()", "Assumption in CHECK_TRUE(\"" #Expr "\") was not correct");                                    \
    }(check_true_helper(Expr))
void discardDbFile();
void createDbFile();
QSqlDatabase openDb();
void closeDb();
QString dbFilePath();
#endif
