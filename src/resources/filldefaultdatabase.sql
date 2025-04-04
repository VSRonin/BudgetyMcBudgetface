BEGIN TRANSACTION;
CREATE TABLE Currencies (Id INTEGER PRIMARY KEY, Currency TEXT NOT NULL);
CREATE TABLE ExchangeRates (
    FromCurrency TEXT NOT NULL,
    ToCurrency TEXT NOT NULL,
    ExchangeRate REAL NOT NULL,
    PRIMARY KEY (FromCurrency, ToCurrency)
);
CREATE TABLE AccountTypes (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL);
CREATE TABLE Family (
    Id INTEGER PRIMARY KEY,
    Name TEXT NOT NULL UNIQUE,
    Birthday TEXT NOT NULL,
    TaxableIncome REAL NOT NULL,
    IncomeCurrency INTEGER NOT NULL,
    RetirementAge INTEGER DEFAULT 67 NOT NULL,
    FOREIGN KEY (IncomeCurrency) REFERENCES Currencies (Id)
);
CREATE TABLE Accounts (
    Id INTEGER PRIMARY KEY,
    Name TEXT NOT NULL UNIQUE,
    Owner TEXT NOT NULL,
    Currency INTEGER NOT NULL,
    AccountType INTEGER NOT NULL,
    AccountStatus INTEGER DEFAULT 1 NOT NULL,
    FOREIGN KEY (Currency) REFERENCES Currencies (Id),
    FOREIGN KEY (AccountType) REFERENCES AccountTypes (Id),
    FOREIGN KEY (AccountStatus) REFERENCES AccountStatuses (Id)
);
CREATE TABLE AccountStatuses (Id INTEGER PRIMARY KEY);
CREATE TABLE MovementTypes (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL);
CREATE TABLE Categories (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL);
CREATE TABLE Subcategories (
    Id INTEGER PRIMARY KEY,
    Category INTEGER NOT NULL,
    Name TEXT NOT NULL,
    NeedWantSave INTEGER NOT NULL,
    FOREIGN KEY (Category) REFERENCES Categories (Id)
);
CREATE TABLE Transactions (
    Id INTEGER PRIMARY KEY,
    Account INTEGER NOT NULL,
    OperationDate TEXT NOT NULL,
    Currency INTEGER NOT NULL,
    Amount REAL NOT NULL,
    PaymentType TEXT,
    Description TEXT,
    Category INTEGER,
    Subcategory INTEGER,
    MovementType INTEGER,
    DestinationAccount INTEGER,
    ExchangeRate REAL,
    FOREIGN KEY (Currency) REFERENCES Currencies (Id),
    FOREIGN KEY (Account) REFERENCES Accounts (Id),
    FOREIGN KEY (Category) REFERENCES Categories (Id),
    FOREIGN KEY (Subcategory) REFERENCES Subcategories (Id),
    FOREIGN KEY (MovementType) REFERENCES MovementTypes (Id),
    FOREIGN KEY (DestinationAccount) REFERENCES Accounts (Id)
);
INSERT INTO AccountStatuses (Id) VALUES (1),(0);
INSERT INTO Currencies (Id, Currency) VALUES (1,'GBP'),(2,'EUR'),(3,'USD'),(4,'CHF');
INSERT INTO ExchangeRates (FromCurrency, ToCurrency, ExchangeRate) VALUES
    ('GBP','EUR',1.0),
    ('GBP','USD',1.0),
    ('GBP','CHF',1.0),
    ('EUR','GBP',1.0),
    ('EUR','USD',1.0),
    ('EUR','CHF',1.0),
    ('USD','EUR',1.0),
    ('USD','GBP',1.0),
    ('USD','CHF',1.0),
    ('CHF','EUR',1.0),
    ('CHF','USD',1.0),
    ('CHF','GBP',1.0)
;
INSERT INTO AccountTypes (Id, Name) VALUES (1,'Current Account'),(2,'Credit Card'),(3,'Expense Account'),(4,'Saving/Investment'),(5,'Debt');
INSERT INTO MovementTypes (Id, Name) VALUES (1,'Income'),(2,'Expense'),(3,'Refund'),(4,'Deposit'),(5,'Repayment'),(6,'Withdrawal');
INSERT INTO Categories (Id, Name) VALUES
    (0,'Internal Transfer'),
    (1,'Income'),
    (2,'Work Expense'),
    (3,'Food'),
    (4,'Healthcare'),
    (5,'Housing'),
    (6,'Insurance'),
    (7,'Others'),
    (8,'Taxes'),
    (9,'Transport'),
    (10,'Utilities'),
    (11,'Beauty'),
    (12,'Clothes'),
    (13,'Electronics'),
    (14,'Baby'),
    (15,'Gifts'),
    (16,'Holidays'),
    (17,'Subscriptions'),
    (18,'Investment'),
    (19,'Debt'),
    (20,'Car')
;
INSERT INTO Subcategories (Id, Category, Name, NeedWantSave) VALUES
    (0,0,'Internal Transfer',1),
    (1,1,'Salary',1),
    (2,2,'Expense',1),
    (3,3,'Groceries',1),
    (4,4,'Dental',1),
    (5,4,'Drugs',1),
    (6,4,'Insurance',1),
    (7,4,'Optics',1),
    (8,5,'Cleaning',2)
    (9,5,'Rent',1),
    (10,5,'Maintenance',1),
    (11,6,'Insurance',1),
    (12,7,'Bureaucracy',1),
    (13,8,'Taxes',1),
    (14,9,'Underground',1),
    (15,10,'Electric',1),
    (16,10,'Gas',1),
    (17,10,'Internet',1),
    (18,10,'Phone',1),
    (19,10,'TV License',1),
    (20,10,'Water',1),
    (21,11,'Hair',2),
    (22,11,'Makeup',2),
    (23,11,'Nails',2),
    (24,11,'Skincare',2),
    (25,12,'Accessories',2),
    (26,12,'Clothes',2),
    (27,13,'Phone',2),
    (28,13,'Videogames',2),
    (29,14,'Accessories',1),
    (30,14,'Clothes',1),
    (31,14,'Education',1),
    (32,3,'Restaurant',2),
    (33,3,'Snack',2),
    (34,3,'Take Away',2),
    (35,15,'Gifts',2),
    (36,4,'Other',2),
    (37,16,'Food',2),
    (38,16,'Hotel',2),
    (39,16,'Other',2),
    (40,16,'Transport',2),
    (41,5,'Furniture',2),
    (42,5,'Other',2),
    (43,7,'Dry Cleaning',2),
    (44,7,'Entertainment',2),
    (45,7,'Moving/Disposal',2),
    (46,7,'Pocket Cash',2),
    (47,7,'Education',1),
    (48,7,'Unknown',2),
    (49,17,'Delivery',2),
    (50,17,'Gym',2),
    (51,17,'Password Manager',2),
    (52,17,'Streaming',2),
    (53,9,'Airplane',2),
    (54,9,'Bus',2),
    (55,9,'Taxi',2),
    (56,9,'Train',2),
    (57,18,'Investment',0),
    (58,19,'Debt',0),
    (59,20,'Fuel',1),
    (60,20,'Insurance',1),
    (61,20,'Repair',1),
    (62,20,'Tax',1),
    (63,8,'Council Tax',1),
    (64,1,'Investment Income',1)
;
COMMIT;
