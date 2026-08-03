#-------------------------------------------------
#
# Project created by QtCreator 2020-06-10 17:17:30
# Excel读写功能模块
# 引用：
#    https://github.com/dbzhang800/QtXlsxWriter
#
#-------------------------------------------------

# QT += core

# 相同的配置
include($$PWD/../same.pro)

TARGET = so_excel
TEMPLATE = lib
DEFINES += SO_EXCEL_LIBRARY

SOURCES += \
    ctrl_excel.cpp

HEADERS += \
        ctrl_excel.h \
        std.h

# QtXlsxWriter库
QtXlsxWriterPth = $$PWD/../library/common/QtXlsxWriter
INCLUDEPATH += $$QtXlsxWriterPth/include
include($$QtXlsxWriterPth/src/xlsx/qtxlsx.pri)

win32 {
    msvc {
        DEF_FILE = so_excel.def
        DISTFILES += so_excel.def
    }
    mingw {
    }

    # QtXlsxWriter库
    # # 32位
    # contains(QT_ARCH, i386) {
    #     QtXlsxWriterPthLIB = $$QtXlsxWriterPth/lib32-msvc-14.1
    # }
    # # 64位
    # else {
    #     QtXlsxWriterPthLIB = $$QtXlsxWriterPth/lib64-msvc-14.1
    # }
    # CONFIG(debug, debug | release) {
    #     LIBS += $$QtXlsxWriterPthLIB/Qt5Xlsxd.lib
    #     # 拷贝库
    #     varCopyDLL = \"$$QtXlsxWriterPthLIB\Qt5Xlsxd.dll\" \"$$DESTDIR\"
    #     QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyDLL $$escape_expand(\\n\\t)
    # } else {
    #     LIBS += $$QtXlsxWriterPthLIB/Qt5Xlsx.lib
    #     # 拷贝库
    #     varCopyDLL = \"$$QtXlsxWriterPthLIB\Qt5Xlsx.dll\" \"$$DESTDIR\"
    #     QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyDLL $$escape_expand(\\n\\t)
    # }
}
unix {
    INCLUDEPATH += $$PWD/../library/common/xlnt/include
    INCLUDEPATH += $$PWD/../library/linux/libxl372/include_cpp

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
