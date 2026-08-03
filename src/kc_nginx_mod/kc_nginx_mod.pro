#-------------------------------------------------
#
# Project created by QtCreator 2024-02-07 00:06:07
# Nginx模块。Nginx可以通过本模块启动整个框架。
# 引用：
#    https://nginx.org/
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_nginx_mod
TEMPLATE = lib
DEFINES += KC_NGINX_MOD_LIBRARY
#DEFINES += AP_HAVE_DESIGNATED_INITIALIZER

SOURCES += \
        kc_nginx_mod.cpp \
        nginx_work.cpp

HEADERS += \
        nginx_work.h \
        std.h

INCLUDEPATH += $$LIBRARYPTH3RD/common/nginx/nginx-with/module/ngx_knewcode_mod

win32 {
    msvc {
        DEF_FILE = kc_nginx_mod.def
        DISTFILES += kc_nginx_mod.def
        DEFINES += _OFF_T_DEFINED
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
