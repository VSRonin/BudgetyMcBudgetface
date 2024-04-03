#include "selectaccountdialog.h"
#include "ui_selectaccountdialog.h"
#include <mainobject.h>
SelectAccountDialog::SelectAccountDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectAccountDialog)
{
    ui->setupUi(this);
}

SelectAccountDialog::~SelectAccountDialog()
{
    delete ui;
}

void SelectAccountDialog::setMainObject(MainObject *mainObj)
{
    m_object = mainObj;
    if (m_object)
        ui->accountCombo->setModel(m_object->accountsModel());
    ui->accountCombo->setModelColumn(MainObject::acName);
}

int SelectAccountDialog::selectedAccountId() const
{
    if (!m_object)
        return -1;
    return m_object->accountsModel()->index(ui->accountCombo->currentIndex(), MainObject::acId).data().toInt();
}
