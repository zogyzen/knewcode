#-------------------------------------------------
#
# Project created by QtCreator 2022-10-20 12:06:32
# 达梦数据库操作插件
# 引用：达梦数据库客户端dpi驱动（达梦的安装目录/drivers/dpi）
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_sql_dm
TEMPLATE = lib
DEFINES += KC_SQL_DM_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_dm.cpp

HEADERS += \
        ctrl_dm.h \
        std.h

win32{
    QMAKE_TARGET_DESCRIPTION = "kc_sql_dm Alpha Version v0.2.0"

    # 达梦c++库
    # 32位
    contains(QT_ARCH, i386) {
        DMLIBPTH = $$LIBRARYPTH3RD/windows/dm-x86/dpi
    }
    # 64位
    else {
        DMLIBPTH = $$LIBRARYPTH3RD/windows/dm-x64/dpi.new
    }
    INCLUDEPATH += $$DMLIBPTH/include
    LIBS += $$DMLIBPTH/dmdpi.lib
    # 拷贝dpi库
    varCopyFile = \"$$DMLIBPTH/*.dll\" \"$$DESTDIR\"
    varCopyFile ~= s,/,\\,g
    QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyFile
    varCopyDirFile = \"$$DMLIBPTH/dependencies\" \"$$DESTDIR/dependencies\"
    varCopyDirFile ~= s,/,\\,g
    QMAKE_POST_LINK += & $$QMAKE_COPY_DIR $$varCopyDirFile $$escape_expand(\\n\\t)
    export(QMAKE_POST_LINK)

    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def
    }
    mingw {
    }
}
unix {
    # x86
    contains(QT_ARCH, x86_64) {
        INCLUDEPATH += $$LIBRARYPTH3RD/linux/dm/dpi/include
        LIBS += $$LIBRARYPTH3RD/linux/dm/dpi/libdmdpi.a
    }
    # arm
    else {
        INCLUDEPATH += $$LIBRARYPTH3RD/linux/dm-arm/dpi/include
        LIBS += $$LIBRARYPTH3RD/linux/dm-arm/dpi/libdmdpi.a
        LIBS += -lrt
    }
    DEFINES += DM64

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
