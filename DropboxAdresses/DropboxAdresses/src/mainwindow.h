#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTableWidget>
#include <QHeaderView>
#include <QFile>
#include <QBuffer>
#include "signinmenu.h"
#include "adressmenu.h"
#include "editrecordform.h"
#include <QAction>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void closeEvent(QCloseEvent *event);
    ~MainWindow();

public slots:
    void getNetworkRequest(QNetworkRequest *networkRequest);
    void putNetworkRequest(QNetworkRequest *networkRequest, QIODevice *data);
    void handleNetworkReply(QNetworkReply *networkReply);
    void handleDownloadReadyRead();
private slots:
    void attemptSignIn();
    void signOut();
    void addRecord();
    void delRecord();
    void editRecord();
    void uploaded();
    void downloaded();
    void rowSelected();
    void setupActions();
    void sort_data(int i, Qt::SortOrder order);
    void criticalExit();

public slots:
    void downloadAdresses();
    void uploadAdresses();

signals:
    void networkRequestGetNeeded(QNetworkRequest *request);
    void networkRequestPutNeeded(QNetworkRequest *request, QIODevice *data);
    void fileUploaded();
    void fileDownloaded();
    void criticalExitRequest();

private:
    QTableWidget* adressTable;
    SignInMenu* sMenu;
    AdressMenu* aMenu;
    QFile *adressesFile;
    QString fileName;
    bool uploading;
    bool downloading;
    QBuffer* buf;
    QNetworkReply *downloadNetworkReply;

    QAction* addAction;
    QAction* delAction;
    QAction* editAction;
    QAction* exitAction;
};

#endif // MAINWINDOW_H
