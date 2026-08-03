#-------------------------------------------------
#
# Project created by QtCreator 2020-04-02 09:41:45
# Web请求的转发插件
# 引用：
#    https://github.com/yhirose/cpp-httplib
#    https://github.com/openssl/openssl
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_request_relay
TEMPLATE = lib
DEFINES += KC_REQUEST_RELAY_LIBRARY

SOURCES += \
        activator.cpp \
        ctrl_request.cpp \
        single_load.cpp

HEADERS += \
        ctrl_request.h \
        single_load.h \
        std.h

INCLUDEPATH += $$LIBRARYPTH3RD/common/cpp-httplib
DEFINES += CPPHTTPLIB_OPENSSL_SUPPORT

win32 {
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def

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
