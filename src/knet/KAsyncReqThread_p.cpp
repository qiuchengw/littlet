#include "KAsyncReqThread_p.h"
#include "include/kconfig.h"
#include "include/base.h"
#include "awaker.h"

#define AWAKE_CONNECTION_SERVERNAME "__AWAKE_SOCKET_SERVER"
#define AWAKE_CONNECTION_ADDR "inproc://awakeconnection"

#define AWAKE_REASON_POSTMSG "postmsg"
#define AWAKE_REASON_ADDCONNECTION "newconn"
#define AWAKE_REASON_SHUTDOWN "shutdown"
#define AWAKE_REASON_ADDHEADSUBSCRIBER "new_head_subscriber"
#define AWAKE_REASON_UNREGISTEROBJECT "object_unreg"

//////////////////////////////////////////////////////////////////////////
KAsyncReqThreadPrivate::KAsyncReqThreadPrivate()
{
    
}

KAsyncReqThreadPrivate::~KAsyncReqThreadPrivate()
{
//    Q_ASSERT(nullptr == ctx_);
}

qint64 KAsyncReqThreadPrivate::PostRequest(QObject* sender_obj,
    const char* slot,
    const QString& connection_name, 
    const QString& service_name, 
    const QByteArray& data)
{
    static const QByteArray NULL_BYTEARRAY;

    _SendingItem* p = new _SendingItem;
    p->connection_name = connection_name;
    p->sender_ = sender_obj;

#ifdef _DEBUG
    p->service_name = service_name;
#endif

    // 共4帧消息
    // 1, 0byte 空串
    // 2, 服务名
    // 3, 发送者标识符
    // 4, 消息数据

    // 1,
    p->data.push_back(NULL_BYTEARRAY);
    // 2, 
    p->data.push_back(service_name.toUtf8());
    // 3, 申请序号
    QByteArray send_id;
    queue_locker_.lock();
    qint64 send_no = recvs_.AddReciver(sender_obj, slot, send_id);
    queue_locker_.unlock();

#ifdef _DEBUG
    p->no_ = send_no;
#endif

    p->data.push_back(send_id);

    // 以下代码在本地测试是可以的，能够写进去，还能够读出来。
    // 但是发送到远程并返回后，读取数据就是不准确的。怀疑远程是作为字符串读取并填入回来了。
    // 暂时的低效的解决办法是：格式化为字符串。返回后再转换为qint64
    //    p->data.push_back(QByteArray((const char*)&send_no, sizeof(qint64)));

    qDebug() << QStringLiteral("异步发送请求，服务")<<service_name << QStringLiteral("序号：")<<send_no;

    // 4, data
    p->data.push_back(data);

    queue_locker_.lock();
    msgs_.enqueue(p);
    queue_locker_.unlock();


    // 发送一个异步激活消息
    // 此socket可以被非创建线程直接访问，因为加锁了。
    AwakeZmq(AWAKE_REASON_POSTMSG);

    return send_no;
}

qint64 KAsyncReqThreadPrivate::PostRequest(
    const QString& connection_name,
    const QString& service_name,
    const QByteArray& data)
{
    return PostRequest(nullptr, nullptr, connection_name, service_name, data);
}

bool KAsyncReqThreadPrivate::run()
{
    // qRegisterMetaType<ReciveMessage>("ReciveMessageType");

    // 唤醒循环的socket，负责接收唤醒消息，使用inproc协议
    nzmqt::KMySocket* sock_awake_server = nullptr;

    ctx_ = new nzmqt::KMyZmqContext();

    // 创建唤醒监听连接，使用inproc协议
    sock_awake_server = AddConnection_p(AWAKE_CONNECTION_SERVERNAME,
        nzmqt::ZMQSocket::TYP_REP, AWAKE_CONNECTION_ADDR, true);

    if (nullptr == sock_awake_server)
    {
        Q_ASSERT(false);
        qCritical() << QStringLiteral("创建唤醒socket失败！");

        // throw nzmqt::ZMQException();
        return false;
    }

    // 创建唤醒器
    awaker_ = new AWaker(ctx_, AWAKE_CONNECTION_ADDR);

// #ifdef _DEBUG
//     // 用于测试的连接
//     AddConnection_p("test_conn", nzmqt::ZMQSocket::TYP_DEALER, "tcp://127.0.0.1:5555", false);
// #endif

    qDebug() << QStringLiteral("进入异步消息循环。");

    while (!stoped_)
    {
        try
        {
            qDebug() << QStringLiteral("进入zmq epoll循环，正在监听发送请求....");
            Q_ASSERT(nullptr != ctx_);
            if (ctx_->poll(&recvs_, sock_awake_server))
            {
                // 激活消息，使用同步模式收取
                QList<QByteArray> m = sock_awake_server->receiveMessage();

                // 接收到消息后，一定要回复一个消息！
                // FUCK!!! 不是说好的，可以一直发送/一直接收的吗？？？？
                sock_awake_server->sendMessage("");

                // 解析消息
                if (m.size() != 2)
                {
                    Q_ASSERT(false);
                    continue;
                }

                if (QString(m[0]).compare(AWAKE_REASON_SHUTDOWN) == 0)
                {
                    // 退出吧
                    break;
                }
                else if (QString(m[0]).compare(AWAKE_REASON_POSTMSG) == 0)
                {
                    // 处理发送队列 
                    HandleSendingQueue();
                }
                else if(QString(m[0]).compare(AWAKE_REASON_ADDCONNECTION) == 0)
                {
                    // 添加连接
                    RJsonDoc cfg(m[1], false);
                    if (!cfg.HasParseError())
                    {
                        AddConnection_p(cfg["name"].GetString(),
                            (nzmqt::ZMQSocket::Type)cfg["type"].GetInt(),
                            cfg["addr"].GetString(),
                            cfg["listen"].GetBool());
                    }
                }
                else if (QString(m[0]).compare(AWAKE_REASON_ADDHEADSUBSCRIBER) == 0)
                {
                    // 添加连接
                    RJsonDoc cfg(m[1], false);
                    if (!cfg.HasParseError())
                    {
                        recvs_.SubscribeHead(
#ifdef _WIN64
                            (QObject*)cfg["object"].GetUint64(),
#else
                            (QObject*)cfg["object"].GetInt(),
#endif
                            cfg["slot"].GetString(),
                            cfg["head"].GetInt());
                    }
                }
                else if (QString(m[0]).compare(AWAKE_REASON_UNREGISTEROBJECT) == 0)
                {
                    RJsonDoc cfg(m[1], false);
                    if (!cfg.HasParseError())
                    {
                        recvs_.UnregisterObject(
#ifdef _WIN64
                            (QObject*)cfg["object"].GetUint64()
#else
                            (QObject*)cfg["object"].GetInt()
#endif
                            );
                    }
                }
            }
            qDebug() << QStringLiteral("退出zmq epoll循环....");
        }
        catch (const nzmqt::ZMQException& ex)
        {
            qWarning("Exception during poll: %s", ex.what());
        }
    }


    // 首先唤醒器删除
    delete awaker_;
    awaker_ = nullptr;

    // delete
    ctx_->stop();

    for (nzmqt::KMySocket* s : sockets_.values())
    {
        s->close();

        delete s;
    }

    delete ctx_;
    ctx_ = nullptr;

    return false;
}

void KAsyncReqThreadPrivate::HandleSendingQueue()
{
    if (!queue_locker_.tryLock())
    {
        return;
    }

    ON_SCOPE_EXIT([=](){
        queue_locker_.unlock();
    });

    // 一次性发送队列中所有的数据
    while (!stoped_ && !msgs_.isEmpty())
    {
        _SendingItem* p = msgs_.dequeue();
        auto* sock = FindSocket(p->connection_name);
        if (nullptr == sock)
        {
            qCritical() << QStringLiteral("没有找到连接名！");
        }
        else
        {
#ifdef _DEBUG
            qDebug() << QStringLiteral("异步发送：")
                << QStringLiteral("服务名：") << p->service_name
                << QStringLiteral("序号：") << p->no_
                << QStringLiteral("数据：") << p->data
                << QStringLiteral("请求被处理！");
            qDebug() << "---> Still has:" << msgs_.size();
#endif // DEBUG

            // 非阻塞发送
            sock->sendMessage(p->data, nzmqt::ZMQSocket::SND_NOBLOCK);
        }
    }
}

void KAsyncReqThreadPrivate::stop()
{
    stoped_ = true;

    if (nullptr != awaker_)
    {
        // 同步的阻塞通信
        AwakeZmq(AWAKE_REASON_SHUTDOWN, "1" , false);
    }
}

nzmqt::KMySocket* KAsyncReqThreadPrivate::AddConnection_p(const QString&connection_name, 
    nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen)
{
    auto *sock = FindSocket(connection_name);
    if (nullptr != sock)
    {
        // Q_ASSERT(false);
        qWarning() << QStringLiteral("连接名已注册！");
        // return nullptr;
        // 把原来的删除掉
        // FIXME: socket需要手动delete吗？
        sock->close();
    }

    sock = (nzmqt::KMySocket*)ctx_->createSocket(typ);
    Q_ASSERT(nullptr != sock);
    if (nullptr != sock)
    {
        if (listen)
        {
            sock->bindTo(addr_port);
        }
        else
        {
            sock->connectTo(addr_port);
        }

        sockets_.insert(connection_name.toLower(), sock);
    }

    return sock;
}

void KAsyncReqThreadPrivate::AddConnection(const QString&connection_name,
    nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen)
{
    KConfig cfg;
    cfg.AddMember("name", connection_name);
    cfg.AddMember("type", typ);
    cfg.AddMember("addr", addr_port);
    cfg.AddMember("listen", listen);

    AwakeZmq(AWAKE_REASON_ADDCONNECTION, cfg.GetConfigString().toUtf8());
}

void KAsyncReqThreadPrivate::AwakeZmq(const char* reason, QByteArray data, bool use_async)
{
    QList<QByteArray> msg;
    // DEALER 和 DEALER 连接的时候，第一帧为必须为空帧，
    // 否则poll可以被唤醒，但是并不能接收到消息
    // 接收消息返回false，错误值是EAGAIN
    msg.push_back("");  
    msg.push_back(reason);
    msg.push_back(data);

    if (nullptr != awaker_)
    {
        awaker_->AwakeMessage(msg, use_async);
    }
}

void KAsyncReqThreadPrivate::UnregisterRequestForObject(QObject* obj)
{
    KConfig cfg;
#ifdef _WIN64
    cfg.AddMember("object", (qulonglong)obj);
#else
    cfg.AddMember("object", (long)obj);
#endif

    AwakeZmq(AWAKE_REASON_UNREGISTEROBJECT, cfg.GetConfigString().toUtf8());
}

void KAsyncReqThreadPrivate::SubscribeHead(int head_no, QObject* obj, const char* slot)
{
    KConfig cfg;
    cfg.AddMember("head", head_no);
#ifdef _WIN64
    cfg.AddMember("object", (qulonglong)obj);
#else
    cfg.AddMember("object", (long)obj);
#endif
    cfg.AddMember("slot", slot);

    AwakeZmq(AWAKE_REASON_ADDHEADSUBSCRIBER, cfg.GetConfigString().toUtf8());
}
