#ifndef KSYNCREQUEST_H
#define KSYNCREQUEST_H


#include <QtCore/qglobal.h>

#ifdef KSYNCREQUEST_LIB
# define KSYNCREQUEST_EXPORT Q_DECL_EXPORT
#else
# define KSYNCREQUEST_EXPORT Q_DECL_IMPORT
#endif

class QObject;

typedef QList<QByteArray> ReciveMessage;

extern "C"
{

    /**
     *	启动网络请求服务	
     *
    **/
    KSYNCREQUEST_EXPORT bool KStartNetRequestService();

    /**
    *	同步发送请求
    *
    *	@return
    *		true    发送成功
    *
    *	@param
    *		-[in]
    *          name        请求服务器的服务名
    *          data        数据
    *          out_data    数据请求的返回数据
    **/
    KSYNCREQUEST_EXPORT bool KSyncSendRequest(const QString& name, const QList<QString>& data,
        QList<QString>& out_data);

    KSYNCREQUEST_EXPORT bool KSyncSendRequestA(const QString& name, const QString& data,
        QList<QString>& out_data);

    /**
    *	异步发送请求
    *
    *	@return
    *		true    发送成功
    *
    *	@param
    *		-[in]
    *          obj         请求者
    *          slot        消息返回后的回调槽
    *          connection_name 连接名，使用哪个连接
    *          name        请求服务器的服务名
    *          data        数据
    **/
    KSYNCREQUEST_EXPORT qint64 KAsyncPostRequest(QObject* obj, const char* slot,
        const QString& connection_name,const QString& service_name, const QByteArray& data);

    /**
    *	异步发送请求
    *       如果想接收数据，必须知道服务器返回的数据的包头号！
    *       并使用 KSubscribeHead 订阅这个消息
    **/
    KSYNCREQUEST_EXPORT qint64 KAsyncPostRequest2(const QString& connection_name, 
        const QString& service_name, const QByteArray& data);

    /**
    *	对象销毁的时候应该调用这个函数
    **/
    KSYNCREQUEST_EXPORT void KUnregisterRequestForObject(QObject* obj);

    /**
    *	添加一个连接
    *
    *	@param
    *		-[in]
    *          connection_name     连接名称不能重复。不区分大小写
    *          addr_port           地址和端口，
    *                              如：inproc://this_addr
    *                              .... tcp://1.2.3.4:555
    *          listen              true 绑定到本地， false 连接到远程
    **/
    KSYNCREQUEST_EXPORT bool KAsyncAddConnection(const QString&connection_name,
        const char* addr_port, bool listen);

    /**
     *	根据消息头，进行订阅	
     *
     *      json_msg["head"] == head_no 的消息将会被发送到这个订阅者
     *		
     *	@param
     *		-[in]
     *          head_no     包头号：
     *          obj
     *          slot
    **/
    KSYNCREQUEST_EXPORT void KSubscribeHead(int head_no,
        QObject* obj, const char* slot);

    /**
     *  设置ip，在发消息之前进行调用。
     *
     *	@param
     *		-[in]
     *          ip_and_port     ip 和 端口，例如：
     *                          tcp://192.168.205.202:5555
     *
    **/
    KSYNCREQUEST_EXPORT void KSetServerIP(const QString& ip_and_port);
    
    KSYNCREQUEST_EXPORT void KShutdownRequest();
}

#endif // KSYNCREQUEST_H
