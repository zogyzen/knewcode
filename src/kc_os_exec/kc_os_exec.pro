#-------------------------------------------------
#
# Project created by QtCreator 2021-11-05 16:16:21
# 执行操作系统命令的插件
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_os_exec
TEMPLATE = lib
DEFINES += KC_OS_EXEC_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_os_exec.cpp

HEADERS += \
        ctrl_os_exec.h \
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
