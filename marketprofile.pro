#-------------------------------------------------
#
# Project created by QtCreator 2016-06-10T00:52:05
#
#-------------------------------------------------

QT       += core gui widgets printsupport network sql quick

TARGET = marketprofile
TEMPLATE = app

#CONFIG += debug

SOURCES += src/main.cpp\
        src/mainwindow.cpp\
        src/marketprofile.cpp\
        src/qcustomplot.cpp\
        src/resthandler.cpp\
        src/datamanager.cpp\
        src/settingsmanager.cpp \
        src/candlestickchart.cpp

HEADERS  += src/mainwindow.h\
        src/marketprofile.h\
        src/qcustomplot.h\
        src/resthandler.h\
        src/datamanager.h\
        src/settingsmanager.h\
        src/config.h \
        src/candlestickchart.h

//DISTFILES += \
//    Menu.qml

RESOURCES += \
    resourcemenu.qrc

DISTFILES += \
    Menu.qml
