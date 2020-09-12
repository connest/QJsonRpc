
GOOGLETEST_DIR = ../../googletest

include(gtest_dependency.pri)


TEMPLATE = app

QT = core

CONFIG += thread

INCLUDEPATH += ../QJsonRpcServer

HEADERS += \
        tst_json_rpc_server_test.h

SOURCES += \
        main.cpp \
        ../QJsonRpcServer/QJsonRpcServer.cpp
