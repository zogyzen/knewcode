#-------------------------------------------------
#
# Project created by QtCreator 2017-08-01T08:47:52
# Apache扩展模块。Apache可以通过本模块启动整个框架。
# 引用：
#    https://downloads.apache.org/httpd/
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_apache_mod
TEMPLATE = lib
DEFINES += KC_APACHE_MOD_LIBRARY
#DEFINES += AP_HAVE_DESIGNATED_INITIALIZER

SOURCES += \
        apache_work.cpp \
        kc_apache_mod.cpp

HEADERS += \
        apache_work.h \
        std.h

win32 {
    msvc {
        DEF_FILE = kc_apache_mod.def
        DISTFILES += kc_apache_mod.def

        # 32位
        contains(QT_ARCH, i386) {
            APACHEPTH = $$LIBRARYPTH3RD/windows/Apache24x86
        }
        # 64位
        else {
            APACHEPTH = $$LIBRARYPTH3RD/windows/Apache24x64
        }
        # apache库
        INCLUDEPATH += $$APACHEPTH/include
        LIBS += -l$$APACHEPTH/lib/libhttpd
        LIBS += -l$$APACHEPTH/lib/libapr-1
        LIBS += -l$$APACHEPTH/lib/libaprutil-1
        LIBS += -l$$APACHEPTH/lib/libaprutil-1
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map

        INCLUDEPATH += $$PWD/include
        APACHEPTH = $$LIBRARYPTH3RD/windows/Apache24x64
        INCLUDEPATH += $$APACHEPTH/include
        LIBS += -L$$APACHEPTH/lib
        LIBS += -llibapr-1 -llibhttpd -llibaprutil-1
        # LIBS += $$LIBRARYPTH3RD/windows/Apache24x64/lib/libhttpd.lib
        # LIBS += $$LIBRARYPTH3RD/windows/Apache24x64/lib/libapr-1.lib
    }
}
unix {
    APACHEPTH = $$LIBRARYPTH3RD/linux/Apache2_4
    # 判断是否centos7
    HOST_VER_LIST = $$split(QMAKE_HOST.version, '.')
    # 非centos7，用静态库
    !contains(HOST_VER_LIST, el7) {
        #LIBS += -static-libgcc -static-libstdc++
        INCLUDEPATH += $$APACHEPTH/include/apache2
        INCLUDEPATH += $$APACHEPTH/include/apr-1.0
    }
    # centos7，用动态库
    contains(HOST_VER_LIST, el7) {
        INCLUDEPATH += $$APACHEPTH/include2/httpd
        INCLUDEPATH += $$APACHEPTH/include2/apr-1
    }

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
