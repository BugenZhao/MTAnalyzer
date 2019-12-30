#-------------------------------------------------
#
# Project created by QtCreator 2019-12-02T16:54:43
#
#-------------------------------------------------

QT       += core gui sql charts concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MTAnalyzer
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

CONFIG += c++14
QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES += \
        baseplotwidget.cpp \
        flowplotwidget.cpp \
        main.cpp \
        mainwindow.cpp \
        pathsearchwidget.cpp \
        preferencesdialog.cpp \
        querywidget.cpp \
        totalflowplotwidget.cpp \
        utilities/BDateTime.cpp \
        utilities/bdatabasemanager.cpp \
        withlineflowplotwidget.cpp \
        stationflowplotwidget.cpp \
        previewwidget.cpp

HEADERS += \
        baseplotwidget.h \
        flowplotwidget.h \
        mainwindow.h \
        pathsearchwidget.h \
        preferencesdialog.h \
        querywidget.h \
        totalflowplotwidget.h \
        utilities/BDateTime.h \
        utilities/base.hpp \
        utilities/bdatabasemanager.h \
        utilities/hint.hpp \
		withlineflowplotwidget.h \
		stationflowplotwidget.h \
		previewwidget.h

FORMS += \
        flowplotwidget.ui \
        mainwindow.ui \
        pathsearchwidget.ui \
        preferencesdialog.ui \
        querywidget.ui \
        totalflowplotwidget.ui \
        withlineflowplotwidget.ui \
        stationflowplotwidget.ui \
        previewwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
