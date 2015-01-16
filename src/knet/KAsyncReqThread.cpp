#include "KAsyncReqThread.h"
#include "KAsyncReqThread_p.h"

KAsyncReqThread::KAsyncReqThread()
{
    QXT_INIT_PRIVATE();
}

KAsyncReqThread::~KAsyncReqThread()
{
}

qint64 KAsyncReqThread::PostRequest(QObject* sender_obj, 
    const char* slot,
    const QString& connection_name, 
    const QString& service_name, 
    const QByteArray& data)
{
    QXT_D;

// #if defined _DEBUG //&& defined _TEST_QUI
//     return d.PostRequest(sender_obj, slot, "test_conn", "Login", data);
// #endif

    return d.PostRequest(sender_obj, slot, connection_name, service_name, data);
}

void KAsyncReqThread::run()
{
    QXT_D;

    d.run();
}

void KAsyncReqThread::Shutdown()
{
    QXT_D;

    d.stop();
}

void KAsyncReqThread::AddConnection(const QString&connection_name, 
    nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen)
{
    QXT_D;

    d.AddConnection(connection_name, typ, addr_port, listen);
}

void KAsyncReqThread::SubscribeHead(int head_no, QObject* obj, const char* slot)
{
    QXT_D;

    d.SubscribeHead(head_no, obj, slot);
}

void KAsyncReqThread::UnregisterRequestForObject(QObject* obj)
{
    QXT_D;

    d.UnregisterRequestForObject(obj);
}
