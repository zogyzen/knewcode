#-------------------------------------------------
#
# Project created by QtCreator 2025-09-17 22:57:22
# ChaiScript脚本插件。
# 引用：
#    https://github.com/ChaiScript/ChaiScript
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_chai_script
TEMPLATE = lib
DEFINES += KC_CHAI_SCRIPT_LIBRARY

# ChaiScript库
CHAISCRIPTPTH = $$LIBRARYPTH3RD/common/ChaiScript
INCLUDEPATH += $$CHAISCRIPTPTH/include

SOURCES += \
        activator.cpp \
        chai_module.cpp \
        ctrl_chai.cpp
HEADERS += \
        chai_module.h \
        ctrl_chai.h \
        std.h

win32 {
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map

        QMAKE_CXXFLAGS_DEBUG += -O1
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
