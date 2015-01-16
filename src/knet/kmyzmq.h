#pragma once

#include "deps/nzmqt/nzmqt.hpp"

class KRecvQueue;
class AWaker;

namespace nzmqt
{

    class KMySocket : public ZMQSocket
    {
        friend class KMyZmqContext;
        friend class AWaker;

    protected:
        KMySocket(KMyZmqContext * context_, Type type_);

        void onMessageReceived(const QList<QByteArray>& message);
    };

    class KMyZmqContext : public ZMQContext//, public QRunnable
    {
        Q_OBJECT

            typedef ZMQContext super;

    public:
        KMyZmqContext(QObject* parent_ = 0, int io_threads_ = NZMQT_DEFAULT_IOTHREADS);

        // Starts the polling process by scheduling a call to the 'run()' method into Qt's event loop.
        void start();

        // Stops the polling process in the sense that no further 'run()' calls will be scheduled into
        // Qt's event loop.
        void stop();

        bool isStopped() const;

        bool poll(KRecvQueue* recv_queue, KMySocket* sock_awake);

    protected:
        KMySocket* createSocketInternal(ZMQSocket::Type type_);

        // Add the given socket to list list of poll-items.
        void registerSocket(ZMQSocket* socket_);

        // Remove the given socket object from the list of poll-items.
        void unregisterSocket(ZMQSocket* socket_);

    private:
        typedef QVector<pollitem_t> PollItems;

        PollItems m_pollItems;
        QMutex m_pollItemsMutex;
    };

}
