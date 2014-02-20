#pragma once

#include "BaseType.h"
#include "QBuffer.h"

/** 更新应用程序文件
 *	return:
 *      无返回
 *	params:
 *		-[in]
 *          wParam      1，有更新， 0，当前无更新
 *          lParam      当wParam==0，此域也为0 
 *                        wParam==1，此域为LPQUI_USERMSGPARAM，
 *  -----------------------------------------
 *          LPQUI_USERMSGPARAM 域的值为：
 *                      wParam  1：更新成功
 *                              0：更新失败
 *                      sParam  已下载的更新文件路径
 *                      bFreeIt TRUE    接收者应该删除这个数据
 *		-[out]
 *      无输出
**/
DECLARE_USER_MESSAGE(QSOFT_MSG_UPDATEAPP);

VOID CALLBACK CheckUpdateCallback(
    __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);
class QAutoUpdater
{
    friend VOID CALLBACK CheckUpdateCallback(
            __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);
    
    SINGLETON_ON_DESTRUCTOR(QAutoUpdater)
    {
        if (NULL != m_hTimer)
        {
            DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
        }
    }
public:
    QAutoUpdater(void);

    /** 启动自动更新程序
     *	return:
     *      TRUE    启动成功
     *	params:
     *		-[in]
     *          dwCheckPeriod   检查更新的频率，单位时间（分）
     *                          最小检查频率为10分钟
     *          sURL            网络地址
     *          szRefer         破解防盗链 如设置：http://www.sina.com，
     *
    **/
    BOOL Startup( __in const QString& sURL, __in LPCWSTR szRefer,
            __in DWORD dwCheckPeriod = 30);

protected:
    /** 异步方式检查程序更新.
     *      如果启动检查成功，检查结束后将会向程序的主窗口发送一个QSOFT_MSG_UPDATEAPP消息
     *	return:
     *      TRUE    启动成功
    **/
    BOOL CheckUpdate( );

    // 下载线程回调函数
    struct _Download_Param 
    {
        QString sURL;	// in
        QString sRefer;	// in
        LPVOID	pData; // in
        QBuffer	bufDown;	// out 下载的数据
    };
    /** 下载数据的线程
     *	params:
     *		-[in]
     *          param            _Download_Param* 
    **/
    static uint __stdcall thread_download(void* param);

private:
    //BOOL				m_bEnd;
    HANDLE              m_hTimer;   // 检查更新的时间定时器
    QString             m_sUrl;
    QString             m_sRefer;   // 可以破解防盗链
};

