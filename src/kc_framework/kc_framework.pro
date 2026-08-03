#-------------------------------------------------
#
# Project created by QtCreator 2017-07-30T17:48:50
# 主框架
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

TARGET = kc_framework
TEMPLATE = lib
DEFINES += KC_FRAMEWORK_LIBRARY

# plusaes加解密库
PLUSAESPTH = $$LIBRARYPTH3RD/common/plusaes-1.0.0
INCLUDEPATH += $$PLUSAESPTH/include

SOURCES += \
    bundle.cpp \
    bundle_context.cpp \
    framework.cpp \
    kc_framework.cpp \
    kc_lock_work.cpp \
    kc_log_thread.cpp \
    service_reference.cpp \
    service_registration.cpp

HEADERS += \
    bundle.h \
    bundle_context.h \
    framework.h \
    kc_lock_work.h \
    kc_log_thread.h \
    service_reference.h \
    service_registration.h \
    std.h

message($$_DATE_)
DEFINES += APP_COMPILE_DATETIME=\\\"$$replace(_DATE_, ' ', '_')\\\"

win32 {
    msvc {
        DEF_FILE = kc_framework.def
        DISTFILES += kc_framework.def
    }
    mingw {
        QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
        DISTFILES += version_script.map
    }

    # 拷贝文件
    #varCopyConfig = \"$$PWD/../config.xml\" \"$$OUT_PWD/$$DESTDIR\"
    # varCopyConfig = \"$$PWD/../config.xml\" \"$$DESTDIR\"
    # varCopyConfig ~= s,/,\\,g
    # QMAKE_POST_LINK += $$QMAKE_COPY $$varCopyConfig $$escape_expand(\\n\\t)

    #varCopyHintFile = \"$$PWD/../hint_file\" \"$$OUT_PWD/$$DESTDIR/hint_file\"
    varCopyHintFile = \"$$PWD/../include/bin\" \"$$DESTDIR/\"
    varCopyHintFile ~= s,/,\\,g
    #copyCmd.depends = first
    #copyCmd.commands = $$QMAKE_COPY $$varCopyCmdPath
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$varCopyHintFile $$escape_expand(\\n\\t)

    varCopyInclude = \"$$PWD\..\include\for_user\" \"$$DESTDIR/include/for_user\"
    varCopyInclude ~= s,/,\\,g
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$varCopyInclude $$escape_expand(\\n\\t)
    #message($$QMAKE_POST_LINK)
    export(QMAKE_POST_LINK)
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map

    # 拷贝文件
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$PWD/../include/bin/hint_file $$DESTDIR/ $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$PWD/../include/bin/LICENSE $$DESTDIR/ $$escape_expand(\\n\\t)
    message($$QMAKE_POST_LINK)
    export(QMAKE_POST_LINK)
}
