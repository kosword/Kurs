#ifndef SIGNINMENU_H
#define SIGNINMENU_H

#include <QWidget>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "common.h"
#include "dropbox.h"
#include "oauth.h"
#include <QMessageBox>

namespace Ui {
class SignInMenu;
}

class SignInMenu : public QWidget
{
    Q_OBJECT

public:
    explicit SignInMenu(QWidget *parent = 0);
    ~SignInMenu();

private slots:
    void openDropboxInABrowser();
signals:
    void networkRequestGetNeeded(QNetworkRequest *request);
    void oauthAccesstokenHandled();
public slots:
    void requestOauthRequesttoken();
    void requestOauthAccesstoken();
    void handleNetworkReply(QNetworkReply *networkReply);
    void handleOauthRequesttoken(QNetworkReply *networkReply);
    void handleOauthAccesstoken(QNetworkReply *networkReply);
private slots:
    void signInButtonClicked();
    void doneButtonClicked();

private:
    Ui::SignInMenu *ui;
    UserData requestUserData;
};

#endif // SIGNINMENU_H
