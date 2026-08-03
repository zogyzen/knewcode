#-------------------------------------------------
#
# Project created by QtCreator 2025-06-08 22:06:07
# SQLite数据库操作插件
# 引用：
#    https://sqlite.org/
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_sql_sqlite
TEMPLATE = lib
DEFINES += KC_SQL_SQLITE_LIBRARY

# sqlite库
SQLITEPTH = $$LIBRARYPTH3RD/common/sqlite/sqlite-3.53.1
INCLUDEPATH += $$SQLITEPTH

SOURCES += \
        $$SQLITEPTH/sqlite3.c \
        activator.cpp \
        ctrl_sqlite.cpp

HEADERS += \
        $$SQLITEPTH/sqlite3.h \
        ctrl_sqlite.h \
        std.h

win32 {
    QMAKE_TARGET_DESCRIPTION = "kc_sql_sqlite Alpha Version v0.5"

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
