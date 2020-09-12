
GOOGLETEST_DIR = ../../googletest
include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread



HEADERS += \
        tst_json_rpc_client_test.h

SOURCES += \
        main.cpp
