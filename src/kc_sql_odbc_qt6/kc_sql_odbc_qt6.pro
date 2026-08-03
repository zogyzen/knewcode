#-------------------------------------------------
#
# Project created by QtCreator 2017-08-01T08:47:52
#
#-------------------------------------------------

QT       -= core gui
QT       += sql

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_sql_odbc
TEMPLATE = lib
DEFINES += KC_SQL_ODBC_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_odbc.cpp \
        kc_sql_odbc.cpp \
        odbc_mssql.cpp

HEADERS += \
        ctrl_odbc.h \
        kc_sql_odbc.h \
        odbc_mssql.h \
        std.h

win32{
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def
    }
    mingw {
    }

    # # qodbc库
    # # 32位
    # contains(QT_ARCH, i386) {
    #     QODBCPTH = $$LIBRARYPTH3RD/windows/qodbc
    # }
    # # 64位
    # else {
    #     QODBCPTH = $$LIBRARYPTH3RD/windows/qodbc-x64
    # }
    # varCopyConfig = \"$$QODBCPTH\" \"$$DESTDIR\"
    # varCopyConfig ~= s,/,\\,g
    # QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$varCopyConfig
    # export(QMAKE_POST_LINK)
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
