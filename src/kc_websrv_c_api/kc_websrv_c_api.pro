#-------------------------------------------------
#
# Project created by QtCreator 2025-07-14 15:06:21
# C语言接口api。为第三方Web平台（如，Apache、Nginx等）的扩展模块，与kc_websrv_proxy模块建立桥梁。
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_websrv_c_api
TEMPLATE = lib
DEFINES += KC_WEBSRV_API_LIBRARY
#DEFINES += AP_HAVE_DESIGNATED_INITIALIZER

SOURCES += \
        api_work.cpp \
        kc_websrv_c_api.cpp \
        work_proxy_cb.cpp

HEADERS += \
        api_work.h \
        std.h \
        work_proxy_cb.h

win32 {
    msvc {
        DEF_FILE = kc_websrv_c_api.def
        DISTFILES += kc_websrv_c_api.def
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
