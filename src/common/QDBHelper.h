#pragma once

#include "BaseType.h"
#include <list>
#include "time/QTime.h"
#include "file/db/QDBMgr.h"
#include "../common/QAutoTask.h"
#include "stdstring.h"

class QIdea;
typedef std::vector<QIdea*> IdeaVec;
typedef IdeaVec::iterator IdeaVecItr;

// 即时任务管理器
enum ENUM_TODOTASK_FLAG
{
	TODO_FLAG_PLANRELATED = 0x1, // 和大计划相关联的任务
	TODO_FLAG_NEEDREMIND = 0x2, // 需要提前提示
    TODO_FLAG_HASEXECTIME = 0x4, // 需要提前提示
    TODO_FLAG_STICKYNOTE = 0x8, // 显示为便签
    TODO_FLAG_ENCRYPTED = 0x10, // 数据已经加密
};

struct TCate
{
	int nID;
	CStdString sCate;
	CStdString sDes;
	int	nIconID;
	QTime	tmCreate;
	QTime	tmUpdate;
};
typedef std::vector<TCate> VecCate;
typedef VecCate::iterator VecCateItr;

enum ENUM_TODO_STATUS
{
	TODO_STATUS_PROCESSING,
	TODO_STATUS_FINISH,
//	TODO_STATUS_ABORT,
    //-------------------
    // 非任务状态，只用于显示任务时候的指示
    TODO_STATUS_SHOWNONE,
    TODO_STATUS_SHOWALL,
};

struct TTodoTask
{
	int					nID;
	int					nCateID;
	int					nPriority;
	int					nFlag;
	CStdString				sTask;
	QTime				tmCreate;
	QTime				tmExec;
	ENUM_TODO_STATUS	eStatus;

	BOOL IsDone()const { return (eStatus == TODO_STATUS_FINISH); };
};
typedef std::list<TTodoTask> TodoTaskList;
typedef TodoTaskList::iterator TodoTaskListItr;

class QDBEvents : public DBMan
{
	friend class QAutoTaskMan;
	friend class QIdeaMgr;
	friend class QDBEvents;
	friend class QTimerMan;
	friend class QTimer;

    SINGLETON_ON_DESTRUCTOR(QDBEvents){
		OnDestruct();
	}
	void OnDestruct();

public:
	QDBEvents(){}

	BOOL Startup();

	// tasks
	//////////////////////////////////////////////////////////////////////////
	BOOL TodoTask_Delete(int nID);
	int TodoTask_Add(TTodoTask *pItem);
	BOOL TodoTask_SetStatus(int nID, ENUM_TODO_STATUS eStatus);
	BOOL TodoTask_SetPriority(int nID,int nPri);
	BOOL TodoTask_SetCate( int nTaskID,int nCateID );
	BOOL TodoTask_IsDone(int nID);
	BOOL TodoTask_Edit(TTodoTask* pItem);
	// bActiveAndDone 为TRUE的时候，eStatus 不使用
	// bAllTimeTask 为TRUE的时候，tmBegin,tmEnd 不使用
// 	BOOL TodoTask_Get(BOOL bActiveAndDone,ENUM_TODO_STATUS eStatus, 
// 		BOOL bAllTimeTask,const QTime& tmBegin,
// 		const QTime& tmEnd,__out TodoTaskList &lst);
	BOOL TodoTask_GetUnfinished( TodoTaskList &lst );
    BOOL TodoTask_GetFinished( TodoTaskList &lst );
	BOOL TodoTask_GetAll( TodoTaskList &lst );
    BOOL TodoTask_GetAllStickyNote(TodoTaskList &lst);

	int TodoTask_GetUnfinishNum( );
	int TodoTask_GetFinishedNum( );
	BOOL TodoTask_Get(int nID,__out TTodoTask &t);
    BOOL TodoTask_SetFlag(int nID, int flag);

public:
	//////////////////////////////////////////////////////////////////////////
	// cate
	// 获取目录下的任务列表
	BOOL Cate_IsExist(const CStdString& sCate);
	BOOL Cate_IsHasTask(int nCateID);
	int Cate_Add( const CStdStringW &sCate ,UINT nFlags,int nIconID,const CStdString& sDescription);
	BOOL Cate_Edit(int nCateID, const CStdStringW &sCate , int nIconID,const CStdString& sDescription);
	BOOL Cate_Delete(int nCateID);
	int Cate_CountNum();
	int Cate_CountTaskInIt( int nCateID );
	int Cate_GetAll(VecCate &vc);

public: // autotask
	int AutoTask_Add(LPCWSTR pszTask, int nTimerID,ENUM_AUTOTASK_DOWHAT eDo,int nFlag);
	BOOL AutoTask_Edit(int nTaskID, LPCWSTR pszTask, ENUM_AUTOTASK_DOWHAT eDo, int nFlag );
	BOOL AutoTask_SetFlag(int nTaskID,int nFlag);
	BOOL AutoTask_SetTimer(int nTaskID,int nTimerID);
	BOOL AutoTask_SetDo(int nTaskID, LPCWSTR pszTask,ENUM_AUTOTASK_DOWHAT eDo);
	int AutoTask_GetTimerID(int nTaskID);
	// 从数据库中删除任务
	BOOL AutoTask_Delete( int nID );
	BOOL AutoTask_GetInfo( int nTaskID, __out CStdString & sTask,
		__out int &nTimerID, __out ENUM_AUTOTASK_DOWHAT &eDo,
		__out int& nFlag,__out QTime &tmCreate);
protected:
	// 读取未完成的任务
	BOOL AutoTask_GetAll( AutoTaskList & lst );

protected:
	//////////////////////////////////////////////////////////////////////////
	// Idea
	BOOL GetIdeas(__in int x, __in int y, __out IdeaVec &v);
	BOOL DeleteIdea(int nID);
	int AddIdea(const CStdString& sContent);

protected:
	//////////////////////////////////////////////////////////////////////////
	// Timer
	// [ID] INTEGER PRIMARY KEY AUTOINCREMENT DEFAULT (1), 
	// [BeginTime] DOUBLE NOT NULL,
	// [EndTime] DOUBLE NOT NULL,
	// [What] INT(4) NOT NULL,
	// [When] TEXT,
	// [Reminder] TEXT,
	// [XFiled] TEXT);
	int Timer_Add(const QTime &tmBegin,const QTime& tmEnd,
		LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXField);
	BOOL Timer_Delete(int nID);
	BOOL EditTimer(int nID,const QTime &tmBegin,const QTime& tmEnd,
		LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXField);
	BOOL Timer_SetRemindExp( int nID,LPCWSTR pszRmdExp );
	QTimer* Timer_Get(int nID);
	BOOL Timer_GetInfo( int nID,__out QTime&tmBegin,
		__out QTime&tmEnd, __out CStdString &sWhenExp,
		__out CStdString &sRemindExp,__out CStdString &sXFiled );
protected:
	BOOL TodoTask_Read( LPCWSTR szSQL ,__out TodoTaskList & lst);
	BOOL _TodoTask(SqlQuery &q ,__out TTodoTask&t);
	BOOL _Cate(SqlQuery &q,__out TCate &c);
};


