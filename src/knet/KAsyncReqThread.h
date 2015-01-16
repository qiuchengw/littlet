#pragma once

#include <QObject>
#include <QThread>

#include "deps/qxtglobal.h"
#include "deps/nzmqt/nzmqt.hpp"

/**
 *  异步的数据收发		
 *
 *
**/
class KAsyncReqThread : public QThread
{
    QXT_DECLARE_PRIVATE(KAsyncReqThread);

public:
    KAsyncReqThread();
    ~KAsyncReqThread();

    /**
     *	@param
     *		-[in]
     *          json_config     连接配置
    **/
    bool Startup(const QString& json_config);

    void Shutdown();

    /**
     *	异步发送数据请求	
     *
     *	@param
     *		-[in]
     *      sender_obj  发送者对象，必须为QObject的派生类，会通过信号告知返回数据
     *      connection_name     使用哪个socket向外发送
     *      service_name        服务名，调用服务器上的什么服务
     *      data                需要发送的数据
    **/
    qint64 PostRequest(QObject* sender_obj, const char* slot, 
        const QString& connection_name, const QString& service_name, 
        const QByteArray& data);
  
    void AddConnection(const QString&connection_name,
        nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen);

    void SubscribeHead(int head_no, QObject* obj, const char* slot);

    void UnregisterRequestForObject(QObject* obj);

protected:
    virtual void run();
};

