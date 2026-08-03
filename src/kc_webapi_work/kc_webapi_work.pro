#-------------------------------------------------
#
# Project created by QtCreator 2017-06-11T22:24:43
# 后端API主处理模块（处理.kc文件）
#
#-------------------------------------------------

QT       -= core gui

# 相同的配置
include($$PWD/../same.pro)

INCLUDEPATH += $$LIBRARYPTH3RD/common
INCLUDEPATH += $$LIBRARYPTH3RD/common/json

TARGET = kc_webapi_work
TEMPLATE = lib
DEFINES += KC_WEBAPI_WORK_LIBRARY

# cJSON库
CJSONPTH = $$LIBRARYPTH3RD/common/json/cJSON/src
# yyjson库
YYJSONPTH = $$LIBRARYPTH3RD/common/json/yyjson/src

SOURCES += \
        $$CJSONPTH/cJSON.c \
        $$YYJSONPTH/yyjson.c \
        action_data.cpp \
        activator.cpp \
        ctrlapi_data.cpp \
        json_cjson.cpp \
        #json_yyjson.cpp \
        json_pack.cpp \
        request_respond_parm.cpp \
        kc_webapi_work.cpp

HEADERS += \
        $$CJSONPTH/cJSON.h \
        $$YYJSONPTH/yyjson.h \
        action_data.h \
        ctrlapi_data.h \
        json_cjson.h \
        #json_yyjson.h \
        json_pack.h \
        request_respond_parm.h \
        kc_webapi_work.h \
        std.h

win32 {
    msvc {
        DEF_FILE = activator.def
        DISTFILES += activator.def
    }
    mingw {
    }
}
unix {
    QMAKE_LFLAGS += -Wl,--version-script=$$PWD/version_script.map
    DISTFILES += version_script.map
}
