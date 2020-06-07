#include "server.h"
#include <process.h>
#include <iostream>

#include "stdstring.h"
#include "db.h"

#define WT_NUM 10


//////////////////////////////////////////////////////////////////////////

class CUserFeedWorkItem : public WorkItemBase
{
public:
    CUserFeedWorkItem(user_feed_param* p)
    {
        run_ = false;
        sock_ = nullptr;
        param_ = p;
    }

    ~CUserFeedWorkItem()
    {
        if (nullptr != sock_)
        {
            delete sock_;
        }
    }

    virtual void   DoWork(void* pThreadContext);
    
    virtual void   Abort ();

    /**
     *	如果消息被处理的话，从队列中删除	
     *
     *	@return
     *	    还没有被处理的消息个数	
     *
    **/
    int scan_usefeed();

    const Msgs* msgs()const
    {
        return &msgs_;
    }

private:
    Msgs msgs_;
    bool run_;
    zmq::socket_t*  sock_;
    user_feed_param* param_;   
};

typedef std::vector<CUserFeedWorkItem*> Workers;

//////////////////////////////////////////////////////////////////////////

CMsgServer::CMsgServer(void)
{
}

CMsgServer::~CMsgServer(void)
{
}

int CMsgServer::Startup()
{
    // 启动10个工作者线程
    zmq::context_t ctx;
    user_feed_param ufp;
    ufp.ctx = &ctx;
    ufp.ip_port = "inproc://*:5230";

    // 工作者线程
    CWorkQueue  work_queue;
    work_queue.Create(WT_NUM);

    // 直接全部创建吧！
    Workers workers;
    for (int i = 0; i < WT_NUM; ++i)
    {
        CUserFeedWorkItem* a_worker = new CUserFeedWorkItem(&ufp);
        if (work_queue.InsertWorkItem(a_worker))
        {
            workers.push_back(a_worker);
        }
        else
        {
            delete a_worker;
        }
    }

    // 使用dealer接收客户端消息
    zmq::socket_t front_end(ctx, ZMQ_ROUTER);
    front_end.bind("tcp://*:5231");  // 本地的端口
    
    zmq::socket_t back_end(ctx, ZMQ_DEALER);
    back_end.bind(ufp.ip_port.c_str());  // 本地的端口

    // 处理数据的线程
    HANDLE h_feed = (HANDLE)_beginthreadex(nullptr, 0, 
        &CMsgServer::thread_handlemsg, &workers, 0, nullptr);
    CloseHandle(h_feed);

    // 持续运行
    zmq::proxy(front_end, back_end, nullptr);

    return 0;
}

CStdString GetModulePath()
{
    TCHAR path[1024];
    int nSize = ::GetModuleFileName(NULL,path,1024);
    path[nSize] = _T('\0');
    CStdString sRet(path);
    sRet.Replace(_T('\\'),_T('/'));
    int idx = sRet.ReverseFind(_T('/'));
    sRet = sRet.Left(idx+1);
    return sRet;
}

UINT_PTR __stdcall CMsgServer::thread_handlemsg( void* param )
{
    // 启动数据库
    if (!db::GetInstance()->Open(GetModulePath() + L"littlet.db"))
    {
        return -1;
    }

    Workers* ws = reinterpret_cast<Workers*>(param);
    // 等待消息
    while (true)
    {
        // 扫描所有工作者线程中收到的数据
        for (auto i_worker = ws->begin(); i_worker != ws->end(); ++i_worker)
        {
            // 处理
            const Msgs* ms = (*i_worker)->msgs();
            for (auto i_msg = ms->begin(); i_msg != ms->end(); ++i_msg)
            {
                const UserMsg *a_msg = *i_msg;
                if (a_msg->handled)
                    continue;   // 已经处理过了

                // 没处理的
                const OneMsg& m = a_msg->msg;
                // 共3帧：
                //  1,服务名
                //  2,MAC 地址
                //  3,内容
                if (m.size() < 3)
                {
                     const_cast<UserMsg*>(a_msg)->handled = true;
                     continue;
                }

                // 全是wchar_t类型的字符串
                CStdString svr_name((const wchar_t*)(m[0]->data()), m[0]->size() / 2);
                CStdString mac_addr((const wchar_t*)(m[1]->data()), m[1]->size() / 2);
                CStdString content((const wchar_t*)(m[2]->data()), m[2]->size() / 2);

                std::wcout<<L"user ["<<mac_addr.c_str()<<L"] activity: ["<<svr_name.c_str()<<"]"<<std::endl;

                // 根据服务名做相应的操作
                if (svr_name.CompareNoCase(L"feed") == 0)
                {
                    // 写到数据库
                    db::GetInstance()->UserFeedback(mac_addr, content, L"");
                }
                else if (svr_name.CompareNoCase(L"login") == 0)
                {
                    // 写到数据库
                    db::GetInstance()->UserLogin(mac_addr);
                }
                else if (svr_name.CompareNoCase(L"logout") == 0)
                {
                    // 写到数据库
                    db::GetInstance()->UserLogout(mac_addr);
                }
                else if (svr_name.CompareNoCase(L"require_feed_qcw") == 0)
                {
                    // 写到数据库
                    db::GetInstance()->UserLogout(mac_addr);
                }

                // 处理完成
                const_cast<UserMsg*>(a_msg)->handled = true;
            }
        }

        Sleep(10);
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
void CUserFeedWorkItem::DoWork( void* thread_context )
{
    // param_ = *reinterpret_cast<user_feed_param*>(thread_context);
    assert(nullptr != param_);

    // 接收者线程
    sock_ = new zmq::socket_t(*(param_->ctx), ZMQ_REP);
    sock_->connect(param_->ip_port.c_str());

    run_ = true;

    // 等待消息的到来
    int has_more;
    while (run_)
    {
        scan_usefeed();

        UserMsg *uf = new UserMsg;
        OneMsg& msg = uf->msg;

        bool bok = false;
        while (run_)
        {
            zmq::message_t* msg_frame = new zmq::message_t;

            std::cout<<"LitteT server tool thread: begin recv ...."<<std::endl;

            // 阻塞式的收消息
            if (!sock_->recv(msg_frame))
            {
                delete msg_frame;
                break;
            }
#ifdef _DEBUG
            std::cout<<"msg recived."<<std::endl;
#endif
            // 记录一帧消息
            msg.push_back(msg_frame);

            // 是否更多消息？
            size_t size_len = sizeof(has_more);
            sock_->getsockopt(ZMQ_RCVMORE, &has_more, &size_len);
            if (!has_more)
            {
                bok = true;
                break;
            }
        }
        
        if (bok)
        {
            // 一个消息接收完成, 可以放入数据队列了
            msgs_.push_back(uf);

            // 发回一个消息
            const char* buf = "thanks!";
            sock_->send(buf, strlen(buf));
        }
        else
        {
            delete uf;
        }
    }
}

void CUserFeedWorkItem::Abort()
{
    run_ = false;

    if (nullptr != sock_)
    {
        sock_->disconnect(param_->ip_port.c_str());
    }
}

int CUserFeedWorkItem::scan_usefeed()
{
    auto i_e = msgs_.end();
    for (auto i_msg = msgs_.begin(); i_msg != i_e; )
    {
        if ((*i_msg)->handled)
        {
            OneMsg& m = (*i_msg)->msg;
            for (auto i_frame = m.begin(); i_frame != m.end(); ++i_frame)
            {
                delete *i_frame;
            }
            delete *i_msg;

            i_msg = msgs_.erase(i_msg);
        }
        else
        {
            ++i_msg;
        }
    }
    return msgs_.size();
}
