#include "adressmenu.h"
#include "ui_adressmenu.h"
#include <QMessageBox>

AdressMenu::AdressMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdressMenu)
{
    ui->setupUi(this);
    connect(
                ui->signOutButton,
                SIGNAL(clicked()),
                this,
                SLOT(signOutButClicked())
                );
    connect(
                ui->addButton,
                SIGNAL(clicked()),
                this,
                SIGNAL(addRecordRequested())
                );
    connect(
                ui->delButton,
                SIGNAL(clicked()),
                this,
                SIGNAL(deleteRecordRequested())
                );
    connect(
                ui->editButton,
                SIGNAL(clicked()),
                this,
                SIGNAL(editRecordRequested())
                );
    ui->delButton->setEnabled(false);
    ui->editButton->setEnabled(false);
    // QMovie initialization
    loading = new QMovie(
        ":/animation/resources/loading.gif",
        QByteArray(),
        this
    );
    ui->loadingLabel->setMovie(loading);
    ui->loadingLabel->setAlignment(Qt::AlignCenter);
}

AdressMenu::~AdressMenu()
{
    delete ui;
}

void AdressMenu::signOutButClicked()
{
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        "DropboxAdressBook",
        "Вы точно хотите выйти?",
        QMessageBox::Yes|QMessageBox::No,
        QMessageBox::No
        );
    if(result != QMessageBox::Yes)
    {
        return;
    }

    emit signOutRequested();
}

void AdressMenu::setAddEnable(bool val)
{
    ui->addButton->setEnabled(val);
}

void AdressMenu::setDelEnable(bool val)
{
    ui->delButton->setEnabled(val);
}

void AdressMenu::setEditEnable(bool val)
{
    ui->editButton->setEnabled(val);
}

void AdressMenu::startMove()
{
    ui->loadingLabel->show();
    loading->start();
}

void AdressMenu::stopMove()
{
    ui->loadingLabel->hide();
    loading->stop();
}
