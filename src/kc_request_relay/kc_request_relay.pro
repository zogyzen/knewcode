#-------------------------------------------------
#
# Project created by QtCreator 2020-04-02 09:41:45
# Web请求的转发插件
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_request_relay
TEMPLATE = lib
DEFINES += KC_REQUEST_RELAY_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_request.cpp

HEADERS += \
        ctrl_request.h \
        std.h

win32 {
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map
    }
}
unix {
    LIBS += -L/usr/local/lib
    LIBS += -L/usr/local/lib64
    # LIBS += -lcrypto -lssl

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
