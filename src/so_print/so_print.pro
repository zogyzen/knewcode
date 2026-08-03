#-------------------------------------------------
#
# Project created by QtCreator 2017-08-01T08:47:52
#
#-------------------------------------------------

QT += core printsupport

CONFIG += c++17

INCLUDEPATH += $$PWD/../include

contains(QT_ARCH, i386) {
    BinPth = $$PWD/../../bin32
} else {
    BinPth = $$PWD/../../bin64
}
CONFIG(debug, debug | release) {
    DESTDIR += $$BinPth/debug
} else {
    DESTDIR += $$BinPth/release
}

TARGET = so_print
TEMPLATE = lib

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
    ctrl_printl.cpp

HEADERS += \
        ctrl_printl.h \
        std.h


# QtXlsxWriter库
QtXlsxWriterPth = $$PWD/../library/common/QtXlsxWriter
INCLUDEPATH += $$QtXlsxWriterPth/include

CONFIG += skip_target_version_ext
CONFIG += no_plugin_name_prefix

win32 {
    # 构建的版本信息
    VERTIMEPART = $$system(echo "%Time:~0,2%%Time:~3,2%")
    VERSION = $$system(echo "%Date:~0,4%.%Date:~5,2%.%Date:~8,2%.")$$VERTIMEPART

    LIBS += -ldbghelp -lpsapi
    DEFINES += WIN32 WIN32_LEAN_AND_MEAN

    msvc {
        QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
        QMAKE_CXXFLAGS_STL_ON = /EHa
        QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE
        DEF_FILE = so_print.def
        QMAKE_LFLAGS_RELEASE += /MAP /DEBUG /opt:ref /INCREMENTAL:NO

        # boost库的路径
        BOOSTPTH = D:/library/boost_1_84
        INCLUDEPATH += $$BOOSTPTH
        # 32位
        contains(QT_ARCH, i386) {
            BOOSTPTHLIB = $$BOOSTPTH/lib32
        }
        # 64位
        else {
            BOOSTPTHLIB = $$BOOSTPTH/lib64
        }
        # vc的版本号，2015~2019
        MSVCVERH = $$(VisualStudioVersion)
        message($$MSVCVERH)
        equals(MSVCVERH, 14.0) LIBS += -L$$BOOSTPTHLIB-msvc-14.0
        equals(MSVCVERH, 15.0) LIBS += -L$$BOOSTPTHLIB-msvc-14.1
        equals(MSVCVERH, 16.0) LIBS += -L$$BOOSTPTHLIB-msvc-14.2
    }
    mingw {
        CONFIG += -static-libstdc++
        QMAKE_CXXFLAGS += -O2

        INCLUDEPATH += D:/library/boost_1_76_qt/include
        LIBS += -LD:/library/boost_1_76_qt/lib
        LIBS += -llibboost_system-mgw8-mt-s-x64-1_76 -llibboost_filesystem-mgw8-mt-s-x64-1_76 -llibboost_locale-mgw8-mt-s-x64-1_76
        LIBS += -llibboost_thread-mgw8-mt-s-x64-1_76

        LIBS += -lpsapi -lws2_32 -liconv
    }

    # QtXlsxWriter库
    # 32位
    contains(QT_ARCH, i386) {
        QtXlsxWriterPthLIB = $$QtXlsxWriterPth/lib32-msvc-14.1
    }
    # 64位
    else {
        QtXlsxWriterPthLIB = $$QtXlsxWriterPth/lib64-msvc-14.1
    }
    CONFIG(debug, debug | release) {
        LIBS += $$QtXlsxWriterPthLIB/Qt5Xlsxd.lib
        # 拷贝库
        varCopyDLL = \"$$QtXlsxWriterPthLIB\Qt5Xlsxd.dll\" \"$$DESTDIR\"
        QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyDLL $$escape_expand(\\n\\t)
    } else {
        LIBS += $$QtXlsxWriterPthLIB/Qt5Xlsx.lib
        # 拷贝库
        varCopyDLL = \"$$QtXlsxWriterPthLIB\Qt5Xlsx.dll\" \"$$DESTDIR\"
        QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyDLL $$escape_expand(\\n\\t)
    }
}
unix {
    INCLUDEPATH += $$PWD/../library/common/xlnt/include
    INCLUDEPATH += $$PWD/../library/linux/libxl372/include_cpp

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
    CONFIG += -static -static-glibc -static-libgcc -static-libstdc++

    # # 判断是否centos7
    # HOST_VER_LIST = $$split(QMAKE_HOST.version, '.')
    # !contains(HOST_VER_LIST, el7) {
    #     LIBS += -static-libgcc -static-libstdc++
    # }
    # 使用静态库
    LIBS += -static-libgcc -static-libstdc++

}
