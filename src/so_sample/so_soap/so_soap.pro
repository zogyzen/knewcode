#-------------------------------------------------
#
# Project created by QtCreator 2017-08-01T08:47:52
#
#-------------------------------------------------

QT       -= core gui

contains(QT_ARCH, i386) {
    BinPth = $$PWD/../../../bin32
} else {
    BinPth = $$PWD/../../../bin64
}
CONFIG(debug, debug | release) {
    DESTDIR += $$BinPth/debug
} else {
    DESTDIR += $$BinPth/release
}

TARGET = so_soap
TEMPLATE = lib

DEFINES += KC_APACHE_MOD_LIBRARY

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
        ctrl_soap.cpp \
        gsoap/soapC.cpp \
        gsoap/soapWKWebServiceSOAP11BindingProxy.cpp \
        gsoap/stdsoap2.cpp

HEADERS += \
        ctrl_soap.h \
        gsoap/WKWebServiceSOAP11Binding.nsmap \
        gsoap/soapH.h \
        gsoap/soapStub.h \
        gsoap/soapWKWebServiceSOAP11BindingProxy.h \
        gsoap/stdsoap2.h \
        gsoap/zzsdzfp.h \
        std.h

INCLUDEPATH += $$PWD/../../include

win32{
    BOOSTPTH = D:/library/boost_1_82
    INCLUDEPATH += $$BOOSTPTH
    LIBS += -L$$BOOSTPTH/lib64-msvc-14.1

    LIBS += -ldbghelp -lpsapi
    DEFINES += WIN32 WIN32_LEAN_AND_MEAN

    QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
    QMAKE_CXXFLAGS_STL_ON = /EHa
    QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE
    DEF_FILE = so_soap.def
}
unix {
    target.path = /usr/lib
    INSTALLS += target
    CONFIG += plugin

}
