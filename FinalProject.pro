#-------------------------------------------------
#
# Project created by QtCreator 2019-12-02T16:54:43
#
#-------------------------------------------------

QT       += core gui sql charts concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FinalProject
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        dataimportingthread.cpp \
        main.cpp \
        mainwindow.cpp \
        pathsearchwidget.cpp \
        plotwidget.cpp \
        preferencesdialog.cpp \
        querywidget.cpp \
        utilities/BDateTime.cpp

HEADERS += \
        dataimportingthread.h \
        mainwindow.h \
        pathsearchwidget.h \
        plotwidget.h \
        preferencesdialog.h \
        querywidget.h \
        utilities/BDateTime.h \
        utilities/base.hpp \
        utilities/hint.hpp

FORMS += \
        mainwindow.ui \
        pathsearchwidget.ui \
        plotwidget.ui \
        preferencesdialog.ui \
        querywidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
