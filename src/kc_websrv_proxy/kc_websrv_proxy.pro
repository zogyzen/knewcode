#-------------------------------------------------
#
# Project created by QtCreator 2023-12-23 21:45:56
# 代理接口。为其他接口（如，kc_websrv_c_api）和内置web服务器，启动框架提供代理。
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_websrv_proxy
TEMPLATE = lib
DEFINES += KC_WEBSRV_PROXY_LIBRARY

SOURCES += \
        websrv_conn.cpp \
        request_respond.cpp \
        kc_websrv_proxy.cpp

HEADERS += \
        websrv_conn.h \
        request_respond.h \
        std.h \
        kc_websrv_proxy.h

win32 {
    # SOURCES += except_filter_win.cpp

    msvc {
        DEF_FILE = kc_websrv_proxy.def
        DISTFILES += kc_websrv_proxy.def
    }
    mingw {
    }
}
unix {
    # SOURCES += except_filter_linux.cpp

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
