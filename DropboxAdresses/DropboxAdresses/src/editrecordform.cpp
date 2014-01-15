#include "editrecordform.h"
#include "ui_editrecordform.h"
#include <QPushButton>

EditRecordForm::EditRecordForm(QString* _name, QString* _adress, QString* _phone, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditRecordForm)
{
    ui->setupUi(this);

    name = _name;
    ui->nameEdit->setText(*name);
    adress = _adress;
    ui->adressEdit->setText(*adress);
    phone = _phone;
    ui->phoneEdit->setText(*phone);

    if(*name == "")
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(nameChanged()));
    connect(ui->adressEdit, SIGNAL(textChanged(QString)), this, SLOT(adressChanged()));
    connect(ui->phoneEdit, SIGNAL(textChanged(QString)), this, SLOT(phoneChanged()));
}

EditRecordForm::~EditRecordForm()
{
    delete ui;
}

void EditRecordForm::nameChanged()
{
    if(ui->nameEdit->text() == "")
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    *name = ui->nameEdit->text();
}

void EditRecordForm::adressChanged()
{
    *adress = ui->adressEdit->text();
}

void EditRecordForm::phoneChanged()
{
    *phone = ui->phoneEdit->text();
}
