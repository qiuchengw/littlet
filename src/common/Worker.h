#pragma once

#include <queue>
#include "BaseType.h"
#include "ConstValues.h"
#include "AppHelper.h"

typedef struct WORK_PARAM 
{
	WORK_PARAM& operator=(const WORK_PARAM& o)
	{
		eWorkType = o.eWorkType;
		lParam = o.lParam;
		pData = o.pData;
		bHandled = o.bHandled;

		return *this;
	}
	ENUM_WORK_TYPE		eWorkType;
	LPARAM				lParam;		// 含义根据 eWorkType 而定
	LPVOID				pData;		// 
	BOOL				bHandled;	// has done this work?
}*LPWORK_PARAM;

class QWorker
{
	typedef std::queue<LPWORK_PARAM> WorkItemQueue;
	typedef std::list<LPWORK_PARAM> WorkingItemList; 

	SINGLETON_ON_DESTRUCTOR(QWorker)
    {
        CheckWorkingList();
    }

public:
	QWorker();

	// 启动Worker 线程
	BOOL Startup();

	// 终止线程
	void EndWorker();
	void DoWork(LPWORK_PARAM pWork);

protected:
	// Worker线程
	static unsigned __stdcall WorkerThread( void* pArguments );
	static DWORD WINAPI QWorkItem(LPVOID lpParameter);

	void CheckWorkingList();

private:
	HANDLE	m_hThread;
	unsigned int	m_nThreadID;
	BOOL	m_bRun;
	HANDLE	m_hWorkEvent;
	WorkItemQueue	m_ItemQueue;
	WorkingItemList m_WorkingList;
	QMutex		m_cMutex;
};

