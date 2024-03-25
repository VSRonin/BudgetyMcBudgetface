#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <mainobject.h>
#include "addaccountdialog.h"
#include <QMessageBox>
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_object(nullptr)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(ui->addAccountButton, &QPushButton::clicked, this, &SettingsDialog::onAddAccount);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    ui->accountsView->setModel(m_object->accountsModel());
    ui->categoriesView->setModel(m_object->categoriesModel());
}

void SettingsDialog::onAddAccount()
{
    Q_ASSERT(m_object);
    AddAccountDialog addAccountDialog(this);
    addAccountDialog.loadCombos(m_object->currenciesModel(), m_object->accountTypesModel());
    while (addAccountDialog.exec()) {
        if (m_object->addAccount(addAccountDialog.name(), addAccountDialog.owner(), addAccountDialog.curr(), addAccountDialog.typ()))
            break;
        QMessageBox::critical(this, tr("Error"), tr("Failed to add a new account, try again or check your input"));
    }
}
