#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>

using namespace testing;

class JsonRpcTest : public ::testing::Test {

protected:
    JsonRpcServer* rpc;
    // Test interface
protected:
    virtual void SetUp() override
    {
        using namespace std::string_literals;

        rpc = new JsonRpcServer();

        rpc->addMethod("subtract"s, {"subtrahend", "minuend"},
                       [](const QVariantList& args) -> QVariant {
            int subtrahend  = args[0].toInt();
            int minuend     = args[1].toInt();
            return subtrahend - minuend;

        });
    }
    virtual void TearDown() override
    {
        delete rpc;
    }

};

/*
    --> {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1}
    <-- {"jsonrpc": "2.0", "result": 19, "id": 1}
*/
TEST_F(JsonRpcTest, Request_positional)
{
    //Arrange
    QJsonDocument request_straight({{"jsonrpc", "2.0"}, {"method", "subtract"},
                                    {"params", QJsonArray{42, 23}}, {"id", 1}});

    QJsonDocument response_straight ({{"jsonrpc", "2.0"}, {"result", 19}, {"id", 1}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_straight);

    //Assert
    ASSERT_EQ(result, response_straight);

}

/*
    --> {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": "1"}
    <-- {"jsonrpc": "2.0", "result": 19, "id": "1"}
*/
TEST_F(JsonRpcTest, Request_positional_id_string)
{
    //Arrange
    QJsonDocument request_straight({{"jsonrpc", "2.0"}, {"method", "subtract"},
                                    {"params", QJsonArray{42, 23}}, {"id", "1"}});

    QJsonDocument response_straight ({{"jsonrpc", "2.0"}, {"result", 19}, {"id", "1"}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_straight);

    //Assert
    ASSERT_EQ(result, response_straight);

}

/*
    --> {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1}
    <-- {"jsonrpc": "2.0", "result": 19, "id": 1}
*/
TEST_F(JsonRpcTest, Request_positional_string_request)
{
    //Arrange
    const std::string request_straight = R"({"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1})";

    QJsonDocument response_straight ({{"jsonrpc", "2.0"}, {"result", 19}, {"id", 1}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_straight);

    //Assert
    ASSERT_EQ(result, response_straight);

}

/*
    --> {"jsonrpc": "2.0", "method": "subtract", "params": [23, 42], "id": 2}
    <-- {"jsonrpc": "2.0", "result": -19, "id": 2}
 */
TEST_F(JsonRpcTest, Request_positional_reverse)
{
    //Arrange

    QJsonDocument request_reverse({{"jsonrpc", "2.0"}, {"method", "subtract"},
                                   {"params", QJsonArray{23, 42}}, {"id", 2}});

    QJsonDocument response_reverse  ({{"jsonrpc", "2.0"}, {"result", -19}, {"id", 2}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_reverse);

    //Assert
    ASSERT_EQ(result, response_reverse);
}

/*
    --> {"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3}
    <-- {"jsonrpc": "2.0", "result": 19, "id": 3}
*/
TEST_F(JsonRpcTest,  Request_named)
{
    //Arrange

    QJsonDocument request_straight({{"jsonrpc", "2.0"}, {"method", "subtract"},
                                    {"params", QJsonObject{{"subtrahend", 42}, {"minuend", 23}}}, {"id", 3}});

    QJsonDocument response ({{"jsonrpc", "2.0"}, {"result", 19}, {"id", 3}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_straight);

    //Assert
    ASSERT_EQ(result, response);
}

/*
    --> {"jsonrpc": "2.0", "method": "subtract", "params": {"minuend": 42, "subtrahend": 23}, "id": 4}
    <-- {"jsonrpc": "2.0", "result": 19, "id": 4}
*/
TEST_F(JsonRpcTest,  Request_named_reverse)
{
    //Arrange
    QJsonDocument request_reverse({{"jsonrpc", "2.0"}, {"method", "subtract"},
                                   {"params", QJsonObject{{"minuend", 23}, {"subtrahend", 42}}}, {"id", 4}});

    QJsonDocument response ({{"jsonrpc", "2.0"}, {"result", 19}, {"id", 4}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_reverse);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> {"jsonrpc": "2.0", "method": "update", "params": [1,2,3,4,5]}
<-- Nothing
*/

TEST_F(JsonRpcTest, Notification_parameters)
{
    //Arrange
    QJsonDocument request({{"jsonrpc", "2.0"},
                           {"method", "update"},
                           {"params", QJsonArray{1,2,3,4,5}}});
    QJsonDocument result;

    int a,b,c,d,e;
    rpc->addMethod("update", {"a","b","c","d","e"},[&](const auto& args){
        a  = args[0].toInt();
        b  = args[1].toInt();
        c  = args[2].toInt();
        d  = args[3].toInt();
        e  = args[4].toInt();
        return QVariant{};
    });

    //Act
    result = rpc->execute(request);

    //Assert
    EXPECT_TRUE(result.isEmpty());

    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 2);
    EXPECT_EQ(c, 3);
    EXPECT_EQ(d, 4);
    EXPECT_EQ(e, 5);
}

/*
--> {"jsonrpc": "2.0", "method": "foobar"}
<-- Nothing
*/

TEST_F(JsonRpcTest, Notification)
{
    //Arrange
    QJsonDocument request({{"jsonrpc", "2.0"},
                           {"method", "foobar"}});
    QJsonDocument result;

    bool flag_is_foobar_executeuted{false};
    rpc->addMethod("foobar", {},[&](const auto& args){
        Q_UNUSED(args)
        flag_is_foobar_executeuted = true;
        return QVariant{};
    });

    //Act
    result = rpc->execute(request);

    //Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(flag_is_foobar_executeuted, true);
}

/*
--> {"jsonrpc": "2.0", "method": "INVALID"}
<-- Nothing
*/

TEST_F(JsonRpcTest, Notification_method_mot_exists)
{
    //Arrange
    QJsonDocument request({{"jsonrpc", "2.0"},
                           {"method", "INVALID"}});
    QJsonDocument result;


    //Act
    result = rpc->execute(request);

    //Assert
    EXPECT_TRUE(result.isEmpty());
}

/*
--> {"jsonrpc": "2.0", "method": "foobar", "id": "1"}
<-- {"jsonrpc": "2.0", "error": {"code": -32601, "message": "Method not found"}, "id": "1"}
*/
TEST_F(JsonRpcTest, Method_not_exists)
{
    //Arrange
    QJsonDocument request({{"jsonrpc", "2.0"},
                           {"method", "INVALID_METHOD"},
                           {"id", 1}});

    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"error", QJsonObject{
                                 {"code", -32601},
                                 {"message", "Method not found"}
                             }},
                            {"id", 1}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> {"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz]
<-- {"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null}
*/
TEST_F(JsonRpcTest, Invalid_json)
{
    //Arrange
    const std::string invalid_json_request = R"({"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz])";

    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"error", QJsonObject{
                                 {"code", -32700},
                                 {"message", "Parse error"}
                             }},
                            {"id", QJsonValue::Null}});
    QJsonDocument result;

    //Act
    result = rpc->execute(invalid_json_request);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> {"jsonrpc": "2.0", "method": 1, "params": "bar"}
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
*/
TEST_F(JsonRpcTest, Invalid_request_object)
{
    //Arrange
    QJsonDocument invalid_request({{"jsonrpc", "2.0"},
                                   {"method", 1},
                                   {"params", "bar"}});

    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"error", QJsonObject{
                                 {"code", -32600},
                                 {"message", "Invalid Request"}
                             }},
                            {"id", QJsonValue::Null}});
    QJsonDocument result;

    //Act
    result = rpc->execute(invalid_request);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> {"jsonrpc": "2.0", "method": "subtract", "params": {"INVALID": 42,"minuend": 42, "subtrahend": 23}, "id": 3}
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
*/
TEST_F(JsonRpcTest, Invalid_parameters)
{
    //Arrange

    QJsonDocument request_invalid_parameters({{"jsonrpc", "2.0"}, {"method", "subtract"},
                                    {"params", QJsonObject{{"INVALID", 42}, {"subtrahend", 42}, {"minuend", 23}}}, {"id", 3}});


    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"error", QJsonObject{
                                 {"code", -32600},
                                 {"message", "Invalid Request"}
                             }},
                            {"id", QJsonValue::Null}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_invalid_parameters);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> {"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23}, "id": 3}
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
*/
TEST_F(JsonRpcTest, Paramenter_not_exists)
{
    //Arrange

    QJsonDocument request_invalid_parameters({{"jsonrpc", "2.0"}, {"method", "subtract"},
                                              {"params", QJsonObject{{"subtrahend", 23}}}, {"id", 3}});


    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"error", QJsonObject{
                                 {"code", -32600},
                                 {"message", "Invalid Request"}
                             }},
                            {"id", QJsonValue::Null}});
    QJsonDocument result;

    //Act
    result = rpc->execute(request_invalid_parameters);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> {"jsonrpc": "2.0", "method": "sum", "params": [1,5,10], "id": "3"}
<-- {"jsonrpc": "2.0", "result": 16, "id": "3}
*/
TEST_F(JsonRpcTest, Variadic_parameters_count)
{
    //Arrange

    QJsonDocument request_invalid_parameters({{"jsonrpc", "2.0"}, {"method", "sum"},
                                              {"params", QJsonArray{1,5,10}}, {"id", "3"}});


    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"result", 16},
                            {"id", "3"}});
    QJsonDocument result;

    rpc->addMethodVariadicParameters("sum", [](const auto& args){
        int res = 0;
        for(const auto& arg: args)
            res += arg.toInt();

        return res;
    });

    //Act
    result = rpc->execute(request_invalid_parameters);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> [
  {"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
  {"jsonrpc": "2.0", "method"
]
<-- {"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null}
*/
TEST_F(JsonRpcTest, Invalid_json_batch)
{
    //Arrange
    const std::string invalid_json_request = R"(["
                                             "{"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},"
                                             "{"jsonrpc": "2.0", "method"
                                             ])";

    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"error", QJsonObject{
                                 {"code", -32700},
                                 {"message", "Parse error"}
                             }},
                            {"id", QJsonValue::Null}});
    QJsonDocument result;

    //Act
    result = rpc->execute(invalid_json_request);

    //Assert
    ASSERT_EQ(result, response);
}
/*
--> []
<-- {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
*/
TEST_F(JsonRpcTest, Empty_array)
{
    //Arrange
    QJsonDocument empty_array_request{QJsonArray{}};

    QJsonDocument response({{"jsonrpc", "2.0"},
                            {"error", QJsonObject{
                                 {"code", -32600},
                                 {"message", "Invalid Request"}
                             }},
                            {"id", QJsonValue::Null}});
    QJsonDocument result;

    //Act
    result = rpc->execute(empty_array_request);

    //Assert
    ASSERT_EQ(result, response);
}
/*
--> [1]
<-- [
  {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
]
*/
TEST_F(JsonRpcTest, Invalid_nonEmpty_array)
{
    //Arrange
    QJsonDocument empty_array_request{QJsonArray{1}};

    QJsonDocument response(QJsonArray{
                               QJsonObject{{"jsonrpc", "2.0"},
                                           {"error", QJsonObject{
                                                {"code", -32600},
                                                {"message", "Invalid Request"}
                                            }},
                                           {"id", QJsonValue::Null}}
                           });
    QJsonDocument result;

    //Act
    result = rpc->execute(empty_array_request);

    //Assert
    ASSERT_EQ(result, response);
}
/*
--> [1,2,3]
<-- [
  {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null},
  {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null},
  {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null}
]
*/
TEST_F(JsonRpcTest, Invalid_nonEmpty_batch)
{
    //Arrange
    QJsonDocument empty_array_request{QJsonArray{1,2,3}};

    QJsonDocument response(QJsonArray{
                               QJsonObject{{"jsonrpc", "2.0"},
                                           {"error", QJsonObject{
                                                {"code", -32600},
                                                {"message", "Invalid Request"}
                                            }},
                                           {"id", QJsonValue::Null}},
                               QJsonObject{{"jsonrpc", "2.0"},
                                           {"error", QJsonObject{
                                                {"code", -32600},
                                                {"message", "Invalid Request"}
                                            }},
                                           {"id", QJsonValue::Null}},
                               QJsonObject{{"jsonrpc", "2.0"},
                                           {"error", QJsonObject{
                                                {"code", -32600},
                                                {"message", "Invalid Request"}
                                            }},
                                           {"id", QJsonValue::Null}}
                           });
    QJsonDocument result;

    //Act
    result = rpc->execute(empty_array_request);

    //Assert
    ASSERT_EQ(result, response);
}

/*
--> [
        {"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
        {"jsonrpc": "2.0", "method": "notify_hello", "params": [7]},
        {"jsonrpc": "2.0", "method": "subtract", "params": [42,23], "id": "2"},
        {"foo": "boo"},
        {"jsonrpc": "2.0", "method": "foo.get", "params": {"name": "myself"}, "id": "5"},
        {"jsonrpc": "2.0", "method": "get_data", "id": "9"}
    ]
<-- [
        {"jsonrpc": "2.0", "result": 7, "id": "1"},
        {"jsonrpc": "2.0", "result": 19, "id": "2"},
        {"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": null},
        {"jsonrpc": "2.0", "error": {"code": -32601, "message": "Method not found"}, "id": "5"},
        {"jsonrpc": "2.0", "result": ["hello", 5], "id": "9"}
    ]
*/
TEST_F(JsonRpcTest, Batch_mix_errors_and_valid)
{
    //Arrange
    const std::string mix_request = R"([
                                    {"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
                                    {"jsonrpc": "2.0", "method": "notify_hello", "params": [7]},
                                    {"jsonrpc": "2.0", "method": "subtract", "params": [42,23], "id": "2"},
                                    {"foo": "boo"},
                                    {"jsonrpc": "2.0", "method": "foo.get", "params": {"name": "myself"}, "id": "5"},
                                    {"jsonrpc": "2.0", "method": "get_data", "id": "9"}
                                ])";

    QJsonDocument response(QJsonArray{
                               QJsonObject{{"jsonrpc", "2.0"}, {"result", 7}, {"id", "1"}},
                               QJsonObject{{"jsonrpc", "2.0"}, {"result", 19}, {"id", "2"}},
                               QJsonObject{{"jsonrpc", "2.0"}, {"error", QJsonObject{{"code", -32600}, {"message", "Invalid Request"}}}, {"id", QJsonValue::Null}},
                               QJsonObject{{"jsonrpc", "2.0"}, {"error", QJsonObject{{"code", -32601}, {"message", "Method not found"}}},{"id", "5"}},
                               QJsonObject{{"jsonrpc", "2.0"}, {"result", QJsonArray{"hello", 5}}, {"id", "9"}}
                           });
    QJsonDocument result;

    rpc->addMethod("sum", {}, [](const auto& args){
        int res = 0;
        for(const auto& arg: args)
            res += arg.toInt();
        return res;
    });

    bool notify_hello_is_called{false};
    bool notify_hello_parameter_equals{false};
    rpc->addMethod("notify_hello", {}, [&](const auto& args){
        notify_hello_is_called = true;
        notify_hello_parameter_equals = (args[0].toInt() == 7);
        return QVariant{};
    });


    rpc->addMethod("get_data", {}, [&](const auto& args){
        Q_UNUSED(args)
        return QJsonArray{"hello", 5};
    });



    //Act
    result = rpc->execute(mix_request);

    //Assert
    EXPECT_EQ(result, response);
    EXPECT_TRUE(notify_hello_is_called);
    EXPECT_TRUE(notify_hello_parameter_equals);
}
/*
--> [
        {"jsonrpc": "2.0", "method": "notify_sum", "params": [1,2,4]},
        {"jsonrpc": "2.0", "method": "notify_hello", "params": [7]}
    ]
<-- //Nothing is returned for all notification batches
*/
TEST_F(JsonRpcTest, Batch_all_notifications)
{
    //Arrange
    const std::string mix_request = R"([
                                    {"jsonrpc": "2.0", "method": "notify_sum", "params": [1,2,4]},
                                    {"jsonrpc": "2.0", "method": "notify_hello", "params": [7]}
                                ])";

    QJsonDocument result;

    bool notify_sum_called{false};
    int result_sum;

    rpc->addMethod("notify_sum", {}, [&](const auto& args){
        notify_sum_called = true;

        int res = 0;
        for(const auto& arg: args)
            res += arg.toInt();

        result_sum = res;

        return QVariant{};
    });


    bool notify_hello_called{false};
    int hello_notify_param{-1};
    rpc->addMethod("notify_hello", {}, [&](const auto& args){
        notify_hello_called = true;

        hello_notify_param = args[0].toInt();

        return QVariant{};
    });

    //Act
    result = rpc->execute(mix_request);

    //Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_TRUE(notify_sum_called);
    EXPECT_EQ(result_sum, 1 + 2 + 4);
    EXPECT_EQ(hello_notify_param, 7);
}

