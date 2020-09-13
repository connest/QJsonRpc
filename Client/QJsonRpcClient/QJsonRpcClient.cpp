#include "QJsonRpcClient.h"

#include <QObject>
QJsonRpcClient::QJsonRpcClient()
{

}

QJsonDocument QJsonRpcClient::execute(const std::vector<QJsonRpcClient::Request> &batchRequest)
{
    QJsonArray requests;
    for(const auto& requestObject: batchRequest)
    {
        const QJsonDocument& requestDocument = execute(requestObject);
        requests.append(requestDocument.object());
    }

    return QJsonDocument(requests);
}

QJsonDocument QJsonRpcClient::execute(const QJsonRpcClient::Request &request)
{
    if(request.m_params.isUndefined())
        return execute(request.m_methodName, request.m_type);
    else
        return execute(request.m_methodName, request.m_params, request.m_type);
}

QJsonDocument QJsonRpcClient::execute(const std::string &methodName,
                                      const QJsonValue &params,
                                      QJsonRpcClient::MethodType type)
{
    if(type == MethodType::DirectCall)
    {
        //ID will begin with 1
        ++m_currentId;
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"method", methodName.c_str()},
                                 {"params", params},
                                 {"id", m_currentId}
                             });
    }
    else if(type == MethodType::Notification)
    {
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"method", methodName.c_str()},
                                 {"params", params}
                             });
    }
    else
        throw std::runtime_error("Invalid method type");
}

QJsonDocument QJsonRpcClient::execute(const std::string &methodName, QJsonRpcClient::MethodType type)
{
    if(type == MethodType::DirectCall)
    {
        //ID will begin with 1
        ++m_currentId;
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"method", methodName.c_str()},
                                 {"id", m_currentId}
                             });
    }
    else if(type == MethodType::Notification)
    {
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"method", methodName.c_str()},
                             });
    }
    else
        throw std::runtime_error("Invalid method type");
}

bool QJsonRpcClient::validate(const QJsonDocument &response)
{
    if(response.isEmpty())
        return false;

    if(response.isNull())
        return false;

    if(response.isArray())
    {
        const std::vector<bool> validStates = validateBatch(response);
        for(const auto& state: validStates)
            if(!state)
                return false;
        return true;
    }
    else if(response.isObject())
    {
        return validateObject(response.object());
    }
    else return false;
}

bool QJsonRpcClient::isError(const QJsonDocument &response)
{
    if(response.isArray())
    {
        //TODO check for batch response
        return false;
    }
    else if(response.isObject())
    {
        return isObjectError(response.object());
    }
    return false;
}

std::vector<bool> QJsonRpcClient::validateBatch(const QJsonDocument &response)
{
    const QJsonArray& responseArray = response.array();

    //Default fill as falses
    std::vector<bool> result;
    result.reserve(responseArray.count());

    for(const auto& responseObject: responseArray)
    {
        if(!responseObject.isObject())
        {
            result.push_back(false);
        }
        else
        {
            const bool validFlag = validateObject(responseObject.toObject());
            result.push_back(validFlag);
        }
    }

    return result;
}

bool QJsonRpcClient::isBatch(const QJsonDocument &response)
{
    return response.isArray();
}

bool QJsonRpcClient::validateObject(const QJsonObject &obj)
{
    if(isObjectError(obj))
    {

        static const QSet<QString> validKeyErrorWords{
            "jsonrpc", "error", "id"
        };

        const QStringList& localKeys = obj.keys();
        QSet<QString> currentKeys {localKeys.begin(), localKeys.end()};

        if(currentKeys != validKeyErrorWords)
            return false;

        QJsonObject errorObject = obj.value("error").toObject();
        int errorCode = errorObject.value("code").toInt();

        /*
            -32700 ---> parse error. not well formed
            -32701 ---> parse error. unsupported encoding
            -32702 ---> parse error. invalid character for encoding
            -32600 ---> server error. invalid xml-rpc. not conforming to spec.
            -32601 ---> server error. requested method not found
            -32602 ---> server error. invalid method parameters
            -32603 ---> server error. internal xml-rpc error
            -32500 ---> application error
            -32400 ---> system error
            -32300 ---> transport error
            -32099 .. -32000 ---> implementation defined server errors
        */

        if(     !(
                    (errorCode <= -32700 && errorCode >= -32702) ||
                    (errorCode <= -32600 && errorCode >= -32603) ||
                    (errorCode == 32500) ||
                    (errorCode == 32400) ||
                    (errorCode == 32300) ||
                    (errorCode >= -32099 && errorCode <= -32000)
                    )
                )
            return false;

        QString errorMessage = errorObject.value("message").toString();

        if(errorCode <= -32700 && errorCode >= -32702)
        {
            if(errorMessage != QString("Parse error"))
                return false;
        }

        if(errorCode == -32600)
        {
            if(errorMessage != QString("Invalid Request"))
                return false;
        }

        if(errorCode == -32601)
        {
            if(errorMessage != QString("Method not found"))
                return false;
        }

        if(errorCode == -32602)
        {
            if(errorMessage != QString("Invalid params"))
                return false;
        }

        if(errorCode == -32603)
        {
            if(errorMessage != QString("Internal error"))
                return false;
        }

        if(errorCode >= -32099 && errorCode <= -32000)
        {
            if(errorMessage != QString("Server error"))
                return false;
        }



        if(!obj.value("id").isNull())
        {
            if(obj.value("id").toInt() < 1 || obj.value("id").toInt() > m_currentId)
                return false;
        }
    }
    else
    {

        static const QSet<QString> validKeyResultWords{
            "jsonrpc", "result", "id"
        };

        const QStringList& localKeys = obj.keys();
        QSet<QString> currentKeys {localKeys.begin(), localKeys.end()};

        if(currentKeys != validKeyResultWords)
            return false;


        if(obj.value("id").toInt() < 1 || obj.value("id").toInt() > m_currentId)
            return false;
    }





    if(obj.value("jsonrpc").toString() != QString("2.0"))
        return false;

    return true;
}

bool QJsonRpcClient::isObjectError(const QJsonObject &obj)
{
    return obj.keys().contains("error");
}



QJsonRpcClient::Request::Request(const std::string &methodName, const QJsonValue &params, const QJsonRpcClient::MethodType type)
    : m_methodName {methodName}
    , m_params {params}
    , m_type {type}
{

}

QJsonRpcClient::Request::Request(const std::string &methodName, const QJsonRpcClient::MethodType type)
    : m_methodName {methodName}
    , m_params {QJsonValue::Undefined}
    , m_type {type}
{

}
