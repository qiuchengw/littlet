#pragma once

#include "QTimer.h"
#include "ConstValues.h"
#include "basetype.h"

DWORD HowManySecondsWillPendding();
VOID CALLBACK TaskCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired);

class QDBEvents;
class QWorker;
class QAutoTask
{
    friend VOID CALLBACK TaskCallback(__in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);

    friend class QAutoTaskMan;
    friend class QDBEvents;
    friend class QWorker;

public:
// 	[ID] INTEGER PRIMARY KEY AUTOINCREMENT DEFAULT (1),
// 	[TimerID] INT(4) NOT NULL,
//	[Task] TEXT NOT NULL,
// 	[What] INT(4) NOT NULL,
// 	[Flag] INT(4) NOT NULL,
// 	[CreateTime] DOUBLE(8) NOT NULL DEFAULT (0.0)
    QAutoTask(LPCWSTR szTask, int nID,int nTimerID,
              ENUM_AUTOTASK_DOWHAT eDo, int nFlag,QTime tmCreate );

    CStdString GetNextExecTimeString() const;

    BOOL GetNextExecTime(QTime &tmNext)const;

    CStdString GetDoWhatString()const;

    CStdString GetWhenDoString()const;

    CStdString GetLifeTimeString()const;

    inline DWORD Flag()const
    {
        return m_nFlag;
    }

    /*
     *	是否是已经启动，并且是最后一次执行任务
     *      以下标志返回true：
     *
     */
    BOOL IsStartupAndLastExec() const;

    // 是否启用定时器
    BOOL EnableReminder(BOOL bEnable=TRUE);

    BOOL IsReminderEnabled()const ;

    BOOL IsOverdue()const;

    BOOL IsStartup()const;

    BOOL IsPaused()const;

    inline QTime CreationTime()const
    {
        return m_tmCreate;
    }

    inline int ID()const
    {
        return m_nID;
    }

    inline CStdString Task()const
    {
        return m_sTask;
    }

    inline CStdString RemindExp()const
    {
        return m_pTimer?m_pTimer->GetRemindExp():L"";
    }

    inline CStdString TimerExp()const
    {
        return m_pTimer?m_pTimer->GetWhenExp():L"";
    }

    inline QTimer* GetTimer()const
    {
        return m_pTimer;
    }

    inline QTime LifeBegin()const
    {
        return m_pTimer?m_pTimer->GetLifeBegin():QTime();
    }

    inline QTime LifeEnd()const
    {
        return m_pTimer?m_pTimer->GetLifeEnd():QTime();
    }

    inline ENUM_AUTOTASK_DOWHAT GetDoWhat()const
    {
        return m_eDoWhat;
    }

    inline BOOL IsHasRunningFlag(int bitFlag)const
    {
        return m_nRunningFlag & bitFlag;
    }

    inline int GetTimerID() const
    {
        return (nullptr!=m_pTimer)?m_pTimer->ID():INVALID_ID;
    };

    BOOL Edit( LPCWSTR szTask,ENUM_AUTOTASK_DOWHAT eDo,int nFlag );

    // Run
    // 如果不具有Pause标志，就Startup
    BOOL Run();

    // 启动和暂停任务
    BOOL Startup();

    BOOL Pause();
    // 跳过此次任务的执行，直接到下一次执行时间执行
    BOOL JumpoverThisExec();

    CStdString GetLastStartStatusDes()const;

    inline ENUM_AUTOTASK_RUNNING_STATUS GetLastStartStatus()const
    {
        return m_eLastStatus;
    };

    BOOL SetDoWhat( ENUM_AUTOTASK_DOWHAT eDo ,LPCWSTR szTask);

    // 任务更改后改动到数据库
//	BOOL Update();
    ENUM_AUTOTASK_EXECFLAG GetExecFlag()const;

    BOOL SetTimer(const QTime&tmBegin,const QTime&tmEnd,LPCWSTR sTimerExp);
    
    CStdString GetRemindString()const;

protected:
    // 定时器执行了
    BOOL TaskFired();
    
    // 此函数用于将flag保存到数据库中
    BOOL FlagChanged();

private:
    int					m_nID;			// 任务ID
    ENUM_AUTOTASK_DOWHAT	m_eDoWhat;			//
    QTimer*				m_pTimer;
    ENUM_AUTOTASK_RUNNING_STATUS	m_eLastStatus; // 最后一次调用Start的状态
    int					m_nFlag;
    CStdString				m_sTask;
    QTime				m_tmCreate;

private:
    DWORD				m_nRunningFlag;	//运行期状态标志
};

typedef std::list<QAutoTask*> AutoTaskList;
typedef AutoTaskList::iterator TaskListItr;

class QAutoTaskMan
{
    SINGLETON_ON_DESTRUCTOR(QAutoTaskMan)
    {
        RemoveAll();
    }

public:
    QAutoTaskMan();

    int GetTaskCount()const;
    int GetOverdueTaskCount();
    BOOL DeleteTask( QAutoTask* pTask );
    QAutoTask* AddTask( LPCWSTR szTask,int nTimerID, ENUM_AUTOTASK_DOWHAT eDo,int nFlag );
    // 从数据库中读取的已有任务
    QAutoTask* GetTask(int nID);
    BOOL SetTaskTimer(QAutoTask* pTask,QTimer* pTimer);

    BOOL Startup();
    void GetTaskList(AutoTaskList &lst);
    // 是否启用自动任务的提示消息
    BOOL EnableTaskReminder(int nTaskID,BOOL bEnable=TRUE);
    BOOL IsTaskReminderEnabled( int nTaskID );
    // 跳过任务的此次执行，到下一次执行
    BOOL JumpoverTaskThisExec(INT nTaskID);
    // 获取最据当前最近执行的任务
    QAutoTask* GetMostCloseExecute();
    // Task 过期了
    void TaskOverdue(QAutoTask* pTask);
    // 过期任务重置
    BOOL ResetOverdueTask(QAutoTask* pTask);

    /** 创建一个新的自动任务
     *	return:
     *      QAutoTask*    创建好的任务指针
     *	params:
     *		-[in]
     *
     *		-[out]
     *          sError      错误信息
    **/
    QAutoTask* NewAutoTask( ENUM_AUTOTASK_DOWHAT nDoWhat,
                            const CStdStringW& sDoWhatParam, const CStdStringW& sWhenDo,
                            const CStdStringW& sRemindexp, QTime tmBegin, QTime tmEnd,
                            __out CStdStringW& sError );

protected:
    void RemoveAll();
    TaskListItr _FindTask( const QAutoTask* pTask );
    TaskListItr _FindTask( int nID );
    void OnWindowCreated(HWND hWnd);
    void OnWindowDestroyed(HWND hWnd);

private:
    AutoTaskList	m_lstTask;
};

