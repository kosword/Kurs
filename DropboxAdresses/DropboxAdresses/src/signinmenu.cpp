#include "signinmenu.h"
#include "ui_signinmenu.h"
#include <QDesktopServices>
#include <QUrlQuery>

SignInMenu::SignInMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SignInMenu)
{
    ui->setupUi(this);
    connect(ui->signInButton, SIGNAL(clicked()), this, SLOT(signInButtonClicked()));
    connect(ui->doneButton, SIGNAL(clicked()), this, SLOT(doneButtonClicked()));
}

SignInMenu::~SignInMenu()
{
    delete ui;
}

void SignInMenu::openDropboxInABrowser()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_AUTHORIZE).toString();
    QUrlQuery query(url);
    query.addQueryItem("oauth_token", requestUserData.token);
    url.setQuery(query);

    QDesktopServices::openUrl(url);
}

void SignInMenu::requestOauthRequesttoken()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_REQUESTTOKEN);
    QNetworkRequest networkRequest(url);
    UserData userData;
    Common::oAuth->signRequestHeader("GET", &networkRequest, &userData);

    emit networkRequestGetNeeded(&networkRequest);
}

void SignInMenu::requestOauthAccesstoken()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_ACCESSTOKEN);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, &requestUserData);

    emit networkRequestGetNeeded(&networkRequest);
}

void SignInMenu::handleNetworkReply(QNetworkReply *networkReply)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkReply->url());
    switch(api)
    {
    case Dropbox::OAUTH_REQUESTTOKEN:
        handleOauthRequesttoken(networkReply);
        break;

    case Dropbox::OAUTH_ACCESSTOKEN:
        handleOauthAccesstoken(networkReply);
        break;

    default:
        break;
    }
}

void SignInMenu::handleOauthRequesttoken(QNetworkReply *networkReply)
{
    QString reply = networkReply->readAll();

    requestUserData.token = reply.split("&").at(1).split("=").at(1);
    requestUserData.secret = reply.split("&").at(0).split("=").at(1);

    openDropboxInABrowser();
}

void SignInMenu::handleOauthAccesstoken(QNetworkReply *networkReply)
{
    QString reply = networkReply->readAll();

    // update user data
    Common::userData->token = reply.split("&").at(1).split("=").at(1);
    Common::userData->secret = reply.split("&").at(0).split("=").at(1);
    Common::userData->uid = reply.split("&").at(2).split("=").at(1);

    emit oauthAccesstokenHandled();
}

void SignInMenu::signInButtonClicked()
{
    requestOauthRequesttoken();
}

void SignInMenu::doneButtonClicked()
{
    if(requestUserData.token.isEmpty() || requestUserData.secret.isEmpty())
    {
        QMessageBox::information(
                    this,
                    "DropboxAdressBook",
                    "<p align=\"center\">Похоже вы еще не разрешили приложению использовать ваш аккаунт dropbox. "
                    "Нажмите \"Войти\" и попробуйте еще раз.</p>"
                    );
    }
    else
    {
        requestOauthAccesstoken();
    }
}

