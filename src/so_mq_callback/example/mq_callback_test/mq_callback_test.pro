QT = core

CONFIG += c++17 cmdline

contains(QT_ARCH, i386) {
    BinPth = $$PWD/../bin32
} else {
    BinPth = $$PWD/../bin64
}
CONFIG(debug, debug | release) {
    BuildType = debug
} else {
    BuildType = release
}
DESTDIR = $$BinPth/$$BuildType

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../../../include
INCLUDEPATH += $$PWD/../include

SOURCES += \
        main.cpp

win32 {
    # 构建的版本信息
    VERTIMEPART = $$system(echo "%Time:~0,2%%Time:~3,2%")
    VERSION = $$system(echo "%Date:~0,4%.%Date:~5,2%.%Date:~8,2%.")$$VERTIMEPART
    QMAKE_TARGET_DESCRIPTION = zogy@knewcode.com
    QMAKE_TARGET_PRODUCT = Knewcode v1.2
    QMAKE_TARGET_COPYRIGHT = "Copyright(C) ShangHai MingYe 2020"

    # 盘符
    DISK = $$str_member($$PWD, 0, 1)

    msvc {
        QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
        QMAKE_CXXFLAGS_STL_ON = /EHa
        QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE
        QMAKE_LFLAGS_RELEASE += /MAP /DEBUG /opt:ref /INCREMENTAL:NO

        # 源文件编码改为utf8，日志可以输出Emoji表情
        QMAKE_CXXFLAGS += /utf-8

        # boost库的路径
        BOOSTPTH = $$DISK/library/boost_1_87
        INCLUDEPATH += $$BOOSTPTH
        # 32位
        contains(QT_ARCH, i386) {
            BOOSTPTHLIB = $$BOOSTPTH/lib32
        }
        # 64位
        else {
            BOOSTPTHLIB = $$BOOSTPTH/lib64
        }
        # vc的版本号，2015~2022
        MSVCVERH = $$(VisualStudioVersion)
        equals(MSVCVERH, 14.0) BOOSTPTHLIBV = $$BOOSTPTHLIB-msvc-14.0
        equals(MSVCVERH, 15.0) BOOSTPTHLIBV = $$BOOSTPTHLIB-msvc-14.1
        equals(MSVCVERH, 16.0) BOOSTPTHLIBV = $$BOOSTPTHLIB-msvc-14.2
        equals(MSVCVERH, 17.0) BOOSTPTHLIBV = $$BOOSTPTHLIB-msvc-14.3
        LIBS += -L$$BOOSTPTHLIBV
    }
    mingw {
        # boost库的路径
        BOOSTPTH = $$DISK/library/boost_1_87_gcc13
        INCLUDEPATH += $$BOOSTPTH/include
        LIBS += -L$$BOOSTPTH/lib
        LIBS += -llibboost_system-mgw13-mt-s-x64-1_87 -llibboost_filesystem-mgw13-mt-s-x64-1_87 -llibboost_locale-mgw13-mt-s-x64-1_87
        LIBS += -llibboost_chrono-mgw13-mt-s-x64-1_87 -llibboost_date_time-mgw13-mt-s-x64-1_87 -llibboost_thread-mgw13-mt-s-x64-1_87
        LIBS += -llibboost_log_setup-mgw13-mt-s-x64-1_87 -llibboost_log-mgw13-mt-s-x64-1_87 -llibboost_json-mgw13-mt-s-x64-1_87
    }
}
unix {
    target.path = ~/website/knewcode_v12
    INSTALLS += target
    # 动态库查找路径
    QMAKE_LFLAGS += -Wl,-rpath,@loader_path
    QMAKE_LFLAGS += -Wl,-rpath,@loader_path/lib
    QMAKE_LFLAGS += -Wl,-rpath,@executable_path
    QMAKE_LFLAGS += -Wl,-rpath,.
    QMAKE_LFLAGS += -Wl,-rpath,./
    QMAKE_LFLAGS += -Wl,-rpath,./lib
    QMAKE_LFLAGS += -Wl,-rpath,../lib
    QMAKE_LFLAGS += -Wl,-rpath,~/lib
    QMAKE_LFLAGS += -Wl,-rpath,~/website/knewcode_v12
    QMAKE_RPATHDIR += :\'\$\$ORIGING\'   # 启动程序目录
    QMAKE_RPATHDIR += \'\$$ORIGIN\'
    QMAKE_RPATHDIR += \'\$$ORIGIN/lib\'
    QMAKE_RPATHDIR += \'\$$ORIGIN/../lib\'
    QMAKE_LFLAGS_RPATH += #. .. ./libs
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING\'
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING/lib\'
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING/../lib\'
    QMAKE_LFLAGS += -Wl,-rpath,$ORIGING/../lib
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/lib
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/knewcode
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/knewcode/lib
    QMAKE_LFLAGS += -Wl,-rpath,target

    CONFIG += -static -static-glibc -static-libgcc

    # 判断x86或arm芯片架构
    contains(QT_ARCH, arm64){
        BOOSTPTH = /data/usershare/library/boost_current
    }else{
        BOOSTPTH = /home/zogy/library/boost_current
    }

    # boost库的路径
    INCLUDEPATH += $$BOOSTPTH

    LIBS += -L$$BOOSTPTH/lib
    LIBS += -lboost_system -lboost_filesystem -lboost_locale -lboost_date_time -lboost_chrono -lboost_regex -lboost_thread -lboost_json
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
