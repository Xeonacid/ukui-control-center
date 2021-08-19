#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:08:41
#
#-------------------------------------------------

QT       += widgets dbus x11extras

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig \
          C++11

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)
include($$PROJECT_COMPONENTSOURCE/closebutton.pri)
include($$PROJECT_COMPONENTSOURCE/imageutil.pri)
include($$PROJECT_COMPONENTSOURCE/hoverwidget.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)
include($$PROJECT_COMPONENTSOURCE/frame.pri)
include($$PROJECT_COMPONENTSOURCE/addbtn.pri)
include($$PROJECT_COMPONENTSOURCE/button.pri)

TARGET = $$qtLibraryTarget(datetime)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lpolkit-qt5-core-1

##加载gio库和gio-unix库，用于处理时间
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    clock.cpp \
    datetime.cpp \
    changtime.cpp \
    timeBtn.cpp \
    worldMap/zoneinfo.cpp \
    worldMap/toolpop.cpp \
    worldMap/timezonemap.cpp \
    worldMap/timezonechooser.cpp \
    worldMap/poplistdelegate.cpp \
    worldMap/poplist.cpp

HEADERS += \
    clock.h \
    datetime.h \
    changtime.h \
    timeBtn.h \
    worldMap/zoneinfo.h \
    worldMap/toolpop.h \
    worldMap/timezonemap.h \
    worldMap/timezonechooser.h \
    worldMap/poplistdelegate.h \
    worldMap/poplist.h

FORMS += \
    datetime.ui \
    changtime.ui

RESOURCES += \
    tz.qrc

DISTFILES +=
