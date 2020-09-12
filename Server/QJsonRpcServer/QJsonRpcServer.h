#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <string>
#include <map>
#include <functional>
#include <QVariantList>




class QJsonRpcServer
{
    using Func = std::function<QVariant(const QVariantList&)>;
    using Params = const QStringList;
    struct Function {
        Func f;
        Params p;
        bool isVariadic;
        Function(Func&& func, Params&& params, bool variadic = false);
    };

    std::map<std::string, Function> m_methods;


public:

    QJsonRpcServer();

    void addMethodVariadicParameters(const std::string& methodName,
                   Func&& callback);

    void addMethod(const std::string& methodName,
                   Params& paramNames,
                   Func&& callback);

    QJsonDocument execute(const QJsonDocument& request);
    QJsonDocument execute(const std::string& request);

private:

    void chackArray(const QJsonArray& requestArray);
    void checkRequest(const QJsonDocument &request);
    QJsonDocument executeByType(const QJsonDocument &request);

    QJsonDocument executeArray(const QJsonArray& requestArray);
    QJsonDocument executeObject(const QJsonObject& obj);
    QJsonDocument executeObjectImpl(const QJsonObject& obj);

    QJsonDocument executeObjectNotification(const QJsonObject& obj);
    QJsonDocument executeObjectWithResult(const QJsonObject& obj);


    bool checkMethodExists(const std::string& methodName);
    Function getFunctionByName(const std::string& methodName);
    QVariant executeObjectByParametersType(const QJsonObject& obj, const Function& currentFunc);

    QVariantList namesToParameterList(const QJsonObject& objectParameters, const Params& methodParamNames);



    void checkObject(const QJsonObject& obj);
    void checkMethodParameters(const QJsonObject& params, const Params& paramNames);
};

