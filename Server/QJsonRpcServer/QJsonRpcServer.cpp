#include <QJsonRpcServer.h>


#include <exception>
class ParseError : public std::exception
{
    std::string what_message {"Json parse error"};
    // exception interface
public:
    virtual const char *what() const noexcept override
    {
        return what_message.c_str();
    }
};


class InvalidRequest : public std::exception
{
    std::string what_message {"Request is not valid"};
    // exception interface
public:
    virtual const char *what() const noexcept override
    {
        return what_message.c_str();
    }
};


QJsonRpcServer::Function::Function(QJsonRpcServer::Func &&func, QJsonRpcServer::Params &&params, bool variadic)
    : f{func}
    , p{params}
    , isVariadic{variadic}
{

}

QJsonRpcServer::QJsonRpcServer()
{

}

void QJsonRpcServer::addMethodVariadicParameters(const std::string &methodName, QJsonRpcServer::Func &&callback)
{
    m_methods.insert(std::make_pair(methodName, Function(std::move(callback), {}, true)));
}

void QJsonRpcServer::addMethod(const std::string &methodName, QJsonRpcServer::Params &paramNames, QJsonRpcServer::Func &&callback)
{
    m_methods.insert(std::make_pair(methodName, Function(std::move(callback), std::move(paramNames))));
}

QJsonDocument QJsonRpcServer::execute(const QJsonDocument &request)
{
    try {
        checkRequest(request);
        return executeByType(request);
    }
    catch(const ParseError& exc)
    {
        qWarning() << exc.what();
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"error", QJsonObject{
                                      {"code", -32700},
                                      {"message", "Parse error"}
                                  }},
                                 {"id", QJsonValue::Null}
                             });
    }
    catch(const InvalidRequest& exc)
    {
        qWarning() << exc.what();
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"error", QJsonObject{
                                      {"code", -32600},
                                      {"message", "Invalid Request"}
                                  }},
                                 {"id", QJsonValue::Null}
                             });
    }
    catch(const std::exception& exc)
    {
        qWarning() << exc.what();
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"error", QJsonObject{
                                      {"code", -32603},
                                      {"message", "Internal error"}
                                  }},
                                 {"id", QJsonValue::Null}
                             });
    }

}

QJsonDocument QJsonRpcServer::execute(const std::string &request)
{
    return execute(QJsonDocument::fromJson(request.c_str()));
}


void QJsonRpcServer::checkRequest(const QJsonDocument &request)
{
    if(request.isNull())
        throw ParseError();

    if(request.isEmpty())
        throw InvalidRequest();
}

QJsonDocument QJsonRpcServer::executeByType(const QJsonDocument &request)
{
    if(request.isArray())
    {
        return executeArray(request.array());
    }
    else if(request.isObject())
    {
        return executeObject(request.object());
    }
    else
    {
        throw InvalidRequest();
    }
}

void QJsonRpcServer::chackArray(const QJsonArray &requestArray)
{
    if(requestArray.isEmpty())
        throw InvalidRequest();
}
QJsonDocument QJsonRpcServer::executeArray(const QJsonArray &requestArray)
{
    chackArray(requestArray);

    QJsonArray response;

    for(const auto& obj: requestArray)
    {
        QJsonDocument result = executeObject(obj.toObject());

        //is not notification
        if(!result.isEmpty())
            response.append(result.object());
    }

    //all is notificaltions
    if(response.isEmpty())
        return QJsonDocument{};
    else
        return QJsonDocument{response};
}

QJsonDocument QJsonRpcServer::executeObject(const QJsonObject &obj)
{
    try {
        checkObject(obj);
        return executeObjectImpl(obj);
    }
    catch(const InvalidRequest& exc)
    {
        qWarning() << exc.what();
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"error", QJsonObject{
                                      {"code", -32600},
                                      {"message", "Invalid Request"}
                                  }},
                                 {"id", QJsonValue::Null}
                             });
    }
    catch(const std::exception& exc)
    {
        qWarning() << exc.what();
        return QJsonDocument({
                                 {"jsonrpc", "2.0"},
                                 {"error", QJsonObject{
                                      {"code", -32603},
                                      {"message", "Internal error"}
                                  }},
                                 {"id", QJsonValue::Null}
                             });
    }


}

QJsonDocument QJsonRpcServer::executeObjectImpl(const QJsonObject &obj)
{
    //Is notification?
    if(obj.value("id").isUndefined())
        return executeObjectNotification(obj);
    else
        return executeObjectWithResult(obj);
}

QJsonDocument QJsonRpcServer::executeObjectNotification(const QJsonObject &obj)
{
    const std::string methodName = obj.value("method").toString().toStdString();

    if(!checkMethodExists(methodName))
        //throw MethodNotFound without ID???
        return  QJsonDocument{};


    Function currentFunc = getFunctionByName(methodName);
    executeObjectByParametersType(obj, currentFunc);

    return QJsonDocument{};
}

QJsonDocument QJsonRpcServer::executeObjectWithResult(const QJsonObject &obj)
{
    QJsonValue request_id = obj.value("id");
    const std::string methodName = obj.value("method").toString().toStdString();

    if(!checkMethodExists(methodName))
        //throw MethodNotFound with ID???
        return  QJsonDocument({{"jsonrpc", "2.0"},
                               {"error", QJsonObject{
                                    {"code", -32601},
                                    {"message", "Method not found"}
                                }},
                               {"id", request_id}});


    Function currentFunc = getFunctionByName(methodName);
    QVariant result = executeObjectByParametersType(obj, currentFunc);

    return QJsonDocument{{{
                {"jsonrpc", "2.0"},
                {"result", QJsonValue::fromVariant(result)},
                {"id", request_id}
            }}};
}

bool QJsonRpcServer::checkMethodExists(const std::string &methodName)
{
    return m_methods.find(methodName) != m_methods.end();
}

QJsonRpcServer::Function QJsonRpcServer::getFunctionByName(const std::string &methodName)
{
    return m_methods.at(methodName);
}

QVariant QJsonRpcServer::executeObjectByParametersType(const QJsonObject &obj, const Function& currentFunc)
{
    QVariant result;
    QJsonValue params = obj.value("params");
    if(params.isUndefined()) //func(void)
    {
        result = currentFunc.f({});
    }
    else if(params.isArray()) //positional parameters
    {
        result = currentFunc.f(params.toArray().toVariantList());
    }
    else if(params.isObject()) //named parameters
    {
        checkMethodParameters(params.toObject(), currentFunc.p);
        result = currentFunc.f(namesToParameterList(params.toObject(), currentFunc.p));
    }
    else {
        throw InvalidRequest();
    }

    return result;
}

QVariantList QJsonRpcServer::namesToParameterList(const QJsonObject &objectParameters, const QJsonRpcServer::Params &methodParamNames)
{
    QVariantList params;
    for(const auto& name: methodParamNames)
    {
        QVariant value = objectParameters.value(QString(name));
        params.append(value);
    }
    return params;
}

void QJsonRpcServer::checkObject(const QJsonObject &obj)
{
    if(obj.empty())
        throw InvalidRequest();

    if(obj.value("jsonrpc").toString() != QString("2.0"))
        throw InvalidRequest();

    if(obj.value("method").isUndefined())
        throw InvalidRequest();

    if(!obj.value("method").isString())
        throw InvalidRequest();

    if(!obj.value("params").isUndefined())
    {
        if(!obj.value("params").isArray() && !obj.value("params").isObject())
            throw InvalidRequest();
    }



    static QSet<QString> validElements {
        "jsonrpc", "method", "params", "id"
    };

    QStringList localKeys = obj.keys();
    QSet<QString> elements(localKeys.begin(), localKeys.end());

    //has invalid elements
    if(!elements.subtract(validElements).isEmpty())
        throw InvalidRequest();

}

void QJsonRpcServer::checkMethodParameters(const QJsonObject &params, const QJsonRpcServer::Params &paramNames)
{
    QStringList keys = params.keys();

    QSet<QString> keys_set(keys.begin(), keys.end());
    QSet<QString> params_set(paramNames.begin(), paramNames.end());

    if(keys_set != params_set)
        throw InvalidRequest();
}
