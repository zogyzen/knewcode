#-------------------------------------------------
#
# Project created by QtCreator 2025-08-06 11:06:54
# 将写日志的功能从主框架中分离出来，单独做成一个模块。可以支持更多的日志库。
# 本模块使用单线程的队列，简单的写文本文件。
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = so_log_my
TEMPLATE = lib
DEFINES += SO_LOG_MY_LIBRARY

SOURCES += \
    func_log.cpp

HEADERS += \
        func_log.h \
        std.h

win32 {
    msvc {
        DEF_FILE = so_log_my.def
        DISTFILES += so_log_my.def
    }
    mingw {
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
