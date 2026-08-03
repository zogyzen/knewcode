#-------------------------------------------------
#
# Project created by QtCreator 2023-10-19 13:50:17
# 动态库插件的例子
#
#-------------------------------------------------

QT       -= core gui

INCLUDEPATH += $$PWD/../../include

contains(QT_ARCH, i386) {
    BinPth = $$PWD/../../../bin32
} else {
    BinPth = $$PWD/../../../bin
}
CONFIG(debug, debug | release) {
    DESTDIR += $$BinPth/debug
} else {
    DESTDIR += $$BinPth/release
}

TARGET = so_helloworld
TEMPLATE = lib

DEFINES += KC_APACHE_MOD_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ctrl_helloworld.cpp

HEADERS += \
        ctrl_helloworld.h \
        std.h

win32 {
    LIBS += -ldbghelp -lpsapi
    DEFINES += WIN32 WIN32_LEAN_AND_MEAN

    msvc {
        QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
        QMAKE_CXXFLAGS_STL_ON = /EHa
        QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE
        DEF_FILE = so_helloworld.def
        DISTFILES += so_helloworld.def
        QMAKE_LFLAGS_RELEASE += /MAP /DEBUG /opt:ref /INCREMENTAL:NO
    }
    mingw {
        CONFIG += -static-libstdc++
        QMAKE_CXXFLAGS += -O2

        LIBS += -lpsapi -lws2_32 -liconv
    }
}
unix {
    target.path = /usr/local/knewcode
    INSTALLS += target
    # 动态库查找路径
    QMAKE_LFLAGS += -Wl,-rpath=./lib
    QMAKE_RPATHDIR += :\'\$\$ORIGING\'   # 启动程序目录
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING/lib\'
    #QMAKE_LFLAGS += -Wl,-rpath=/usr/local/lib
    QMAKE_LFLAGS += -Wl,-rpath=/usr/local/knewcode
    QMAKE_LFLAGS += -Wl,-rpath=/usr/local/knewcode/lib
    #QMAKE_LFLAGS += -Wl,-rpath=target

    CONFIG += plugin
    CONFIG += no_plugin_name_prefix
    CONFIG += c++11 -static -static-glibc -static-libgcc -static-libstdc++

    # 判断是否centos7
    HOST_VER_LIST = $$split(QMAKE_HOST.version, '.')
    !contains(HOST_VER_LIST, el7) {
        LIBS += -static-libgcc -static-libstdc++
    }

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
