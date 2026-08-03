#-------------------------------------------------
#
# Project created by QtCreator 2017-06-11T20:41:50
# 框架主模块
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_web_main
TEMPLATE = lib
DEFINES += KC_WEB_MAIN_LIBRARY

SOURCES += \
        activator.cpp \
        kc_web_work.cpp \
        request_respond.cpp

HEADERS += \
        kc_web_work.h \
        request_respond.h \
        std.h

win32 {
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def
    }
    mingw {
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
