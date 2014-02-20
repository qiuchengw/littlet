#pragma once

//////////////////////////////////////////////////////////////////////////
// 一个无窗口的定时器实现
// 使用方法：
//		1，调用全局函数 【StkSetTimer】设定定时器
//		2，调用全局函数 【StkKillTimer】删除定时器
//////////////////////////////////////////////////////////////////////////

#include <Windows.h>

class StkTimerMan
{
private:
	StkTimerMan() 
	{
		InitTimerQueue();
	}
	~StkTimerMan()
    {
        Destroy();
    }

protected:
	BOOL InitTimerQueue()
	{
		if (NULL == m_hTimerQueue)
		{
			m_hTimerQueue = CreateTimerQueue();
		}
		return NULL != m_hTimerQueue;
	}

public:
	static StkTimerMan* GetInstance()
	{
		static StkTimerMan _theInstance;
		return &_theInstance;
	}
	void Destroy()
	{
		if (NULL != m_hTimerQueue)
		{
			// 标记删除标志，不等待正在执行的回调完成，立即返回
			DeleteTimerQueueEx(m_hTimerQueue,NULL);
            m_hTimerQueue = NULL;
		}
	}

public:
	/** 设定定时器，MSDN：By default, the callback function is queued to a non-I/O worker thread.
	 *	-return:	
	 *			HANDLE		如果成功，返回定时器句柄。如果失败，返回NULL
	 *						相当于窗口（HWND）定时器（WM_TIMER）的定时器ID
	 *	-params:	
	 *		-[in]	dwMillsec	定时器间隔，单位毫秒 （ms）
	 *				pCallback	回调函数，函数原型为：
	 *					VOID CALLBACK WaitOrTimerCallback(
								 __in  PVOID lpParameter,
								 __in  BOOLEAN TimerOrWaitFired );
					pData	回调函数被激发的时候，传入的第一个参数值，见上面的原型
	 **/
	HANDLE SetTimer(DWORD dwMillsec,WAITORTIMERCALLBACK pCallback,LPVOID pData)
	{
		InitTimerQueue();

		HANDLE hTimer;
		BOOL bOK = CreateTimerQueueTimer(&hTimer,m_hTimerQueue,pCallback,
			pData,dwMillsec,dwMillsec,WT_EXECUTEDEFAULT);
		return bOK ? hTimer : NULL;
	}
	
	void KillTimer(HANDLE hTimer,BOOL bWaitForComplete=FALSE)
	{
		if ((NULL != hTimer) && (NULL != m_hTimerQueue))
		{
            __try
            {
                DeleteTimerQueueTimer(
                    m_hTimerQueue,
                    hTimer,
                    bWaitForComplete ? INVALID_HANDLE_VALUE : NULL);
            }
            __except(EXCEPTION_CONTINUE_EXECUTION)
            {
                // 在定时器回调中执行删除定时器操作有可能会产生异常
                //....
            }
		}
	}

private:
	HANDLE	m_hTimerQueue;
};

/** 设定定时器，MSDN：By default, the callback function is queued to a non-I/O worker thread.
	*	-return:	
	*			HANDLE		如果成功，返回定时器句柄。如果失败，返回NULL
	*						相当于窗口（HWND）定时器（WM_TIMER）的定时器ID
	*	-params:	
	*		-[in]	dwMillsec	定时器间隔，单位毫秒 （ms）
	*				pCallback	回调函数，函数原型为：
	*					VOID CALLBACK WaitOrTimerCallback(
								__in  PVOID lpParameter,
								__in  BOOLEAN TimerOrWaitFired );
				pData	回调函数被激发的时候，传入的第一个参数值，见上面的原型
	**/
inline HANDLE StkSetTimer(DWORD dwMillsec,WAITORTIMERCALLBACK pCallback,LPVOID pData=NULL)
{
	return StkTimerMan::GetInstance()->SetTimer(dwMillsec,pCallback,pData);
}

/** 删除定时器，如果回调正在执行
	 *	-params:	
	 *		-[in]	hTimer	需要Kill的定时器句柄
	 *						！！！需要确保此句柄为SetTimer调用的返回值
	 *						
	 *				bWaitForComplete	
	 *					此参数指示调用此函数的时候是否等待已激发的回调完成再返回
	 *					如果为TRUE，那么此函数阻塞，等待回调完成后返回
	 *					如果为FALSE，函数立即返回
	 **/
inline void StkKillTimer(HANDLE hTimer,BOOL bWaitForComplete=FALSE)
{
	StkTimerMan::GetInstance()->KillTimer(hTimer,bWaitForComplete);
}


