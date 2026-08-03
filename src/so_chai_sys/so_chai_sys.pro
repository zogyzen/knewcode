#-------------------------------------------------
#
# Project created by QtCreator 2026-04-17 11:15:23
# ChaiScript脚本扩展模块（系统通用功能）
# 引用：
#    https://github.com/ChaiScript/ChaiScript
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = so_chai_sys
TEMPLATE = lib
DEFINES += SO_CHAI_MATH_LIBRARY

# ChaiScript库
CHAISCRIPTPTH = $$LIBRARYPTH3RD/common/ChaiScript
INCLUDEPATH += $$CHAISCRIPTPTH/include

SOURCES += \
    chai_sys.cpp

HEADERS += \
        chai_sys.h \
        std.h

win32 {
    msvc {
        DEF_FILE = so_chai_sys.def
        DISTFILES += so_chai_sys.def
    }
    mingw {
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
