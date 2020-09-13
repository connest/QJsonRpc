#pragma once

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <string>

class QJsonRpcClient
{
public:
    enum MethodType {
        Notification,
        DirectCall //TODO rename
    };

    QJsonRpcClient();
    QJsonDocument execute(const std::string& methodName,
                          const QJsonValue& params,
                          MethodType type = MethodType::DirectCall);
    QJsonDocument execute(const std::string& methodName,
                          MethodType type = MethodType::DirectCall);

    bool validate(const QJsonDocument& response);
    bool isError(const QJsonDocument& response);

private:
    int m_currentId {0};

    bool validateObject(const QJsonObject& obj);
    bool isObjectError(const QJsonObject& obj);
};

