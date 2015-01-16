#pragma once

#include <QObject>
#include <QHash>
#include <QMultiHash>
#include <QList>

typedef QList<QByteArray> ReciveMessage;

// 数据接收者
class ReciverSignaler : public QObject
{
    Q_OBJECT;

public:
    ReciverSignaler(QObject* sender_obj, const char* slot);

    QObject *object()
    {
        return obj_;
    }

    QString sloter()
    {
        return slot_;
    }

signals:
    void signal_MessageRecived(qint64 send_no, QByteArray data);

private:
    // 发送者对象
    QObject*    obj_ = nullptr;

    // 信号槽
    QString     slot_;
};

// 某一个包类型的数据接收者
class HeadSubscriber : public QObject
{
    Q_OBJECT;

public:
    HeadSubscriber(QObject* sender_obj, const char* slot);

    QObject *object()
    {
        return obj_;
    }

    QString sloter()
    {
        return slot_;
    }

signals:
    void signal_MessageRecived(int head, QByteArray data);

private:
    // 发送者对象
    QObject*    obj_ = nullptr;

    // 信号槽
    QString     slot_;
};

/**
 *	异步zmq的数据接收队列	
 *
**/
class KRecvQueue : public QObject
{
    Q_OBJECT;

public:
    KRecvQueue();
    ~KRecvQueue();

    /**
     *	添加一个收信者	
     *
     *	@param
     *		-[in]
     *          obj     收信者
     *          slot    收消息时激活的slot
     *      -[out]
     *          send_id 这个字段一定要写入到发送数据中
    **/
    qint64 AddReciver(QObject* obj, const char* slot, QByteArray& send_id);

    // 某一类包头的接收者
    // 当接收数据中的 json ["head"] == head_no时，发送到槽上
    bool SubscribeHead(QObject* obj, const char* slot, int head_no);

    /**
     *	消息收到！	
     *
     *	@return
     *		true    消息被确认
     *
    **/
    bool MessageRecevied(const QList<QByteArray>& data);

    bool UnregisterObject(QObject* obj);

protected:
    /**
     *	如果已经注册，返回nullptr	
     *
    **/
    ReciverSignaler* RegisterReciver(QObject *obj, const char* slot);

private:
    // 某一类head的接收者
    // 一个消息号可以被多个接收者预定
    QMultiHash<int, HeadSubscriber*>  head_subs_;

    // 已经注册的接收者
    //QMultiHash<QObject*, QString> recvs_;
    QList<ReciverSignaler*> recvs_;
};

