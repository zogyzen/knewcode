#-------------------------------------------------
#
# Project created by QtCreator 2026-03-09 16:16:45
# 通过消息队列的Webapi回调模块。用于实现客户端调用Webapi接口，触发服务器端其他进程的功能。
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = so_mq_callback
TEMPLATE = lib
DEFINES += SO_MQ_CALLBACK_LIBRARY

SOURCES += \
    ctrl_mq_callback.cpp

HEADERS += \
    ctrl_mq_callback.h \
    std.h

win32 {
    msvc {
        DISTFILES += library_export.def
        DEF_FILE = library_export.def
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
