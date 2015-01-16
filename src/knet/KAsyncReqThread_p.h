#ifndef KAsyncReqThread_p_h__
#define KAsyncReqThread_p_h__

#include <QHash>
#include <QByteArray>
#include <QQueue>

#include "deps/qxtglobal.h"
#include "KAsyncReqThread.h"
#include "KRecvQueue.h"
#include "kmyzmq.h"

class AWaker;

/**
 *	
 *  异步收发线程需要管理的东西有：
 *  
 *      1，socket连接管理器
 *      2，数据发送队列
 *          这个是需要的，所有的socket都位于KAsyncReqThread线程内
 *          发送消息的函数的调用可能是在任何一个线程，而zmq不建议跨线程直接访问socket，
 *          所以我们将所有的socket调用都限定在KAsyncReqThread内。
 *      //----------------
 *      zmq_send is given a socket to send on as one of its arguments. 
 *      The thread that calls zmq_send must be the same thread that 
 *      created the socket by calling zmq_socket.
 *      // ----------
 *       You MUST NOT share ØMQ sockets between threads. ØMQ sockets are not threadsafe.
 *       Technically it's possible to do this, but it ……
 *      //----------------
 *      3，数据接收队列
 *
**/

// 消息收发器
class KAsyncReqThreadPrivate :
    public QxtPrivate < KAsyncReqThread >
{
    QXT_DECLARE_PUBLIC(KAsyncReqThread);

    // 待发送消息
    struct _SendingItem
    {
        QObject*    sender_;
        QString     connection_name;
        QList<QByteArray>  data;
#ifdef _DEBUG
        QString     service_name;
        qint64      no_;
#endif
    };

    struct _ConnectItem 
    {
        // 连接名字
        QString     connection_name;

        // 类型
        nzmqt::ZMQSocket::Type typ;

        // 地址
        QString     addr_port;

        // 是否监听
        bool        listen;
    };

public:
    KAsyncReqThreadPrivate();

    ~KAsyncReqThreadPrivate();

    /**
     *	@return
     *      0       发送失败
     *      > 0     发送序号，需要保存此序号。
     *              消息接收后，通知接收者时会以此序号区别是那次请求
     *
    **/
    qint64 PostRequest(QObject* sender_obj, const char* slot, 
        const QString& connection_name, const QString& service_name, 
        const QByteArray& data);
    qint64 PostRequest(const QString& connection_name, 
        const QString& service_name, const QByteArray& data);
    
    // 循环监听
    bool run();

    // 停止监听
    void stop();

    void AddConnection(const QString&connection_name,
        nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen);

    void SubscribeHead(int head_no, QObject* obj, const char* slot);

    void UnregisterRequestForObject(QObject* obj);

protected:
    /**
     *	添加一个连接	
     *	
     *	@param
     *		-[in]
     *          connection_name     连接名称不能重复。不区分大小写
     *          typ                 连接类型
     *          addr_port           地址和端口，
     *                              如：inproc://this_addr 
     *                              .... tcp://1.2.3.4:555
     *          listen              true 绑定到本地， false 连接到远程
    **/
    nzmqt::KMySocket* AddConnection_p(const QString&connection_name,
        nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen);

    void AwakeZmq(const char* reason, QByteArray data = QByteArray(), bool use_async = true);

protected:
    /**
     *	查找连接socket，	
     *
     *	@param
     *		-[in]
     *        connection_name   连接名，不区分大小写  
    **/
    inline nzmqt::KMySocket* FindSocket(const QString& connection_name)
    {
        return sockets_.value(connection_name.toLower());
    }

    void HandleSendingQueue();
   
private:
    // 1，接收数据的队列
    KRecvQueue      recvs_;

    // 2，socket连接管理器
    QHash<QString, nzmqt::KMySocket*>  sockets_;

    // zmq context
    //  ！！！不能调用此对象的run方法！
    nzmqt::KMyZmqContext*  ctx_ = nullptr;

    // 复杂发送唤醒消息
//    nzmqt::KMySocket *sock_awake_client_ = nullptr;

    // 停止标志
    bool stoped_ = false;

    // 唤醒自己的对象
    // KAsyncReqAwaker*        awaker_ = nullptr;

    // 等待发送的消息队列
    QQueue<_SendingItem*>    msgs_;

    // 队列锁
    QMutex      queue_locker_;

    // 唤醒器
    AWaker*      awaker_;
};


#endif // KAsyncReqThread_p_h__
