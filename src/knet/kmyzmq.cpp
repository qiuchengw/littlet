#include "kmyzmq.h"
#include "KRecvQueue.h"

class AWaker;

namespace nzmqt
{
    KMySocket::KMySocket(KMyZmqContext* context_, Type type_)
        : ZMQSocket(context_, type_)
    {
    }

    /*
    * KMyPollingContext
    */

    KMyZmqContext::KMyZmqContext(QObject* parent_, int io_threads_)
        : super(parent_, io_threads_)
        , m_pollItemsMutex(QMutex::Recursive)
    {
    }

    bool KMyZmqContext::poll(KRecvQueue* recv_queue, KMySocket* sock_awake)
    {
        QMutexLocker lock(&m_pollItemsMutex);

        if (m_pollItems.empty())
            return false;

        int cnt = zmq::poll(&m_pollItems[0], m_pollItems.size(), 3600*1000);
        Q_ASSERT_X(cnt >= 0, Q_FUNC_INFO, "A value < 0 should be reflected by an exception.");
        if (0 == cnt)
            return false;

        bool is_awake_socket = false;

        PollItems::iterator poIt = m_pollItems.begin();
        ZMQContext::Sockets::const_iterator soIt = registeredSockets().begin();
        int i = 0;
        while (i < cnt && poIt != m_pollItems.end())
        {
            if (poIt->revents & ZMQSocket::EVT_POLLIN)
            {
                KMySocket* socket = static_cast<KMySocket*>(*soIt);
                if (socket == sock_awake)
                {
                    // 唤醒消息
                    is_awake_socket = true;

                }
                else
                {
                    // 非唤醒消息，放入到消息接收队列，处理之！
                    recv_queue->MessageRecevied(socket->receiveMessage());
                }
                i++;
            }
            ++soIt;
            ++poIt;
        }
        return is_awake_socket;
    }

    KMySocket* KMyZmqContext::createSocketInternal(ZMQSocket::Type type_)
    {
        return new KMySocket(this, type_);
    }

    void KMyZmqContext::registerSocket(ZMQSocket* socket_)
    {
        pollitem_t pollItem = { (void*)(*socket_), 0, ZMQSocket::EVT_POLLIN, 0 };

        QMutexLocker lock(&m_pollItemsMutex);

        m_pollItems.push_back(pollItem);

        super::registerSocket(socket_);
    }

    void KMyZmqContext::unregisterSocket(ZMQSocket* socket_)
    {
        QMutexLocker lock(&m_pollItemsMutex);

        PollItems::iterator poIt = m_pollItems.begin();
        ZMQContext::Sockets::const_iterator soIt = registeredSockets().begin();
        while (soIt != registeredSockets().end())
        {
            if (*soIt == socket_)
            {
                m_pollItems.erase(poIt);
                break;
            }
            ++soIt;
            ++poIt;
        }

        super::unregisterSocket(socket_);
    }

    void KMyZmqContext::start()
    {

    }

    void KMyZmqContext::stop()
    {

    }

    bool KMyZmqContext::isStopped() const
    {
        return true;
    }

}