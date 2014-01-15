#-------------------------------------------------
#
# Project created by QtCreator 2013-12-17T16:15:37
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DropboxAdressBook
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    util.cpp \
    userdata.cpp \
    oauth.cpp \
    dropbox.cpp \
    consumerdata.cpp \
    common.cpp \
    json.cpp \
    signinmenu.cpp \
    adressmenu.cpp \
    editrecordform.cpp

HEADERS  += mainwindow.h \
    util.h \
    userdata.h \
    oauth.h \
    dropbox.h \
    consumerdata.h \
    common.h \
    json.h \
    signinmenu.h \
    adressmenu.h \
    editrecordform.h

FORMS    += \
    signinmenu.ui \
    adressmenu.ui \
    editrecordform.ui

RESOURCES += \
    resources.qrc
