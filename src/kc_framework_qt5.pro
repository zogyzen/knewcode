TEMPLATE = subdirs

include($$PWD/kc_framework.pro)

QTVERSION = $$replace(QT_VERSION, \., )
message($$QTVERSION)

message($$QT_ARCH)
contains(QT_ARCH, i386) || contains(QT_ARCH, x86_64) {
    SUBDIRS += \
        # so_excel \
        kc_sql_odbc
}
