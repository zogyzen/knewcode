#-------------------------------------------------
#
# Project created by QtCreator 2025-10-02 20:53:51
# Java模块。适配Java的Web平台（如，Tomcat、东方通等），可以通过本模块启动整个框架。
#
#-------------------------------------------------

QT       -= core gui

message("*** Begin Build ***")
message($$PWD)

contains(QT_ARCH, i386) {
    BinPth = $$PWD/../../bin32
} else {
    BinPth = $$PWD/../../bin
}
CONFIG(debug, debug | release) {
    BuildType += debug
} else {
    BuildType += release
}
DESTDIR = $$BinPth/$$BuildType

KcApiVersion = 1
DEFINES += KCAPIVERSION=$$KcApiVersion

TARGET = kc_java_mod.$$KcApiVersion
TEMPLATE = lib
DEFINES += KC_JAVA_MOD_LIBRARY
#DEFINES += AP_HAVE_DESIGNATED_INITIALIZER

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += skip_target_version_ext
CONFIG += no_plugin_name_prefix

COMPILER = $$system($$QMAKE_CC --version 2>&1)
#message($$COMPILER)
#message($$CONFIG)

INCLUDEPATH += $$PWD/../include

SOURCES += \
    load_kc.c \
    func_cb.c \
    kc_java_mod.c

HEADERS += \
    load_kc.h \
    func_cb.h \
        std.h

win32 {
    #中文（简体）
    RC_LANG = 0x0004

    # 构建的版本信息
    VERTIMEPART = $$system(echo "%Time:~0,2%%Time:~3,2%")
    VERSION = $$system(echo "%Date:~0,4%.%Date:~5,2%.%Date:~8,2%.")$$VERTIMEPART
    QMAKE_TARGET_DESCRIPTION = "kc_java_mod Alpha Version v0.0.1"
    QMAKE_TARGET_PRODUCT = Knewcode v1.2.1
    QMAKE_TARGET_COMPANY = zogy@knewcode.com
    QMAKE_TARGET_COPYRIGHT = "Copyright(C) ShangHai MingYe 2020"

    LIBS += -ldbghelp -lpsapi -luser32
    DEFINES += WIN32 WIN32_LEAN_AND_MEAN

    CONFIG += -static -static-glibc -static-libgcc -static-libstdc++
    LIBS += -static -static-glibc -static-libgcc -static-libstdc++

    msvc {
        QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
        QMAKE_CXXFLAGS_STL_ON = /EHa
        QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE
        # QMAKE_LFLAGS_RELEASE += /MAP /DEBUG /opt:ref /INCREMENTAL:NO
        QMAKE_LFLAGS_RELEASE += /MAP /opt:ref /INCREMENTAL:NO

        # QMAKE_CXXFLAGS_RELEASE -= -O2
        # QMAKE_CXXFLAGS_RELEASE += -O0
        # QMAKE_CFLAGS_RELEASE -= -O2
        # QMAKE_CFLAGS_RELEASE += -O0
        # QMAKE_CFLAGS_RELEASE += -O0
        # QMAKE_CXXFLAGS_DEBUG -= -MDd
        # QMAKE_CXXFLAGS_DEBUG += -MTd
        # QMAKE_CFLAGS_DEBUG -= -MDd
        # QMAKE_CFLAGS_DEBUG += -MTd
        # QMAKE_CXXFLAGS_RELEASE -= -MD
        # QMAKE_CXXFLAGS_RELEASE += -MT
        # QMAKE_CFLAGS_RELEASE -= -MD
        # QMAKE_CFLAGS_RELEASE += -MT
        # QMAKE_CXXFLAGS_RELEASE -= -MD
        # QMAKE_CXXFLAGS_RELEASE += -MDd
        # QMAKE_CFLAGS_RELEASE -= -MD
        # QMAKE_CFLAGS_RELEASE += -MDd

        DEF_FILE = kc_java_mod.def
        DISTFILES += kc_java_mod.def
    }
    mingw {
        DESTDIR = $$BinPth/mingw/$$BuildType
        # 动态库查找路径
        QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING\'
        QMAKE_LFLAGS += -Wl,-rpath,@loader_path
        QMAKE_LFLAGS += -Wl,-rpath,./

        # QMAKE_CXXFLAGS += -O2

        LIBS += -lws2_32 -liconv
    }
}
unix {
    VERSION = 0.0.1.0
    # desktopfile.path = /usr/share/applications/ # 根据需要调整路径。确保有这个目录的权限。 例如：$$[QT_INSTALL_EXAMPLES]/path/to/example/myapp.desktop 替换为实际的路径。

    target.path = /usr/local/knewcode
    INSTALLS += target
    # 动态库查找路径
    QMAKE_LFLAGS += -Wl,-rpath,@loader_path
    QMAKE_LFLAGS += -Wl,-rpath,@loader_path/lib
    QMAKE_LFLAGS += -Wl,-rpath,@executable_path
    QMAKE_LFLAGS += -Wl,-rpath,./
    QMAKE_LFLAGS += -Wl,-rpath,./lib
    QMAKE_RPATHDIR += :\'\$\$ORIGING\'   # 启动程序目录
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING\'
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING/lib\'
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/lib
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/knewcode
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/knewcode/lib
    QMAKE_LFLAGS += -Wl,-rpath,target

    # QMAKE_CFLAGS += -fPIC
    QMAKE_CXXFLAGS += -fPIC -fexceptions
    QMAKE_LFLAGS  += -fPIC -fexceptions
    # set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # cmake
    # LFLAGS += -fPIC
    # QMAKE_CXXFLAGS += -m64
    # QMAKE_LFLAGS  += -m64

    CONFIG += plugin
    # CONFIG += -static -static-glibc -static-libgcc -static-libstdc++
    CONFIG += -static -static-glibc -static-libgcc

    # 判断是否centos7
    HOST_VER_LIST = $$split(QMAKE_HOST.version, '.')
    # 非centos7，用静态库
    !contains(HOST_VER_LIST, el7) {
        #LIBS += -static-libgcc -static-libstdc++
    }
    # centos7，用动态库
    contains(HOST_VER_LIST, el7) {
    }
    # 使用静态库
    # LIBS += -static-libgcc -static-libstdc++
    LIBS += -static-libgcc #-static-libstdc++ -ldl -lstdc++

    # LIBS += -ldl -lrt -fPIC -fexceptions
    LIBS += -lrt -fPIC -fexceptions

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
