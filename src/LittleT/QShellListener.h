#pragma once

#include "BaseType.h"
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>

// 监听LittleTShell的管道消息

class QShellListener
{
    SINGLETON_ON_DESTRUCTOR(QShellListener)
    {
        Shutdown();
    }

public:
    QShellListener(void);

    // 启动监听
    BOOL Startup();
    
    // 关闭监听
    BOOL Shutdown();

protected:
    // 永久线程，在程序运行期间一直监听
    static UINT_PTR __stdcall ListenThread(void* parm);
   
    /** 处理Shell发过来的消息
     *	return
     *      TRUE    消息被成功处理
     *	param
     *		-[in]
     *          pMsg        消息
     *          nLen        消息长度
     *		-[out]	
     *          sReply      发给Shell的回复
    **/
    BOOL HandleShellRequest(__in wchar_t* pMsg, __in DWORD nLen, __out CStdString& sReply);
    /** 回复Shell的请求
     *	return
     *      TRUE    成功回复
     *	param
     *		-[in]
     *          hPipe   管道
     *          sPeply  消息
    **/
    BOOL ReplyShellRequest(__in HANDLE hPipe, __in const CStdString& sReply);
    /** Shell 连接到Pipe
     *	return
     *	param
     *		-[in]
     *		-[out]	
     *
    **/
    BOOL HandleShellConnected(HANDLE hPipe);
private:
    BOOL    m_bStarted;
    BOOL    m_bContinueListen;
};

