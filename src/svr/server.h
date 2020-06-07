#pragma once

#include <winsock2.h>
#include <windows.h>

#include <vector>
#include <list>

#include "WorkQueue.h"

#include "deps/zmq.hpp"

#pragma comment(lib, "libzmq.lib")

typedef std::vector<zmq::message_t*> OneMsg;
struct  user_feed_param
{
    zmq::context_t* ctx;
    std::string     ip_port;  
};

struct UserMsg
{
    UserMsg()
    {
        handled = false;
    }

    OneMsg  msg;
    bool handled;
};

typedef std::list<UserMsg*> Msgs;

/**
 *	服务器进程
 *      1，接收用户意见反馈
 *      2，接收自动上传的bug（todo）
 *
**/
class CMsgServer
{
public:
    CMsgServer(void);
    ~CMsgServer(void);

    // 启动服务
    int Startup();

protected:
    // 数据处理线程
    static UINT_PTR __stdcall thread_handlemsg(void* param);

private:
    // 意见接收线程
    zmq::context_t  ctx_;
};

