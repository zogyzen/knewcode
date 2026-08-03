#-------------------------------------------------
#
# Project created by QtCreator 2023-10-13 16:29:48
# 文件目录管理（上传下载文件、文件目录管理、读写xml文件等）插件
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_os_file
TEMPLATE = lib
DEFINES += KC_OS_FILE_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_disk.cpp \
        ctrl_updown.cpp \
        ctrl_xml.cpp \
        kc_os_file.cpp

HEADERS += \
        ctrl_disk.h \
        ctrl_updown.h \
        ctrl_xml.h \
        kc_os_file.h \
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
