include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)

QT       += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(operatingmode)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt

CONFIG +=  \ 
          link_pkgconfig \
          c++11 \

PKGCONFIG += gsettings-qt \

DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    operatingmode.cpp

HEADERS += \
    operatingmode.h

FORMS += \
    operatingmode.ui

INSTALLS += target