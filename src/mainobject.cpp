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
#include "mainobject.h"
#include "globals.h"
#include "offlinesqltable.h"
#include <QSqlDatabase>
#include <QSqlQuery>
MainObject::MainObject(QObject *parent)
    : QObject(parent)
    , m_transactionsModel(new OfflineSqlTable(this))
{

}

MainObject::~MainObject()
{

}

void MainObject::newBudget()
{
    discardDbFile();
    createDbFile();
    QSqlDatabase budgetDB = openDb();
    QSqlQuery createCurrencyTable(budgetDB);
    createCurrencyTable.prepare(QStringLiteral("CREATE TABLE Currencies (Id INTEGER PRIMARY KEY, Currency TEXT NOT NULL)"));
    Q_ASSUME(createCurrencyTable.exec());
    QSqlQuery createAccountTypeTable(budgetDB);
    createAccountTypeTable.prepare(QStringLiteral("CREATE TABLE AccountTypes (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL)"));
    Q_ASSUME(createAccountTypeTable.exec());
    QSqlQuery createAccountsTable(budgetDB);
    createAccountsTable.prepare(QStringLiteral("CREATE TABLE Accounts (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL, Owner TEXT NOT NULL, Currency INTEGER NOT NULL, AccountType INTEGER NOT NULL, FOREIGN KEY (Currency) REFERENCES Currencies (Id), FOREIGN KEY (AccountType) REFERENCES AccountTypes (Id))"));
    Q_ASSUME(createAccountsTable.exec());
    QSqlQuery createMoevementTypeTable(budgetDB);
    createMoevementTypeTable.prepare(QStringLiteral("CREATE TABLE MoevementTypes (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL)"));
    Q_ASSUME(createMoevementTypeTable.exec());
    QSqlQuery createCategoryTable(budgetDB);
    createCategoryTable.prepare(QStringLiteral("CREATE TABLE Categories (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL)"));
    Q_ASSUME(createCategoryTable.exec());
    QSqlQuery createSubcategoryTable(budgetDB);
    createSubcategoryTable.prepare(QStringLiteral("CREATE TABLE Subcategories (Id INTEGER PRIMARY KEY, Category INTEGER NOT NULL, Name TEXT NOT NULL, NeedWantSave INTEGER NOT NULL, FOREIGN KEY (Category) REFERENCES Categories (Id))"));
    Q_ASSUME(createSubcategoryTable.exec());
    QSqlQuery createTransactionsTable(budgetDB);
    createTransactionsTable.prepare(QStringLiteral("CREATE TABLE Transactions (Id INTEGER PRIMARY KEY, Account INTEGER NOT NULL, Date TEXT NOT NULL, Currency INTEGER NOT NULL, Amount REAL NOT NULL, PaymentType TEXT, Description TEXT, Category INT, Subcategory INT, MovementType INT, FOREIGN KEY (Currency) REFERENCES Currencies (Id), FOREIGN KEY (Account) REFERENCES Accounts (Id), FOREIGN KEY (Category) REFERENCES Categories (Id), FOREIGN KEY (Subcategory) REFERENCES Subcategories (Id), FOREIGN KEY (MoevementTyp) REFERENCES MoevementTypes (Id))"));
    Q_ASSUME(createTransactionsTable.exec());
    fillDefaultDbFields();

    m_transactionsModel->setTable(QStringLiteral("Transactions"));
}

void MainObject::fillDefaultDbFields()
{
    QSqlDatabase budgetDB = openDb();
    QSqlQuery fillCurrencyTable(budgetDB);
    fillCurrencyTable.prepare(QStringLiteral("INSERT INTO Currencies (Id, Currency) VALUES (1,'GBP'),(2,'EUR'),(3,'USD')"));
    Q_ASSUME(fillCurrencyTable.exec());
    QSqlQuery fillAccountTypeTable(budgetDB);
    fillAccountTypeTable.prepare(QStringLiteral("INSERT INTO AccountTypes (Id, Name) VALUES (1,'Current Account'),(2,'ISA'),(3,'GIA')"));
    Q_ASSUME(fillAccountTypeTable.exec());
    QSqlQuery fillMoevementTypeTable(budgetDB);
    fillMoevementTypeTable.prepare(QStringLiteral("INSERT INTO MoevementTypes (Id, Name) VALUES (1,'Income'),(2,'Expense'),(3,'Deposit'),(4,'Withdrawal'),(5,'Refund')"));
    Q_ASSUME(fillMoevementTypeTable.exec());
    QSqlQuery fillCategoryTable(budgetDB);
    fillCategoryTable.prepare(QStringLiteral("INSERT INTO Categories (Id, Name) VALUES (0,Internal Transfer), (1,Income), (2,Work Expense), (3,Food), (4,Healthcare), (5,Housing), (6,Insurance), (7,Others), (8,Taxes), (9,Transport), (10,Utilities), (11,Beauty), (12,Clothes), (13,Electronics), (14,Baby), (15,Gifts), (16,Holidays), (17,Subscriptions), (18,Investment)"));
    Q_ASSUME(fillCategoryTable.exec());
    QSqlQuery fillSubcategoryTable(budgetDB);
    fillSubcategoryTable.prepare(QStringLiteral("INSERT INTO Subcategories (Id, Category, Name, NeedWantSave) VALUES (0,0,'Internal Transfer',1), (1,1,'Salary',1), (2,2,'Expense',1), (3,3,'Groceries',1), (4,4,'Dental',1), (5,4,'Drugs',1), (6,4,'Insurance',1), (7,4,'Optics',1), (8,5,'Council Tax',1), (9,5,'Rent',1), (10,5,'Maintenance',1), (11,6,'Insurance',1), (12,7,'Bureaucracy',1), (13,8,'Taxes',1), (14,9,'Underground',1), (15,10,'Electric',1), (16,10,'Gas',1), (17,10,'Internet',1), (18,10,'Phone',1), (19,10,'TV License',1), (20,10,'Water',1), (21,11,'Hair',2), (22,11,'Makeup',2), (23,11,'Nails',2), (24,11,'Skincare',2), (25,12,'Accessories',2), (26,12,'Clothes',2), (27,13,'Phone',2), (28,13,'Videogames',2), (29,14,'Accessories',1), (30,14,'Clothes',1), (31,14,'Education',1), (32,3,'Restaurant',2), (33,3,'Snack',2), (34,3,'Take Away',2), (35,15,'Gifts',2), (36,4,'Other',2), (37,16,'Food',2), (38,16,'Hotel',2), (39,16,'Other',2), (40,16,'Transport',2), (41,5,'Furniture',2), (42,5,'Other',2), (43,7,'Dry Cleaning',2), (44,7,'Entertainment',2), (45,7,'Moving/Disposal',2), (46,7,'Pocket Cash',2), (47,7,'Education',1), (48,7,'Unknown',2), (49,17,'Delivery',2), (50,17,'Gym',2), (51,17,'Password Manager',2), (52,17,'Streaming',2), (53,9,'Airplane',2), (54,9,'Bus',2), (55,9,'Taxi',2), (56,9,'Train',2), (57,18,'GIA',0), (58,18,'ISA',0)"));
    Q_ASSUME(fillSubcategoryTable.exec());
}
