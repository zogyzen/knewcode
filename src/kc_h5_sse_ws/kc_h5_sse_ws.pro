#-------------------------------------------------
#
# Project created by QtCreator 2021-08-22 22:53:22
# Server-Sent Events和Websock插件
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

INCLUDEPATH += $$LIBRARYPTH3RD/common
INCLUDEPATH += $$LIBRARYPTH3RD/common/json

TARGET = kc_h5_sse_ws
TEMPLATE = lib
DEFINES += KC_H5_SSE_WS_LIBRARY

# cJSON库
CJSONPTH = $$LIBRARYPTH3RD/common/json/cJSON/src

SOURCES += \
        $$CJSONPTH/cJSON.c \
        activator.cpp \
        ctrl_sse.cpp \
        ctrl_ws.cpp \
        kc_h5_sse_ws.cpp

HEADERS += \
        $$CJSONPTH/cJSON.h \
        ctrl_sse.h \
        ctrl_ws.h \
        kc_h5_sse_ws.h \
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
