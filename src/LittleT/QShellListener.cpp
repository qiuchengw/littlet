#include "QShellListener.h"
#include "../common/LittleTcmn.h"
#include "XTrace.h"
#include "stdstring.h"
#include <process.h>

QShellListener::QShellListener(void)
{
    m_bContinueListen = TRUE;
    m_bStarted = FALSE;
}

BOOL QShellListener::Startup()
{
    if (m_bStarted)
        return TRUE;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, 
        &QShellListener::ListenThread, this, 0, NULL);
    if (hThread > 0)
    {
        m_bStarted = TRUE;
        CloseHandle(hThread);
    }
    m_bContinueListen = m_bStarted;
    return m_bStarted;
}

BOOL QShellListener::Shutdown()
{
    m_bContinueListen = FALSE;
    return TRUE;
}

UINT_PTR __stdcall QShellListener::ListenThread( void* parm )
{
    QShellListener *pThis = reinterpret_cast<QShellListener*>(parm);
    ASSERT(NULL != pThis);
    int iTry = 0;
    do 
    {
        // The main loop creates an instance of the named pipe and 
        // then waits for a client to connect to it. When the client 
        // connects, a thread is created to handle communications 
        // with that client, and the loop is repeated. 
        HANDLE hPipe = CreateNamedPipe( LITTLET_SHELL_PIPLENAME, PIPE_ACCESS_DUPLEX,       // read/write access 
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,  // 重叠，阻塞，消息，读模式，
            1, // 只要创建一个命名管道 
            G_LITTLET_SHELL_PIPEL_BUFFER_SIZE,                  // output buffer size 
            G_LITTLET_SHELL_PIPEL_BUFFER_SIZE,                  // input buffer size 
            0,                        // client time-out 
            NULL);                    // default security attribute 
        if (hPipe == INVALID_HANDLE_VALUE) 
        {
            if (iTry++ > 3)   // 尝试创建3次，如果都错误的话，退出
            {
                //MessageBox(NULL,L"进程通信错误，将不能使用Shell进程创建自动任务！");
                return 0;
            }
            break;
        }

        while (pThis->m_bContinueListen)
        { 
            // Wait for the client to connect; if it succeeds, 
            // the function returns a nonzero value. If the function
            // returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
            TRACE(L"--->Wait for Shell Connect the pipe;\n");
            BOOL fConnected = ConnectNamedPipe(hPipe, NULL) 
                                ? TRUE 
                                : (GetLastError() == ERROR_PIPE_CONNECTED); 
            if (fConnected) 
            { 
                // Shell进程连接了
                TRACE(L"--->Shell Connected\n");
                // 处理连接
                pThis->HandleShellConnected(hPipe);
            } 
            else
            {
                TRACE(L"--->Wait shell Connect Failed!\n");
                // The client could not connect, so close the pipe. 
                CloseHandle(hPipe); 
                // 关闭通道，再来一次
                break;
            }
        } 
    } while (pThis->m_bContinueListen);

    return 0; 
}

BOOL QShellListener::HandleShellConnected(HANDLE hPipe) 
{ 
    wchar_t szBufRequest[G_LITTLET_SHELL_PIPEL_BUFFER_SIZE]; 
    DWORD cbBytesRead = 0; 

    // Read client requests from the pipe. 
    BOOL fSuccess = ReadFile( 
        hPipe,        // handle to pipe 
        szBufRequest,    // buffer to receive data 
        G_LITTLET_SHELL_PIPEL_BUFFER_SIZE * sizeof(wchar_t), // size of buffer 
        &cbBytesRead, // number of bytes read 
        NULL);        // not overlapped I/O 
    if ( !fSuccess || (cbBytesRead == 0)) 
        return FALSE;  // 读错误

    // 处理数据
    CStdString sReply;
    HandleShellRequest(szBufRequest, cbBytesRead, sReply);

    // 回复请求
    ReplyShellRequest(hPipe, sReply);

    return TRUE;
}

BOOL QShellListener::HandleShellRequest( __in wchar_t* pMsg, __in DWORD nLen, __out CStdString& sReply )
{
    TRACE(L"--->Shell Msg:%s", pMsg);

    return TRUE;
}

BOOL QShellListener::ReplyShellRequest( __in HANDLE hPipe, __in const CStdString& sReply )
{
    DWORD cbWritten = 0;
    DWORD cbReplyBytes = sReply.GetLength() * sizeof(CStdString::size_type);
    // Write the reply to the pipe. 
    BOOL fSuccess = WriteFile( 
                hPipe,        // handle to pipe 
                sReply,      // buffer to write from 
                cbReplyBytes, // number of bytes to write 
                &cbWritten,   // number of bytes written 
                NULL);        // not overlapped I/O 
    if ( fSuccess && (cbReplyBytes == cbWritten))
    {
        // Flush the pipe to allow the client to read the pipe's contents 
        FlushFileBuffers(hPipe); 
        return TRUE;
    }
    return FALSE;
}
