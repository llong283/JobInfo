#-------------------------------------------------
#
# Project created by QtCreator 2012-03-14T20:18:07
#
#-------------------------------------------------

QT       += core gui network webkit sql

TARGET = JobInfo
TEMPLATE = app
#CONFIG += qtestlib
RC_FILE = JobInfo.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    downloadmanager.cpp \
    dialogremind.cpp \
    dialogset.cpp \
    setting.cpp \
    dialogjobfairremind.cpp

HEADERS  += mainwindow.h \
    downloadmanager.h \
    dialogremind.h \
    config.h \
    dialogset.h \
    setting.h \
    dialogjobfairremind.h

FORMS    += mainwindow.ui \
    dialogremind.ui \
    dialogset.ui \
    dialogjobfairremind.ui

RESOURCES += \
    resource.qrc

OTHER_FILES +=
