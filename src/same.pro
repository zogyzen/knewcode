CONFIG += c++17
# QMAKE_CXXFLAGS += /std:c++17

message("*** Begin Build ***")
message($$PWD)

INCLUDEPATH += $$PWD/include

# 第三方库的路径
LIBRARYPTH3RD = $$PWD/../../MY2602KnewcodeLibrary
message(LIBRARYPTH3RD: $$LIBRARYPTH3RD)

#OBJECTS_DIR += $$PWD/../obj
contains(QT_ARCH, i386) {
    BinPth = $$PWD/../bin32
} else {
    BinPth = $$PWD/../bin
}
CONFIG(debug, debug | release) {
    BuildType = debug
} else {
    BuildType = release
}
DESTDIR = $$BinPth/$$BuildType

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
# DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += NOMINMAX

CONFIG += skip_target_version_ext
CONFIG += no_plugin_name_prefix

#LFLAGS += -Wl,-E

COMPILER = $$system($$QMAKE_CC --version 2>&1)
#message($$COMPILER)
#message($$CONFIG)

# Qt Pro 的变量
# TARGET	生成目标的名字
# DESTDIR	目标输出文件路径
# PWD	当前文件(.pro或.pri)所在的路径
# OUT_PWD	Makefile生成的路径
# PRO_FILE_PWD	pro项目文件所在的路径
# QMAKE_HOST.arch	计算机架构
# QMAKE_HOST.os	计算机系统
# QMAKE_HOST.cpu_count	计算机CPU核心数
# QMAKE_HOST.name	计算机名
# QMAKE_HOST.version	系统版本(数字形式)
# QMAKE_HOST.version_string	系统版本(字符串形式)
# QMAKE_POST_LINK	编译链接后自动执行命令
# DEFINES	编译器定义的预处理器宏，类似于gcc -D 选项
# INCLUDEPATH	包含头文件路径
# LIBS	指定要链接到项目中的库

# 显示 Qt Pro 变量值
#message(QMAKE_HOST.version)
#COMPILER = $$system($$QMAKE_CC --version 2>&1)
#message($$COMPILER)
#message($$QMAKE_CC)
#message($$CONFIG)
#message($$QMAKE_MSC_VER)
#message($$QMAKE_MSC_FULL_VER)
#message($$COMPAT_MKSPEC)
#message($$MSVC_VER)
#message($$(VisualStudioVersion))
#message($$QT_ARCH)
#message($$QMAKE_HOST.arch)

win32 {
    # 构建的版本信息
    VERTIMEPART = $$system(echo "%Time:~0,2%%Time:~3,2%")
    VERTIMEFULL = $$system(echo "%Date:~0,4%.%Date:~5,2%.%Date:~8,2%.")$$VERTIMEPART
    QMAKE_TARGET_DESCRIPTION = "Knewcode Mainline Version v1.2.1"    # Alpha Version / Beta Version / Release Candidate / Trial Version / Stable Release / Mainline Version
    QMAKE_TARGET_PRODUCT = Knewcode v1.2.1
    QMAKE_TARGET_COMPANY = zogy@knewcode.com
    QMAKE_TARGET_COPYRIGHT = "Copyright(C) ShangHai MingYe 2020"
    VERSION = $$VERTIMEFULL
    # VERSION_PE_HEADER = 1.1

    LIBS += -ldbghelp -lpsapi
    LIBS += -luser32 -ladvapi32
    DEFINES += WIN32 WIN32_LEAN_AND_MEAN
    CONFIG += -static-libstdc++

    # 盘符
    DISK = $$str_member($$PWD, 0, 1)

    msvc {
        QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
        QMAKE_CXXFLAGS_STL_ON = /EHa
        QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE
        QMAKE_LFLAGS_RELEASE += /MAP /DEBUG /opt:ref /INCREMENTAL:NO
        # QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRT.lib

        # 增加每个节的大小限制
        QMAKE_CXXFLAGS += /bigobj

        # 源文件编码改为utf8，日志可以输出Emoji表情
        QMAKE_CXXFLAGS += /utf-8

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

        # boost库的路径
        BOOSTPTH = $$DISK/library/boost_1_87
        # DEFINES += BOOST_THREAD_VERSION=4

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

        message($$BOOSTPTHLIBV)
    }
    mingw {
        DESTDIR = $$BinPth/mingw/$$BuildType
        # 动态库查找路径
        QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGING\'
        QMAKE_LFLAGS += -Wl,-rpath,@loader_path
        QMAKE_LFLAGS += -Wl,-rpath,./

        QMAKE_LFLAGS += -Wl,-Bsymbolic
        QMAKE_LFLAGS += -Wl,--stack,33554432

        # 增加每个节的大小限制
        QMAKE_CXXFLAGS += -Wa,-mbig-obj #-Wl,--large-address-aware
        # QMAKE_CFLAGS += -Wa,-mbig-obj

        # QMAKE_CXXFLAGS += -O2
        # QMAKE_CFLAGS += -fseh
        # QMAKE_CXXFLAGS += -fseh
        QMAKE_CXXFLAGS += -fno-keep-inline-dllexport
        QMAKE_LFLAGS  += -fno-keep-inline-dllexport

        QMAKE_CXXFLAGS += -m64
        QMAKE_LFLAGS  += -m64

        # QMAKE_CXXFLAGS += -nostdlib
        # QMAKE_LFLAGS += -nostartfiles -nostdlib -nodefaultlibs

        DEFINES += _FILE_OFFSET_BITS=64
        DEFINES += NO_ICONV

        # boost库的路径
        BOOSTPTH = $$DISK/library/boost_1_87_gcc13
        INCLUDEPATH += $$BOOSTPTH/include
        LIBS += -L$$BOOSTPTH/lib
        LIBS += -llibboost_system-mgw13-mt-s-x64-1_87 -llibboost_filesystem-mgw13-mt-s-x64-1_87 -llibboost_locale-mgw13-mt-s-x64-1_87
        LIBS += -llibboost_chrono-mgw13-mt-s-x64-1_87 -llibboost_date_time-mgw13-mt-s-x64-1_87 -llibboost_thread-mgw13-mt-s-x64-1_87
        LIBS += -llibboost_log_setup-mgw13-mt-s-x64-1_87 -llibboost_log-mgw13-mt-s-x64-1_87 -llibboost_json-mgw13-mt-s-x64-1_87

        LIBS += -lws2_32 -lmswsock -liconv
    }
}
unix {
    VERSION = 1.2.1

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
    # export LD_LIBRARY_PATH=/home/user/lib:$LD_LIBRARY_PATH                # 在命令行下设置动态库查找路径
    # patchelf --print-rpath /path/to/your/executable                       # 查看当前RPATH
    # patchelf --set-rpath /new/path:/new/path2 /path/to/your/executable    # 设置新的RPATH

    QMAKE_LFLAGS += -Wl,-Bsymbolic
    # QMAKE_LFLAGS += -Wl,-soname

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


    # 判断x86或arm芯片架构
    contains(QT_ARCH, arm64){
        BOOSTPTH = $$LIBRARYPTH3RD/linux/boost/arm_gcc9
    }else{
        BOOSTPTH = $$LIBRARYPTH3RD/linux/boost/x64_clang18
    }

    # boost库的路径
    INCLUDEPATH += $$BOOSTPTH

    LIBS += -L$$BOOSTPTH/lib
    LIBS += -lboost_system -lboost_filesystem -lboost_locale -lboost_date_time -lboost_chrono
    LIBS += -lboost_regex -lboost_thread -lboost_json -lboost_iostreams
}
