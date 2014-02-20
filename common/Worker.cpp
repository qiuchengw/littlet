#include "Worker.h"
#include <process.h>

#include "ui/QUIDlgs.h"
#include "../common/QHelper.h"
#include "../common/QAutoTask.h"


QWorker::QWorker()
{
	m_hThread = NULL;
	m_nThreadID = 0;
	m_bRun = FALSE;
	m_hWorkEvent = NULL;
}

BOOL QWorker::Startup()
{
	if (m_hThread != NULL)
		return TRUE;
	m_hThread = (HANDLE)_beginthreadex(NULL,0,
		QWorker::WorkerThread,this,0,&m_nThreadID);
	if (NULL == m_hThread)
	{
		return FALSE;
	}
	return TRUE;
}

unsigned __stdcall QWorker::WorkerThread( void* pArguments )
{
	QWorker *pThis = reinterpret_cast<QWorker*>(pArguments);
	ASSERT(NULL != pThis);

	// Work event
	pThis->m_hWorkEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	if (NULL == pThis->m_hWorkEvent)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	pThis->m_bRun = TRUE;
	LPWORK_PARAM pWorkParam;
	while (pThis->m_bRun)
	{
		WaitForSingleObject(pThis->m_hWorkEvent,INFINITE);
		ResetEvent(pThis->m_hWorkEvent);

		// get a work
		pThis->m_cMutex.Lock();
		pWorkParam = NULL;
		ASSERT(!(pThis->m_ItemQueue.empty()));
		if (!(pThis->m_ItemQueue.empty()))
		{
			pWorkParam = pThis->m_ItemQueue.front();
			pThis->m_ItemQueue.pop();
			pThis->m_WorkingList.push_back(pWorkParam);
		}
		pThis->m_cMutex.Unlock();

		// do work
		if (pWorkParam != NULL)
		{
			QueueUserWorkItem(QWorker::QWorkItem,pWorkParam,WT_EXECUTEDEFAULT);
		}
	}
	CloseHandle(pThis->m_hWorkEvent);
	pThis->m_hWorkEvent = NULL;

	return 0;
}

void QWorker::EndWorker()
{
	m_bRun = FALSE;
	m_hThread = NULL;
}

DWORD WINAPI QWorker::QWorkItem(LPVOID lpParameter)
{
	LPWORK_PARAM pWP = reinterpret_cast<LPWORK_PARAM>(lpParameter);
	if (NULL == pWP)
	{
		ASSERT(FALSE);
		return 0;
	}
	switch(pWP->eWorkType)
	{
	case WORK_TYPE_AUTOTASKFIRED:
		{
			QAutoTask *pTask = reinterpret_cast<QAutoTask*>(pWP->lParam);
			if (NULL != pTask)
            {
                if (pTask->GetDoWhat() == AUTOTASK_DO_EXECPROG)
                {
                    ShellExecute(NULL,NULL,pTask->Task(),NULL,NULL,SW_SHOWNORMAL);
                }
                pTask->TaskFired();

//                 else
//                 {
//                     pTask->TaskFired();
//                     QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKFIRED,(LPARAM)pTask);
//                 }
            }
            break;
		}
	}
	pWP->bHandled = TRUE;
	return 0;
}

void QWorker::DoWork( LPWORK_PARAM pWork )
{
	if (NULL == pWork)
	{
		ASSERT(FALSE);
		return ;
	}
	LPWORK_PARAM pCopy = new WORK_PARAM;
	*pCopy = *pWork;
	pCopy->bHandled = FALSE;

	// queue work
	m_cMutex.Lock();
	CheckWorkingList();
	m_ItemQueue.push(pCopy);
	m_cMutex.Unlock();

	// notify the work thread
	SetEvent(m_hWorkEvent);
}

void QWorker::CheckWorkingList()
{
	WorkingItemList::iterator itrEnd = _EndItr(m_WorkingList); 
	for (WorkingItemList::iterator itr = _BeginItr(m_WorkingList);
		itr != itrEnd; ++itr)
	{
		if ((*itr)->bHandled)
		{
			delete *itr;
			itr = m_WorkingList.erase(itr);
			if (itr == itrEnd)
				break;
		}
	}
}



