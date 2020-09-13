#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QJsonRpcClient.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace testing;

TEST(Json_RPC_Clent, Response)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument request ({{"jsonrpc", "2.0"},
                            {"method", "myMethod"},
                            {"params", QJsonArray{1, 2, 3}},
                            {"id", 1}
    });
    QJsonDocument result;

    //Act
    result = rpc.execute("myMethod", QJsonArray{1, 2, 3}, QJsonRpcClient::DirectCall);

    //Assert
    ASSERT_EQ(result, request);
}

TEST(Json_RPC_Clent, Response_no_parameters)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument request ({{"jsonrpc", "2.0"},
                            {"method", "myMethod"},
                            {"id", 1}
                           });
    QJsonDocument result;

    //Act
    result = rpc.execute("myMethod");

    //Assert
    ASSERT_EQ(result, request);
}

TEST(Json_RPC_Clent, Notification_positional_parameters)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument request ({{"jsonrpc", "2.0"},
                            {"method", "notification"},
                            {"params", QJsonArray{1, 2, 3}},
                           });
    QJsonDocument result;

    //Act
    result = rpc.execute("notification", QJsonArray{1, 2, 3}, QJsonRpcClient::Notification);

    //Assert
    ASSERT_EQ(result, request);
}

TEST(Json_RPC_Clent, Notification_named_parameters)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument request ({{"jsonrpc", "2.0"},
                            {"method", "notification"},
                            {"params", QJsonObject{
                                 {"param1", 1},
                                 {"param2", 2}
                             }
                            }
                           });
    QJsonDocument result;

    //Act
    result = rpc.execute("notification", QJsonObject{{"param1", 1}, {"param2", 2}}, QJsonRpcClient::Notification);

    //Assert
    ASSERT_EQ(result, request);
}

TEST(Json_RPC_Clent, Notification_named_parameters_reverse)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument request ({{"jsonrpc", "2.0"},
                            {"method", "notification"},
                            {"params", QJsonObject{
                                 {"param2", 1},
                                 {"param1", 2}
                             }
                            }
                           });
    QJsonDocument result;

    //Act
    result = rpc.execute("notification", QJsonObject{{"param2", 1}, {"param1", 2}}, QJsonRpcClient::Notification);

    //Assert
    ASSERT_EQ(result, request);
}

TEST(Json_RPC_Clent, Response_validate_valid)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument valid_response ({{"jsonrpc", "2.0"}, {"result", 19}, {"id", 1}});
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(valid_response);

    //Assert
    ASSERT_EQ(result, true);
}

TEST(Json_RPC_Clent, Response_validate_ID_not_1)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument valid_response ({{"jsonrpc", "2.0"}, {"result", 19},
                                   {"id", 2}}); // VALID, because 2 executions
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    rpc.execute("dummy1", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(valid_response);

    //Assert
    ASSERT_EQ(result, true);
}

TEST(Json_RPC_Clent, Response_validate_header_not_exists)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument invalid_response ({/*{"jsonrpc", "2.0"},*/ {"result", 19}, {"id", 1}});
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(invalid_response);

    //Assert
    ASSERT_EQ(result, false);
}

TEST(Json_RPC_Clent, Response_validate_invalid_header)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument invalid_response ({{"jsonrpc", "INVALID"}, {"result", 19}, {"id", 1}});
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(invalid_response);

    //Assert
    ASSERT_EQ(result, false);
}

TEST(Json_RPC_Clent, Response_validate_invalid_body)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument invalid_response ({{"jsonrpc", "2.0"}, {"INVALID_BODY", 42}, {"id", 1}});
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(invalid_response);

    //Assert
    ASSERT_EQ(result, false);
}

TEST(Json_RPC_Clent, Response_validate_invalid_id)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument invalid_response ({{"jsonrpc", "2.0"}, {"result", 42},
                                   {"id", 666}}); //INVALID ID
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(invalid_response);

    //Assert
    ASSERT_EQ(result, false);
}

TEST(Json_RPC_Clent, Response_validate_with_error)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument valid_error_response ({{"jsonrpc", "2.0"},
                                                          {"error", QJsonObject{
                                                               {"code", -32601},
                                                               {"message", "Method not found"}
                                                           }},
                                                          {"id", 1}});
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(valid_error_response);

    //Assert
    ASSERT_EQ(result, true);
}

TEST(Json_RPC_Clent, Response_validate_with_error_id_null)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument valid_error_response ({{"jsonrpc", "2.0"},
                                         {"error", QJsonObject{
                                              {"code", -32700},
                                              {"message", "Parse error"}
                                          }},
                                         {"id", QJsonValue::Null}});
    bool result;

    //Act
    result = rpc.validate(valid_error_response);

    //Assert
    ASSERT_EQ(result, true);
}

TEST(Json_RPC_Clent, Response_validate_invalid_error_code)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument invalid_error_response ({{"jsonrpc", "2.0"},
                                         {"error", QJsonObject{
                                              {"code", 666}, //INVALID error code
                                              {"message", "Some message"}
                                          }},
                                         {"id", 1}});
    bool result;

    //Act
    rpc.execute("dummy", QJsonObject{{"param1", 1}, {"param2", 2}}); // Save request ID
    result = rpc.validate(invalid_error_response);

    //Assert
    ASSERT_EQ(result, false);
}


TEST(Json_RPC_Clent, Response_validate_parse_error)
{
    //Arrnge
    QJsonRpcClient rpc;
    const std::string invalid_json = R"({"INVALID": ]foo{bar}}}})";
    QJsonDocument invalid_json_document  = QJsonDocument::fromJson(invalid_json.c_str());
    bool result;

    //Act
    result = rpc.validate(invalid_json_document);

    //Assert
    ASSERT_EQ(result, false);
}

TEST(Json_RPC_Clent, Is_error)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument response_error({{"jsonrpc", "2.0"},
                                              {"error", QJsonObject{
                                                   {"code", -32601},
                                                   {"message", "Method not found"}
                                               }},
                                              {"id", 1}});
    bool result;

    //Act
    result = rpc.isError(response_error);

    //Assert
    ASSERT_EQ(result, true);
}

TEST(Json_RPC_Clent, Is_not_error)
{
    //Arrnge
    QJsonRpcClient rpc;
    QJsonDocument response_not_error ({{"jsonrpc", "2.0"}, {"result", 19}, {"id", 4}});

    bool result;

    //Act
    result = rpc.isError(response_not_error);

    //Assert
    ASSERT_EQ(result, false);
}

