#include "mainwindow.h"
#include "common.h"
#include "dropbox.h"
#include "json.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QBuffer>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStatusBar>
#include <iostream>
#include <QTableWidgetSelectionRange>

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent)
{
    fileName = "db_adresses.xml";
    uploading = false;
    downloading = false;
    buf = new QBuffer();

    QWidget* centralWidget = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    adressTable = new QTableWidget(0, 3, this);
    QTableWidgetItem* headerItem = new QTableWidgetItem(tr("ФИО"));
    adressTable->setHorizontalHeaderItem(0, headerItem);
    headerItem = new QTableWidgetItem(tr("Адрес"));
    adressTable->setHorizontalHeaderItem(1, headerItem);
    headerItem = new QTableWidgetItem(tr("Телефон"));
    adressTable->setHorizontalHeaderItem(2, headerItem);
    QHeaderView *headerView = new QHeaderView(Qt::Horizontal, adressTable);
    adressTable->setHorizontalHeader(headerView);
    headerView->setSectionResizeMode(0, QHeaderView::Stretch);
    headerView->setSectionResizeMode(1, QHeaderView::Stretch);
    headerView->setSectionResizeMode(2, QHeaderView::Stretch);
    adressTable->horizontalHeader()->setSectionsClickable(true);
    adressTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    adressTable->setSelectionMode(QAbstractItemView::SingleSelection);
    adressTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    mainLayout->addWidget(adressTable);
    adressTable->hide();
    sMenu = new SignInMenu();
    mainLayout->addWidget(sMenu);
    aMenu = new AdressMenu();
    mainLayout->addWidget(aMenu);
    aMenu->hide();
    this->setCentralWidget(centralWidget);

    setupActions();

    connect(
        adressTable->horizontalHeader(),
        SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
        this,
        SLOT(sort_data(int,Qt::SortOrder))
        );
    connect(
        Common::networkAccessManager,
        SIGNAL(finished(QNetworkReply*)),
        SLOT(handleNetworkReply(QNetworkReply*))
        );
    connect(
        sMenu,
        SIGNAL(networkRequestGetNeeded(QNetworkRequest*)),
        SLOT(getNetworkRequest(QNetworkRequest*))
        );
    connect(
        sMenu,
        SIGNAL(oauthAccesstokenHandled()),
        SLOT(attemptSignIn())
        );
    connect(
        aMenu,
        SIGNAL(signOutRequested()),
        SLOT(signOut())
        );
    connect(
        aMenu,
        SIGNAL(addRecordRequested()),
        SLOT(addRecord())
        );
    connect(
        aMenu,
        SIGNAL(deleteRecordRequested()),
        SLOT(delRecord())
        );
    connect(
        aMenu,
        SIGNAL(editRecordRequested()),
        SLOT(editRecord())
        );
    connect(
        this,
        SIGNAL(networkRequestPutNeeded(QNetworkRequest*,QIODevice*)),
        SLOT(putNetworkRequest(QNetworkRequest*,QIODevice*))
        );
    connect(
        this,
        SIGNAL(networkRequestGetNeeded(QNetworkRequest*)),
        SLOT(getNetworkRequest(QNetworkRequest*))
        );
    connect(
        this,
        SIGNAL(fileUploaded()),
        this,
        SLOT(uploaded())
        );
    connect(
        this,
        SIGNAL(fileDownloaded()),
        this,
        SLOT(downloaded())
        );
    connect(
        adressTable,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(rowSelected())
        );
    connect(
        this,
        SIGNAL(criticalExitRequest()),
        this,
        SLOT(criticalExit())
        );
    attemptSignIn();
}

MainWindow::~MainWindow()
{

}

void MainWindow::getNetworkRequest(QNetworkRequest *networkRequest)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkRequest->url());
    downloadNetworkReply = Common::networkAccessManager->get(*networkRequest);
    switch(api)
    {
    case Dropbox::FILES:
        connect(
            downloadNetworkReply,
            SIGNAL(readyRead()),
            SLOT(handleDownloadReadyRead())
            );
        break;
    }
}

void MainWindow::putNetworkRequest(
    QNetworkRequest *networkRequest,
    QIODevice *data
    )
{
    QNetworkReply *networkReply = Common::networkAccessManager->put(*networkRequest, data);
}

void MainWindow::handleNetworkReply(QNetworkReply *networkReply)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkReply->url());

    networkReply->deleteLater();

    if(networkReply->error() != QNetworkReply::NoError &&
       networkReply->error() != QNetworkReply::OperationCanceledError)
    {
        QString replyData = networkReply->readAll();
        QVariantMap jsonResult = QtJson::Json::parse(replyData).toMap();

        if(jsonResult.contains("error"))
        {
            if(jsonResult["error"].type() != QVariant::Map)
            {
                QMessageBox::critical(
                    this,
                    "DropBoxAdressBook",
                    jsonResult["error"].toString()
                    );
                return;
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "DropBoxAdressBook",
                    QString(
                        jsonResult["error"].toMap().keys().first() +
                        ": " +
                        jsonResult["error"].toMap()
                                .values().first().toString()
                        )
                    );
                return;
            }
        }
        else
        {
            if(!replyData.isEmpty())
            {
                QMessageBox::critical(
                    this,
                    "DropBoxAdressBook",
                    replyData
                );
                return;
            }
            else
            {
                if(networkReply->error() == QNetworkReply::ContentNotFoundError)
                {
                    uploadAdresses();
                }
                else
                {
                    QMessageBox::critical(
                        this,
                        "DropBoxAdressBook",
                        networkReply->errorString()
                        );
                    emit criticalExit();
                }
                return;
            }
        }
    }

    switch(api)
    {
    case Dropbox::OAUTH_REQUESTTOKEN:
    case Dropbox::OAUTH_ACCESSTOKEN:
        sMenu->handleNetworkReply(networkReply);
        break;
    case Dropbox::FILES:
        if(networkReply->error() == QNetworkReply::NoError)
        {
            emit fileDownloaded();
        }
    case Dropbox::FILESPUT:
        if(networkReply->error() == QNetworkReply::NoError)
        {
            emit fileUploaded();
        }
        break;

    default:
        break;
    }
}

void MainWindow::attemptSignIn()
{
    if( !Common::userData->token.isEmpty() &&
        !Common::userData->secret.isEmpty() &&
        !Common::userData->uid.isEmpty() )
    {
        sMenu->hide();
        aMenu->show();
        adressTable->show();
        downloadAdresses();
        QApplication::processEvents();
        adjustSize();
    }
}

void MainWindow::signOut()
{
    // remove old user data
    uploadAdresses();
    QSettings settings;
    settings.remove("user_data");
    Common::userData->token.clear();
    Common::userData->secret.clear();
    Common::userData->uid.clear();
    aMenu->startMove();
    adressTable->setRowCount(0);
}

void MainWindow::addRecord()
{
    QString name;
    QString adress;
    QString phone;
    EditRecordForm form(&name, &adress, &phone);
    if(form.exec() == QDialog::Accepted)
    {
        adressTable->insertRow(adressTable->rowCount());
        QTableWidgetItem* item = new QTableWidgetItem(name);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        adressTable->setItem(adressTable->rowCount() - 1, 0, item);
        item = new QTableWidgetItem(adress);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        adressTable->setItem(adressTable->rowCount() - 1, 1, item);
        item = new QTableWidgetItem(phone);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        adressTable->setItem(adressTable->rowCount() - 1, 2, item);
        adressTable->selectRow(adressTable->rowCount() - 1);
    }
}

void MainWindow::editRecord()
{
    QString name = adressTable->item(adressTable->currentRow(), 0)->data(Qt::DisplayRole).toString();
    QString adress = adressTable->item(adressTable->currentRow(), 1)->data(Qt::DisplayRole).toString();
    QString phone = adressTable->item(adressTable->currentRow(), 2)->data(Qt::DisplayRole).toString();
    EditRecordForm form(&name, &adress, &phone);
    if(form.exec() == QDialog::Accepted)
    {
        QTableWidgetItem* item = new QTableWidgetItem(name);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        adressTable->setItem(adressTable->currentRow(), 0, item);
        item = new QTableWidgetItem(adress);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        adressTable->setItem(adressTable->currentRow(), 1, item);
        item = new QTableWidgetItem(phone);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        adressTable->setItem(adressTable->currentRow(), 2, item);
        adressTable->selectRow(adressTable->currentRow());
    }
}

void MainWindow::delRecord()
{
    int curRow = adressTable->currentRow();
    adressTable->removeRow(curRow);
    if(adressTable->rowCount() == 0)
    {
        aMenu->setDelEnable(false);
        aMenu->setEditEnable(false);
    }
    else
    {
        adressTable->selectRow(curRow);
    }
}

 void MainWindow::downloadAdresses()
 {
     buf->open(QBuffer::WriteOnly);

     // request the content of the remote file
     QUrl url = Common::dropbox->apiToUrl(
         Dropbox::FILES
         ).toString() + "/" + fileName;
     QNetworkRequest networkRequest(url);
     Common::oAuth->signRequestHeader("GET", &networkRequest);
     aMenu->startMove();
     this->statusBar()->showMessage(tr("Синхронизация..."), 3000);
     emit networkRequestGetNeeded(&networkRequest);
 }

 void MainWindow::uploadAdresses()
 {
     if(buf)
         delete buf;
     buf = new QBuffer;
     buf->open(QBuffer::WriteOnly);
     QXmlStreamWriter xmlWriter(buf);
     xmlWriter.setAutoFormatting(true);
     xmlWriter.writeStartDocument();
     xmlWriter.writeStartElement("adress_book");
     for( int i = 0 ; i < adressTable->rowCount() ; i++)
     {
         xmlWriter.writeStartElement("person");
         xmlWriter.writeAttribute(
                     "name",
                     adressTable->item(i, 0)->data(Qt::DisplayRole).toString()
                     );
         xmlWriter.writeAttribute(
                     "adress",
                     adressTable->item(i, 1)->data(Qt::DisplayRole).toString()
                     );
         xmlWriter.writeAttribute(
                     "phone",
                     adressTable->item(i, 2)->data(Qt::DisplayRole).toString()
                     );
         xmlWriter.writeEndElement();
     }
     xmlWriter.writeEndElement();
     xmlWriter.writeEndDocument();
     buf->close();

     // send the content of the local file
     QUrl url;
     url = Common::dropbox->apiToUrl(Dropbox::FILESPUT).toString() + "/" + fileName;
     QUrlQuery query(url);
     query.addQueryItem("overwrite", "true");
     url.setQuery(query);

     QNetworkRequest networkRequest(url);
     Common::oAuth->signRequestHeader("PUT", &networkRequest);
     networkRequest.setRawHeader("Content-Length", QString("%1").arg(buf->size()).toLatin1());
     uploading = true;
     emit networkRequestPutNeeded(&networkRequest, buf);
 }

 void MainWindow::uploaded()
 {
    uploading = false;
    if( Common::userData->token.isEmpty() &&
        Common::userData->secret.isEmpty() &&
        Common::userData->uid.isEmpty() )
    {
        sMenu->show();
        aMenu->hide();
        adressTable->hide();
        QApplication::processEvents();
        adjustSize();
    }
    aMenu->stopMove();
 }

 void MainWindow::closeEvent(QCloseEvent *event)
 {
     if( !Common::userData->token.isEmpty() &&
         !Common::userData->secret.isEmpty() &&
         !Common::userData->uid.isEmpty() )
     {
        uploadAdresses();
        this->statusBar()->showMessage(tr("Синхронизация..."), 3000);
     }
     aMenu->startMove();
     while(uploading)
     {
         QApplication::processEvents();
     }
     aMenu->stopMove();
     close();
 }

 void MainWindow::handleDownloadReadyRead()
 {
     buf->write(downloadNetworkReply->readAll());
 }

 void MainWindow::downloaded()
 {
     buf->open(QBuffer::ReadOnly);
     QXmlStreamReader xmlReader(buf);
     xmlReader.readNext();
     int counter = 0;
     while(!xmlReader.atEnd())
     {
         if(xmlReader.isStartElement())
         {
             if(xmlReader.name() == "person")
             {
                 QXmlStreamAttributes attrs = xmlReader.attributes();
                 adressTable->insertRow(adressTable->rowCount());
                 QTableWidgetItem* item = new QTableWidgetItem(attrs.value("name").toString());
                 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                 adressTable->setItem(adressTable->rowCount() - 1, 0, item);
                 item = new QTableWidgetItem(attrs.value("adress").toString());
                 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                 adressTable->setItem(adressTable->rowCount() - 1, 1, item);
                 item = new QTableWidgetItem(attrs.value("phone").toString());
                 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                 adressTable->setItem(adressTable->rowCount() - 1, 2, item);
                 counter++;
              }
         }
         xmlReader.readNext();
         if(counter % 100 == 0)
            QApplication::processEvents();
     }
     if(xmlReader.hasError())
     {
         std::cerr<<"Error: failed to parse file"<<qPrintable(fileName)<<": "<<qPrintable(xmlReader.errorString())<< std::endl;
     }
     buf->close();
     aMenu->stopMove();
     adressTable->sortByColumn(0, Qt::AscendingOrder);
     adressTable->setSortingEnabled(true);
 }

 void MainWindow::rowSelected()
 {
     aMenu->setDelEnable(true);
     aMenu->setEditEnable(true);
 }

 void MainWindow::setupActions()
 {
     addAction = new QAction(tr("Добавить запись"), adressTable);
     connect(addAction, SIGNAL(triggered()), this, SLOT(addRecord()));
     adressTable->addAction(addAction);
     editAction = new QAction(tr("Редактировать"), adressTable);
     connect(editAction, SIGNAL(triggered()), this, SLOT(editRecord()));
     adressTable->addAction(editAction);
     delAction = new QAction(tr("Удалить"), adressTable);
     connect(delAction, SIGNAL(triggered()), this, SLOT(delRecord()));
     adressTable->addAction(delAction);
     exitAction = new QAction(tr("Выйти"), adressTable);
     connect(exitAction, SIGNAL(triggered()), this, SLOT(signOut()));
     adressTable->addAction(exitAction);
 }

 void MainWindow::sort_data(int i, Qt::SortOrder order)
 {
     adressTable->sortByColumn(i, order);
 }

 void MainWindow::criticalExit()
 {
     uploading = false;
     exit(0);
 }

