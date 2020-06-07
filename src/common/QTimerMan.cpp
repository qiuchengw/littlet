#pragma warning(disable:4244 4018)

#include "QTimerMan.h"
#include "QHelper.h"
#include "QDBHelper.h"
#include "ui/QUIGlobal.h"
#include "xtrace.h"

VOID CALLBACK TaskCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired);
// 自动任务提前提示的的回调函数
VOID CALLBACK TaskRemindCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired)
{
	ASSERT(lpParameter != NULL);
    TRACE(L"TaskRemindCallback :: %p\n",lpParameter);
    QUIPostCodeToMainWnd(MWND_CMD_SHOWAUTOTASKREMIND,(LPARAM)lpParameter);
}

//////////////////////////////////////////////////////////////////////////
BOOL QTimerMan::Startup()
{
	m_hTimerQueue = CreateTimerQueue();
	if (NULL == m_hTimerQueue)
	{
		return FALSE;
	}
	return TRUE;
}

QTimer* QTimerMan::GetTimer( int nTimerID )
{
	TimerListItr itr = _FindTimer(nTimerID);
	if (m_lstTimer.end() != itr)
		return *itr;
	QTimer* pTimer = QDBEvents::GetInstance()->Timer_Get(nTimerID);
	if (NULL != pTimer)
	{
		m_lstTimer.push_back(pTimer);
	}
	return pTimer;
}

TimerListItr QTimerMan::_FindTimer( int nID )
{
	TimerListItr itrEnd = m_lstTimer.end();
	for (TimerListItr itr = m_lstTimer.begin();
		itr != itrEnd; ++itr)
	{
		if ((*itr)->ID() == nID)
			return itr;
	}
	return itrEnd;
}

BOOL QTimerMan::DeleteTimer( QTimer *pTimer )
{
    if (StopTimer(pTimer))
    {
        if (QDBEvents::GetInstance()->Timer_Delete(pTimer->ID()))
        {
            TimerListItr itr = _FindTimer(pTimer->ID());
            if (m_lstTimer.end() != itr)
            {
                delete *itr;
                m_lstTimer.erase(itr);
            }
            return TRUE;
        }
    }
	return FALSE;
}

BOOL QTimerMan::RemoveTimer( QTimer* pTimer )
{
	ASSERT(NULL != pTimer);
	pTimer->Stop(m_hTimerQueue);
	TimerListItr itr = _FindTimer(pTimer->ID());
	if (m_lstTimer.end() != itr)
	{
		delete *itr;
		m_lstTimer.erase(itr);
	}
	return FALSE;
}

BOOL QTimerMan::StopTimer( QTimer* pTimer )
{
	if (nullptr != pTimer)
	{
		return pTimer->Stop(m_hTimerQueue);
	}
	ASSERT(FALSE);
	return TRUE;
}

ENUM_AUTOTASK_RUNNING_STATUS QTimerMan::StartTimer( QTimer *pTimer ,int nTaskID)
{
	if (pTimer != NULL)
	{
		return pTimer->Start(m_hTimerQueue,nTaskID);
	}
    return AUTOTASK_RUNNING_STATUS_BADTIMER;
}

QTimer* QTimerMan::AddTimer( const QTime &tmBegin,
	const QTime& tmEnd, LPCWSTR szWhen,
	LPCWSTR szReminder,LPCWSTR szXField )
{
	int nID = QDBEvents::GetInstance()->Timer_Add(
			tmBegin,tmEnd,szWhen,szReminder,szXField);
	if (INVALID_ID != nID)
	{
		QTimer *pTimer = new QTimer(nID,tmBegin,tmEnd,szWhen,szReminder,szXField);
		m_lstTimer.push_back(pTimer);
		return pTimer;
	}
	return NULL;
}

BOOL QTimerMan::EditTimer( QTimer* pTimer,const QTime &tmBegin,
	const QTime& tmEnd, LPCWSTR szWhen,
	LPCWSTR szReminder,LPCWSTR szXField )
{
	if (pTimer->IsStarted())
	{
		ASSERT(FALSE);
		return FALSE;
	}
    if (pTimer->Update(pTimer->ID(), tmBegin, tmEnd, szWhen, szReminder, szXField))
    {
        return QDBEvents::GetInstance()->EditTimer(pTimer->ID(),
            tmBegin,tmEnd,szWhen,szReminder,szXField);
    }
	return FALSE;
}

BOOL QTimerMan::SetRemindExp( QTimer* pTimer,LPCWSTR pszRmdExp )
{
	if ((NULL == pTimer) || (pTimer->IsStarted()))
	{
		ASSERT(FALSE);
		return FALSE;
	}
    if (pTimer->SetRemindExp(pszRmdExp))
    {
	    return QDBEvents::GetInstance()->Timer_SetRemindExp(pTimer->ID(),pszRmdExp);
    }
	return FALSE;
}

void QTimerMan::Stop()
{
	RemoveAllTimer();
	DeleteTimerQueue(m_hTimerQueue);
}

void QTimerMan::RemoveAllTimer()
{
	for (TimerListItr i = m_lstTimer.begin(); i != m_lstTimer.end(); ++i)
	{
		(*i)->Stop(m_hTimerQueue);
		delete (*i);
	}
	m_lstTimer.clear();
}

BOOL QTimerMan::EnableTimerReminder( QTimer*pTimer,int nTaskID,BOOL bEnabled )
{
	if ((NULL == pTimer) || (nTaskID < 0))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	return pTimer->EnableReminder(m_hTimerQueue,nTaskID,bEnabled);
}

ENUM_AUTOTASK_RUNNING_STATUS QTimerMan::JumpoverTimerThisExec( QTimer *pTimer,int nTaskID )
{
	return pTimer->JumpoverThisExec(m_hTimerQueue,nTaskID);
}

//////////////////////////////////////////////////////////////////////////
void QTimerEventHandler::OnTimerReminderSetted( TASK_REMINDER_PARAM* pParam )
{
    if (NULL != pParam)
    {
        QUIPostCodeToMainWnd(MWND_CMD_SHOWAUTOTASKREMIND,(LPARAM)pParam);
    }
}


