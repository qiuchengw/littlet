#pragma once

#include "QTimer.h"
#include "basetype.h"

typedef std::list<QTimer*>	TimerList;
typedef TimerList::iterator TimerListItr;

class QTimerMan
{
	SINGLETON_ON_DESTRUCTOR(QTimerMan)
	{
		Stop();
	}
	
public:
	BOOL Startup();

	QTimer* GetTimer(int nID);
	QTimer* AddTimer(const QTime &tmBegin,const QTime& tmEnd,
		LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXField);
	BOOL EditTimer(QTimer* pTimer,const QTime &tmBegin,const QTime& tmEnd,
		LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXField);
	BOOL SetRemindExp(QTimer* pTimer,LPCWSTR pszRmdExp);

	BOOL DeleteTimer( QTimer *pTimer );
	BOOL RemoveTimer( QTimer* pTimer );

	BOOL StopTimer(QTimer* pTimer);
	ENUM_AUTOTASK_RUNNING_STATUS StartTimer(QTimer *pTimer,int nTaskID);
	BOOL EnableTimerReminder(QTimer*pTimer,int nTaskID,BOOL bEnabled);
	ENUM_AUTOTASK_RUNNING_STATUS JumpoverTimerThisExec(QTimer *pTimer,int nTaskID);

protected:
	TimerListItr _FindTimer(int nID);
	void RemoveAllTimer();
	void Stop();

private:
	
private:
	TimerList		m_lstTimer;
	HANDLE			m_hTimerQueue;
};

#include "QTimerListener.h"

class QTimerEventHandler : public QTimerEventHandlerBase
{
public:
    virtual void OnTimerReminderSetted(TASK_REMINDER_PARAM* pParam);
};

