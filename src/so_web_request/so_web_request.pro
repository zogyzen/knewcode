#-------------------------------------------------
#
# Project created by QtCreator 2026-01-06 16:59:59
# 将Web转发的功能，从kc_request_relay插件中分离出来。
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = so_web_request
TEMPLATE = lib
DEFINES += SO_WEB_REQUEST_LIBRARY

SOURCES += \
        single_load.cpp

HEADERS += \
        single_load.h \
        std.h

INCLUDEPATH += $$LIBRARYPTH3RD/common/cpp-httplib
DEFINES += CPPHTTPLIB_OPENSSL_SUPPORT

win32 {
    msvc {
        DEF_FILE = so_web_request.def
        DISTFILES += so_web_request.def

        #OPENSSL库
        # 32位
        contains(QT_ARCH, i386) {
            OPENSLLPTH = $$LIBRARYPTH3RD/windows/OpenSSL-Win32
        }
        # 64位
        else {
            OPENSLLPTH = $$LIBRARYPTH3RD/windows/OpenSSL-Win64
        }
        INCLUDEPATH += $$OPENSLLPTH/include
        LIBS += -l$$OPENSLLPTH/lib/libssl_static
        LIBS += -l$$OPENSLLPTH/lib/libcrypto_static
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map

        LIBS += -lcrypt32

        #OPENSSL库
        OPENSLLPTH = $$LIBRARYPTH3RD/windows/OpenSSL-Win64
        INCLUDEPATH += $$OPENSLLPTH/include
        LIBS += -L$$OPENSLLPTH/lib/
        LIBS += -llibssl_static -llibcrypto_static
    }
}
unix {
    LIBS += -L/usr/local/lib
    LIBS += -L/usr/local/lib64
    LIBS += -lcrypto -lssl

    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
