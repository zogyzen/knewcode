#-------------------------------------------------
#
# Project created by QtCreator 2023-10-19 13:50:17
# 管理KC工程（待定）
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = so_prjs
TEMPLATE = lib
DEFINES += SO_PRJS_LIBRARY

SOURCES += \
    ctrl_prjs.cpp \
    ctrl_prjs_common.cpp \
    ctrl_prjs_prj.cpp

HEADERS += \
        ctrl_prjs.h \
        std.h

win32 {
    msvc {
        DEF_FILE = so_prjs.def
        DISTFILES += so_prjs.def
    }
    mingw {
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
