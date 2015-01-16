#ifndef awaker_h__
#define awaker_h__

#include <QHash>
#include <QMutex>
#include "deps/nzmqt/nzmqt.hpp"

class AWaker
{
    Q_DISABLE_COPY(AWaker);

    friend class KAsyncReqThreadPrivate;

private:
    AWaker(nzmqt::KMyZmqContext* ctx, const char* awake_addr)
        :ctx_(ctx), awake_addr_(awake_addr)
    {
        // 1个线程足够了
        // ctx_ = ctx; new nzmqt::PollingZMQContext(nullptr, 1);
        Q_ASSERT(nullptr != ctx);
        Q_ASSERT(nullptr != awake_addr);
    }

    ~AWaker()
    {
        QMutexLocker locker(&event_locker_);

        for (auto *p : awake_socks_.values())
        {
            delete p;
        }
        // delete ctx_;
    }

    /**
     *	向被唤醒者发送消息	
     *
     *	@return
     *		true    发送唤醒成功。
     *	@param
     *		-[in]
     *      msg     唤醒消息
     *      use_async   false ，会使用同步模式发送唤醒消息
    **/
    bool AwakeMessage(QList<QByteArray>& msg, bool use_async)
    {
        event_locker_.lock();
        nzmqt::ZMQSocket* sock = socket(QThread::currentThreadId());
        event_locker_.unlock();

        Q_ASSERT(nullptr != sock);
        if (nullptr != sock)
        {
            return  sock->sendMessage(msg,
                (nzmqt::ZMQSocket::SendFlags)(use_async ? nzmqt::ZMQSocket::SND_NOBLOCK : 0));
        }
        return false;
    }

private:
    // 如果当前没有则创建
    nzmqt::ZMQSocket* socket(Qt::HANDLE thread_id)
    {
        auto * p = awake_socks_.value(thread_id);
        if (nullptr == p)
        {
            // 不能使如下方式创建，因为内部逻辑是：
            //      1,创建socket的
            //      2,注册socket  （在注册的时候加锁了，可能会造成死锁！）
            // p = ctx_->createSocket(nzmqt::ZMQSocket::TYP_DEALER, nullptr);

            p = new nzmqt::KMySocket(ctx_, nzmqt::ZMQSocket::TYP_DEALER);

            // 保存
            awake_socks_.insert(thread_id, p);

            // 连接上去
            p->connectTo(awake_addr_);
        }
        return p;
    }

private:
    // 每个线程有且仅有一个唤醒socket
    QHash<Qt::HANDLE, nzmqt::ZMQSocket*> awake_socks_;

    // 所有的唤醒socket都在这个context上创建
    nzmqt::KMyZmqContext* ctx_ = nullptr;

    // 需要被唤醒的地址
    QString     awake_addr_;

    // 事件锁
    QMutex      event_locker_;
};

#endif // awaker_h__
