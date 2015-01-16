#include "KRecvQueue.h"
#include <QDebug>
#include "deps/rjson/inc.h"

ReciverSignaler::ReciverSignaler(QObject* sender_obj, const char* sloter)
    :QObject(nullptr), obj_(sender_obj), slot_(sloter)
{
    // 当数据收到的时候会激发信号
    sender_obj->connect(this, SIGNAL(signal_MessageRecived(qint64, QByteArray)),
        sloter, Qt::QueuedConnection);
}

HeadSubscriber::HeadSubscriber(QObject* sender_obj, const char* sloter)
    :QObject(nullptr), obj_(sender_obj), slot_(sloter)
{
    // 当数据收到的时候会激发信号
    sender_obj->connect(this, SIGNAL(signal_MessageRecived(int, QByteArray)),
        sloter, Qt::QueuedConnection);
}

//////////////////////////////////////////////////////////////////////////
KRecvQueue::KRecvQueue()
{
}

KRecvQueue::~KRecvQueue()
{
    // 回收内存
    for (auto *p : recvs_)
    {
        delete p;
    }

    for (auto i = head_subs_.begin(); i != head_subs_.end(); ++i)
    {
        delete i.value();
    }
}

qint64 KRecvQueue::AddReciver(QObject* obj, const char* slot, QByteArray& send_id)
{
    // 每一个发送请求都唯一被编号
    static qint64 _send_no = 0;

    // 发送序号自动累计
    ++_send_no;

    if ((nullptr != obj) && (nullptr != slot))
    {
        ReciverSignaler* reciver = RegisterReciver(obj, slot);

#ifdef _WIN64
        send_id = QString("%1;%2").arg((qlonglong)reciver).arg(_send_no).toUtf8();
#else
        send_id = QString("%1;%2").arg((long)reciver).arg(_send_no).toUtf8();
#endif
    }
    else
    {
        send_id = QString("0;%1").arg(_send_no).toUtf8();
    }

    return _send_no;
}

bool KRecvQueue::MessageRecevied(const QList<QByteArray>& message)
{
    // 共3帧消息，如果不是，则丢弃
    if (message.size() != 4)
        return false;

#ifdef _DEBUG
    qDebug() << QStringLiteral("异步数据接收队列收到数据：");
    qDebug() << QStringLiteral("------------------");
    for (auto i = message.begin(); i != message.end(); ++i)
    {
        qDebug() << *i;
    }
    qDebug() << QStringLiteral("------------------");
#endif

    // 解包！
    // 0, 空帧，无用，丢弃

    // 1 , ReciverSignaler指针 ; 发送序号
    QStringList lst = QString(message[1]).split(';');
    if (lst.size() != 2)
    {
        Q_ASSERT(false);
        return false;
    }

    // 1, recvier
#ifdef _WIN64
    ReciverSignaler* reciver_signaler = (ReciverSignaler*)(lst[0].toLongLong());
#else
    ReciverSignaler* reciver_signaler = (ReciverSignaler*)(lst[0].toLong());
#endif

    // ！！！不能删掉这个接收者
    //      现在的服务器会主动推送一些数据
    // recvs_.takeAt(recvs_.indexOf(p));

    // 2, 服务名
    // ....，无用，丢弃

    // qDebug() << QStringLiteral("-------->处理接收到的数据，还有：") << items_.size();
    
    // 3, 返回的消息数据
    // 2014/9/3
    //      消息的接收者不再是请求的发送者
    //      此处根据包头进行再分发

    // 解包数据
    //////////////////////////////////////////////////////////////////////////
    // -- 协议设计有问题
    RJsonDoc doc(message[3], false);
    bool bhandled = false;
    if (!doc.HasParseError() && doc.HasMember("head"))
    {
        // 根据包头进行分发数据
        int head = doc["head"].GetInt();
        for (HeadSubscriber* h : head_subs_.values(head))
        {
            Q_ASSERT(nullptr != h);
            if (nullptr != h)
            {
                bhandled = true;
                emit h->signal_MessageRecived(head, message[3]);
            }
        }
    }

    if (!bhandled && reciver_signaler)
    {
        emit reciver_signaler->signal_MessageRecived(lst[1].toLongLong(), message[3]);
    }

    return true;
}

ReciverSignaler* KRecvQueue::RegisterReciver(QObject *obj, const char* slot)
{
    for (ReciverSignaler* r : recvs_)
    {
        if ((r->object() == obj) && (r->sloter().compare(slot) == 0))
        {
            return r;
        }
    }

    ReciverSignaler* r = new ReciverSignaler(obj, slot);
    recvs_.push_back(r);
    return r;
}

bool KRecvQueue::UnregisterObject(QObject* obj)
{
    for (auto i = head_subs_.begin(); i != head_subs_.end();)
    {
        auto *p = i.value();
        if (p->object() == obj)
        {
            delete p;
            i = head_subs_.erase(i);
        }
        else
        {
            ++i;
        }
    }

    for (auto i = recvs_.begin(); i != recvs_.end();)
    {
        auto *p = *i;
        if (p->object() == obj)
        {
            i = recvs_.erase(i);
            delete p;
        }
        else
        {
            ++i;
        }
    }
    return true;
}

bool KRecvQueue::SubscribeHead(QObject* obj, const char* slot, int head_no)
{
    HeadSubscriber* p = head_subs_.value(head_no);
    if (nullptr != p)
    {
        qWarning() << QStringLiteral("Object已经预定消息号：")<< head_no;

        if (p->object() == obj  && (p->sloter().compare(slot) == 0))
        {
            Q_ASSERT(false);
            return false;
        }

        // return false;
    }

    head_subs_.insert(head_no, new HeadSubscriber(obj, slot));

    return true;
}
