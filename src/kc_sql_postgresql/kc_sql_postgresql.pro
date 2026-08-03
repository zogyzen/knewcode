#-------------------------------------------------
#
# Project created by QtCreator 2020-11-26 17:17:37
# PostgreSQL数据库操作插件（也支持其他PostgreSQL衍生的数据库，如，电科金仓数据库）
# 引用：libpq驱动
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_sql_postgresql
TEMPLATE = lib
DEFINES += KC_SQL_POSTGRESQL_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_postgresql.cpp

HEADERS += \
        ctrl_postgresql.h \
        std.h

win32 {
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def

        # libpq库
        # 32位
        contains(QT_ARCH, i386) {
            LIBPQPTH = $$LIBRARYPTH3RD/windows/postgresql/pgsql1023
        }
        # 64位
        else {
            LIBPQPTH = $$LIBRARYPTH3RD/windows/postgresql/pgsql153x64
        }
        INCLUDEPATH += $$LIBPQPTH/include
        INCLUDEPATH += $$LIBPQPTH/include/server
        LIBS += -L$$LIBPQPTH/lib
        LIBS += -llibpq -lSecur32
        # 拷贝libpq库
        varCopyConfig = \"$$LIBPQPTH/dll/*.dll\" \"$$DESTDIR\"
        varCopyConfig ~= s,/,\\,g
        QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyConfig $$escape_expand(\\n\\t)
        export(QMAKE_POST_LINK)
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map

        # libpq库
        LIBPQPTH = $$LIBRARYPTH3RD/windows/postgresql1612
        INCLUDEPATH += $$LIBPQPTH/include
        INCLUDEPATH += $$LIBPQPTH/include/server
        LIBS += -L$$LIBPQPTH/lib
        LIBS += -llibpq -lSecur32
        # 拷贝libpq库
        varCopyConfig = \"$$LIBPQPTH/dll/*.dll\" \"$$DESTDIR\"
        varCopyConfig ~= s,/,\\,g   # 将linux路径斜杠变windows路径斜杠
        QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyConfig $$escape_expand(\\n\\t)
        export(QMAKE_POST_LINK)
    }
}
unix {
    # INCLUDEPATH += /usr/include/postgresql
    # LIBS += -L/usr/local/lib
    # LIBS += -L/usr/local/lib64
    # LIBS += -lpq

    # 判断x86或arm芯片架构
    contains(QT_ARCH, arm64){
        LIBPQPTH = /usr/include/postgresql
        INCLUDEPATH += $$LIBPQPTH
    }else{
        LIBPQPTH = $$LIBRARYPTH3RD/linux/postgresql/pgsql18x64
        INCLUDEPATH += $$LIBPQPTH/include
        LIBS += -L$$LIBPQPTH/lib

        # 拷贝文件
        QMAKE_POST_LINK += $$QMAKE_COPY $$LIBPQPTH/lib/libpq.so.5 $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
    }
    LIBS += -lpq

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map

    # 拷贝文件
    message($$QMAKE_POST_LINK)
    export(QMAKE_POST_LINK)
}
