#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

#include "common.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("org.suai.g1142");
    a.setApplicationName("dropbox-adress-book");

    // load user data
    QSettings settings;
    settings.beginGroup("user_data");
    if( (settings.childKeys().indexOf("access_token") == -1) ||
        (settings.childKeys().indexOf("access_token_secret") == -1) ||
        (settings.childKeys().indexOf("uid") == -1) )
    {
        settings.remove("user_data");
    }
    else
    {
        Common::userData->token = settings.value("access_token").toString();
        Common::userData->secret = settings.value("access_token_secret").toString();
        Common::userData->uid = settings.value("uid").toString();
    }

    MainWindow w;
    w.show();

    int exitCode = a.exec();

    // save user data
    settings.setValue("access_token", Common::userData->token);
    settings.setValue("access_token_secret", Common::userData->secret);
    settings.setValue("uid", Common::userData->uid);

    return exitCode;
}
