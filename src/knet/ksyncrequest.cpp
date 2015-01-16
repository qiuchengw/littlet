#include "ksyncrequest.h"
#include "ksyncrequestman.h"
#include "KAsyncReqThread.h"

extern "C"
{
    static SyncReq _sync_recv;

    bool KSyncSendRequest(const QString& name, const QList<QString>& data,
        QList<QString>& out_data)
    {
        _sync_recv.conn();

        return _sync_recv.SendRequest(name, data, out_data);
    }

    bool KSyncSendRequestA(const QString& name, const QString& data,
        QList<QString>& out_data)
    {
        QList<QString> lst;
        lst.append(data);

        return KSyncSendRequest(name, lst, out_data);
    }

    void KSetServerIP(const QString& ip_and_port)
    {
        _sync_recv.SetServer(ip_and_port);
    }


    KAsyncReqThread* GetAsyncPostThread()
    {
        static KAsyncReqThread _async_req;

        return &_async_req;
    }

    void KShutdownRequest()
    {
        _sync_recv.Shutdown();

        GetAsyncPostThread()->Shutdown();
    }

    qint64 KAsyncPostRequest(QObject* obj, const char* slot, 
        const QString& connection_name,
        const QString& service_name, const QByteArray& data)
    {
        Q_ASSERT(nullptr != obj);
        Q_ASSERT(nullptr != slot);
        Q_ASSERT(!connection_name.isEmpty());
        Q_ASSERT(!service_name.isEmpty());

        return GetAsyncPostThread()->PostRequest(obj, slot, 
            connection_name, service_name, data);
    }

    qint64 KAsyncPostRequest2(const QString& connection_name,
        const QString& service_name, const QByteArray& data)
    {
        Q_ASSERT(!connection_name.isEmpty());
        Q_ASSERT(!service_name.isEmpty());

        return GetAsyncPostThread()->PostRequest(nullptr, nullptr,
            connection_name, service_name, data);
    }



    bool KAsyncAddConnection(const QString&connection_name, const char* addr_port, bool listen)
    {
        GetAsyncPostThread()->AddConnection(connection_name,
            nzmqt::ZMQSocket::TYP_DEALER, addr_port, listen);

        return true;
    }

    void KSubscribeHead(int head_no, QObject* obj, const char* slot)
    {
        GetAsyncPostThread()->SubscribeHead(head_no, obj, slot);
    }

    void KUnregisterRequestForObject(QObject* obj)
    {
        GetAsyncPostThread()->UnregisterRequestForObject(obj);
    }

    bool KStartNetRequestService()
    {
        static bool _inited = false;
        Q_ASSERT(!_inited);

        auto *p = GetAsyncPostThread();
        if (!_inited)
        {
            p->start();
            _inited = true;
        }
        return _inited;
    }

}



