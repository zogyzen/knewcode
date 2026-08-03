#-------------------------------------------------
#
# Project created by QtCreator 2026-04-26 12:48:51
# 内置的Web服务器
# 引用：
#    https://github.com/openssl/openssl
#
#-------------------------------------------------

QT     -= core gui

# 相同的配置
include($$PWD/../same.pro)

CONFIG += cmdline
CONFIG -= app_bundle

SOURCES += \
        websrv_work.cpp \
        work_proxy_cb.cpp \
        main.cpp

HEADERS += \
        websrv_work.h \
        work_proxy_cb.h \
        std.h

INCLUDEPATH += $$PWD/../include
INCLUDEPATH += $$LIBRARYPTH3RD/common

win32 {
    QMAKE_TARGET_DESCRIPTION = "kc_websrv Alpha Version v0.0.1"

    RC_ICONS = $$PWD/../res/icon/knewcode.ico

    msvc {
        # CONFIG(debug, debug|release): {
        #     QMAKE_CFLAGS_DEBUG += -MTd
        #     QMAKE_CXXFLAGS_DEBUG += -MTd
        # }
        # else:CONFIG(release, debug|release): {
        #     #win32:QMAKE_CXXFLAGS += /MD
        #     QMAKE_CFLAGS_RELEASE += -MT
        #     QMAKE_CXXFLAGS_RELEASE += -MT
        # }

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
        LIBS += -luser32

    }
    mingw {
        # 无窗口
        #QMAKE_CXXFLAGS += -mwindows

        #OPENSSL库
        OPENSLLPTH = $$LIBRARYPTH3RD/windows/OpenSSL-Win64
        INCLUDEPATH += $$OPENSLLPTH/include
        LIBS += -L$$OPENSLLPTH/lib/
        LIBS += -llibssl_static -llibcrypto_static
    }
}
unix {
    VERSION = 0.0.1

    # 判断x86或arm芯片架构
    contains(QT_ARCH, arm64){
    }else{
    }

    # 无窗口
    #QMAKE_CXXFLAGS += -mwindows

    LIBS += -L/usr/local/lib
    LIBS += -L/usr/local/lib64
    LIBS += -lcrypto -lssl
    LIBS += -ldl

    # 拷贝文件
    QMAKE_POST_LINK += $$QMAKE_COPY $$PWD/../include/bin/*.sh $$DESTDIR/ $$escape_expand(\\n\\t)
    message($$QMAKE_POST_LINK)
    export(QMAKE_POST_LINK)
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
