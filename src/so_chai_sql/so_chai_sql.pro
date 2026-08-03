#-------------------------------------------------
#
# Project created by QtCreator 2026-04-17 13:55:25
# ChaiScript脚本扩展模块（数据库操作功能）
# 引用：
#    https://github.com/ChaiScript/ChaiScript
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = so_chai_sql
TEMPLATE = lib
DEFINES += SO_CHAI_SQL_LIBRARY

# ChaiScript库
CHAISCRIPTPTH = $$LIBRARYPTH3RD/common/ChaiScript
INCLUDEPATH += $$CHAISCRIPTPTH/include

SOURCES += \
    chai_sql.cpp

HEADERS += \
        chai_sql.h \
        std.h

win32 {
    msvc {
        DEF_FILE = so_chai_sql.def
        DISTFILES += so_chai_sql.def
    }
    mingw {
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
