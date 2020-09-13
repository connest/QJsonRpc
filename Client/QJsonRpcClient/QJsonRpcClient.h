#pragma once

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <string>
#include <vector>

class QJsonRpcClient
{
public:
    enum class MethodType {
        Notification,
        DirectCall //TODO rename
    };
    struct Request {
        const std::string m_methodName;
        const QJsonValue m_params;
        const MethodType m_type;
        Request(const std::string& methodName,
                 const QJsonValue& params,
                 const MethodType type = MethodType::DirectCall);
        Request(const std::string& methodName,
                 const MethodType type = MethodType::DirectCall);
    };

    QJsonRpcClient();
    QJsonDocument execute(const std::vector<Request>& batchRequest);
    QJsonDocument execute(const Request& request);
    QJsonDocument execute(const std::string& methodName,
                          const QJsonValue& params,
                          MethodType type = MethodType::DirectCall);
    QJsonDocument execute(const std::string& methodName,
                          MethodType type = MethodType::DirectCall);

    bool validate(const QJsonDocument& response);
    bool isError(const QJsonDocument& response);

    std::vector<bool> validateBatch(const QJsonDocument& response);
    bool isBatch(const QJsonDocument& response);
private:
    int m_currentId {0};

    bool validateObject(const QJsonObject& obj);
    bool isObjectError(const QJsonObject& obj);
};

