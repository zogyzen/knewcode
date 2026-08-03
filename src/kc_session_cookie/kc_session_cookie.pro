#-------------------------------------------------
#
# Project created by QtCreator 2017-09-03T18:23:44
# 管理Cookie和Session的插件
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_session_cookie
TEMPLATE = lib
DEFINES += KC_SESSION_COOKIE_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_local_ctrl.cpp \
        kc_session_cookie.cpp

HEADERS += \
        ctrl_local_ctrl.h \
        ctrl_session.h \
        kc_session_cookie.h \
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
