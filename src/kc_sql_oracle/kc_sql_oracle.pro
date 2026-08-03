#-------------------------------------------------
#
# Project created by QtCreator 2020-03-29 02:13:55
# Oracle数据库操作插件
# 引用：OCCI驱动（Oracle客户端连接）
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_sql_oracle
TEMPLATE = lib
DEFINES += KC_SQL_ORACLE_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_oracle.cpp

HEADERS += \
        ctrl_oracle.h \
        std.h

#DEFINES += OCCI_ORACLE12C
win32 {
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def

        # Oracle库
        # 32位
        contains(QT_ARCH, i386) {
            contains(DEFINES, OCCI_ORACLE12C) {
                OCCIPTH = $$LIBRARYPTH3RD/windows/occi-sdk-x86-12c
            } else {
                OCCIPTH = $$LIBRARYPTH3RD/windows/occi-sdk-x86
            }
        }
        # 64位
        else {
            OCCIPTH = $$LIBRARYPTH3RD/windows/occi-sdk-x64/instantclient_21_20
            OCCIPTHLIB = $$OCCIPTH/lib/msvc
            # OCCIPTH = $$LIBRARYPTH3RD/windows/occi-sdk-x64/instantclient_23_26
            # OCCIPTHLIB = $$OCCIPTH/lib/msvc
        }
        INCLUDEPATH += $$OCCIPTH/include
        CONFIG(debug, debug | release) {
            contains(DEFINES, OCCI_ORACLE12C) {
                LIBS += $$OCCIPTH/lib/MSVC/vc12/oraocci12d.lib
            } else {
                LIBS += $$OCCIPTHLIB/oraocci21d.lib
                # LIBS += $$OCCIPTHLIB/oraocci23d.lib
            }
        }
        else {
            contains(DEFINES, OCCI_ORACLE12C) {
                LIBS += $$OCCIPTH/lib/MSVC/vc12/oraocci12.lib
            } else {
                LIBS += $$OCCIPTHLIB/oraocci21.lib
                # LIBS += $$OCCIPTHLIB/oraocci23.lib
            }
        }
        # 拷贝Oracle库
        contains(DEFINES, OCCI_ORACLE12C) {
            varCopyConfig = \"$$OCCIPTH/lib/msvc/vc12/*.dll\" \"$$DESTDIR\"
        } else {
            varCopyConfig = \"$$OCCIPTHLIB/*.dll\" \"$$DESTDIR\"
        }
        varCopyConfig ~= s,/,\\,g
        QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyConfig $$escape_expand(\\n\\t)
        export(QMAKE_POST_LINK)
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map

    # Oracle库
    # 判断x86或arm芯片架构
    contains(QT_ARCH, arm64){
        OCCIPTH = $$LIBRARYPTH3RD/linux/occi-sdk-arm/instantclient_19_29/sdk
        # OCCIPTH = $$LIBRARYPTH3RD/linux/occi-sdk-arm/instantclient_23_26/sdk

        LIBS += -L$$OCCIPTH/lib
        LIBS += -locci -lclntsh -lociei
    }else{
        OCCIPTH = $$LIBRARYPTH3RD/linux/occi-sdk-x64/instantclient_21_21/sdk
        # OCCIPTH = $$LIBRARYPTH3RD/linux/occi-sdk-x64/instantclient_23_26/sdk

        LIBS += -L$$OCCIPTH/lib
        LIBS += -locci_gcc53 -lclntsh -lociei

        # 拷贝文件
        QMAKE_POST_LINK += $$QMAKE_COPY $$OCCIPTH/lib/libclntsh.so.21.1 $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$OCCIPTH/lib/libclntshcore.so.21.1 $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$OCCIPTH/lib/libnnz21.so $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$OCCIPTH/lib/libocci.so.21.1 $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$OCCIPTH/lib/libocci_gcc53.so.21.1 $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$OCCIPTH/lib/libociei.so $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
    }
    INCLUDEPATH += $$OCCIPTH/include

    # patchelf --set-rpath ./:../lib libclntsh.so.23.1    # 设置libnnz.so的搜索路径

    # 拷贝文件
    QMAKE_POST_LINK += $$QMAKE_COPY $$LIBRARYPTH3RD/linux/other/lib/* $$DESTDIR/../lib/ $$escape_expand(\\n\\t)
    message($$QMAKE_POST_LINK)
    export(QMAKE_POST_LINK)
}
