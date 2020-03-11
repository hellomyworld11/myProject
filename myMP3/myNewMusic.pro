#-------------------------------------------------
#
# Project created by QtCreator 2019-03-14T09:08:41
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myNewMusic
TEMPLATE = app


SOURCES += main.cpp\
        themainwidget.cpp \
    mythreadlyricshow.cpp

HEADERS  += themainwidget.h \
    mythreadlyricshow.h

FORMS    += themainwidget.ui

RESOURCES += \
    image.qrc
CONFIG += C++11
