
GOOGLETEST_DIR = ../../googletest

include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt



HEADERS += \
        tst_json_rpc_server_test.h

SOURCES += \
        main.cpp
