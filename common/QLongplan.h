#pragma once

#include "BaseType.h"
#include "time/QTime.h"
#include "../common/ConstValues.h"
#include "../common/QDBRes.h"

// sID是以字符‘;’分割的如： 1;2;5;8
// 将1;2;3;8 解析为整数数组返回
BOOL ParseID(const QString&sID,__out IntArray &vi);
// 返回形式：1;2;3;8
QString MakeIDS(__in IntArray &vi);


class QDBPlan;
enum ENUM_GOAL_STATUS
{
	GOAL_STATUS_WORKING,
	GOAL_STATUS_ABORT,
	GOAL_STATUS_CHECKED,
};

enum ENUM_GOALITEM_STATUS
{
    GOALITEM_STATUS_NOTSET = -1,    // 用于读取goalitem的时候，读取全部类型
	GOALITEM_STATUS_WAITING = 0,  //
	GOALITEM_STATUS_GAIN,
};

class LGoalitemWidget;
class QGoalItem
{
	friend class QDBPlan;
    friend class LGoalitemWidget;

public:
    QGoalItem()
    {
        m_nID = INVALID_ID;
        m_nTimerID = INVALID_ID;
        m_nGoalID = INVALID_ID;
        m_eStatus = GOALITEM_STATUS_WAITING;
    }
    QGoalItem(const QGoalItem& g);
    QGoalItem& operator=(const QGoalItem& o);

	inline int ID()const { return m_nID; }
	inline int GoalID()const { return m_nGoalID; }
	inline QString Item()const { return m_sItem; }
	inline QString Notes()const { return m_sNotes; }
	inline QString Attachs()const { return m_sAttachs; }
	inline ENUM_GOALITEM_STATUS Status()const { return m_eStatus; }
	inline QTime BeginTime()const { return m_tmBegin; }
	inline QTime EndTime()const { return m_tmEnd; }
	inline BOOL IsDone()const { return m_eStatus == GOALITEM_STATUS_GAIN; }
    inline int TimerID()const { return m_nTimerID; }
    inline BOOL IsHasTimer()const
    {
        return (INVALID_ID != m_nTimerID) && (m_nTimerID > 0);
    }

	int GetAttachs(__out VecFileData &vfd,BOOL bIncludeData=FALSE);
	int AddNotes(int nNoteID);

public:
    BOOL Edit(const QString& sItem, QTime tmBegin, QTime tmEnd);
    // 删除自己
    BOOL Delete();

    /** 设置状态
     *	return:
     *      TRUE        成功
     *	params:
     *		-[in]
     *          bChecked    完成
    **/
    BOOL CheckIt(BOOL bChecked = TRUE);

public:
	QString		m_sItem;
	int			m_nGoalID;
    int         m_nTimerID;
	QTime		m_tmBegin;
	QTime		m_tmEnd;
	ENUM_GOALITEM_STATUS	m_eStatus;
	QString		m_sAttachs;
	QString		m_sNotes;

private:
	int			m_nID;
	QTime		m_tmCreate;
	QTime		m_tmUpdate;
};

typedef std::vector<QGoalItem*> VecGoalItem;
typedef VecGoalItem::iterator VecGoalItemItr;

// 阶段目标
class LGoalDlg;
class QGoal
{
	friend class QDBPlan;
    friend class LGoalDlg;

public:
	QGoal();
    QGoal(const QGoal& g);
    QGoal& operator=(const QGoal& o);

	inline int ID()const { return m_nID; }
	inline int StageID()const { return m_nStageID; }
	inline QString Goal()const { return m_sGoal; }
	inline QString Des()const { return m_sDes; }
	inline ENUM_GOAL_STATUS Status()const { return m_eStatus; }
	inline LONG Flag()const {  return m_nFlag; }
	inline QTime CreationTime()const { return m_tmCreate; }
	BOOL AddItem(__inout QGoalItem &gi);
	BOOL DeleteItem(QGoalItem *gi);
	// eStatus 为NOTSET 则获取全部
	BOOL GetAllItems(__out VecGoalItem &vgi,ENUM_GOALITEM_STATUS eStatus = GOALITEM_STATUS_NOTSET);
	int GetGoalItemNum();
	int GetGoalItemUnfinishedNum();
	QString TimeFlee();

public:
    BOOL EditGoal( const QString& sGoal, const QString& sDes);
    // 删除自己，其下的goalitem也会被删除
    BOOL Delete();
    // 删除goal item
    BOOL DeleteGoalitem(QGoalItem* pGoalitem);
    // 返回的数据必须使用delete删除
    QGoalItem* AddGoalItem( __in const QGoalItem &gi );
public:
	QString				m_sGoal;
	QString				m_sDes;

private:
	int					m_nID;
	int					m_nStageID;
	LONG				m_nFlag;
	ENUM_GOAL_STATUS	m_eStatus;
	QTime				m_tmCreate;
	QTime				m_tmUpdate;
};
typedef std::vector<QGoal*> VecGoal;
typedef VecGoal::iterator VecGoalItr;

// 阶段
enum ENUM_STAGE_STATUS
{
	STAGE_STATUS_WORKING,
	STAGE_STATUS_FINISH,
	STAGE_STATUS_WAITING,
	STAGE_STATUS_ABORT,
};

class LStageDlg;
class QStage
{
	friend class QDBPlan;
    friend class LStageDlg;

public:
    QStage();
	QStage(const QStage& g);
    QStage& operator=(const QStage& o);

	inline int ID()const { return m_nID; }
	inline QString Des()const { return m_sDes; }
	inline QString Stage()const { return m_sStage; }
	inline ENUM_STAGE_STATUS Status()const { return m_eStatus; }
	inline QTime BeginTime()const { return m_tmBegin; }
	inline QTime EndTime()const { return m_tmEnd; }
	inline BOOL IsFinished()const { return FALSE; }
    inline int IconID()const { return m_nIconID; }
    BOOL SetIcon(int nIconID);
    QString IconPath()const;

	int GetGoalNum();
    int GetFinishedGoalNum();
    int GetWorkingGoalNum();
	int GetKonwledgeNum(); 
	// ret:0-100
	int GetStageProcess();  // 阶段进展
	// ret:0-100
	int GetTimeProcess();  // 规划的日期进展
	// ret: 0年3月2天
	QString GetTimeFlee();  // 从计划开始到现在已经过去了多久

	int GetAllGoals(__out VecGoal &vg);

public:
    // 返回的数据必须使用delete删除
    QGoal* AddGoal( __in const QGoal& g );

    // 删除自己
    BOOL Delete();

    // 删除下面的goal
    BOOL DeleteGoal(QGoal *pGoal);

    // edit
    BOOL Edit(const QString& sStage, const QString& sDes, QTime tmBegin, QTime tmEnd);

public:
	int		m_nPlanID;	// 所属长期计划ID
    int     m_nIconID;  // icon id
	LONG	m_nFlag;
	QString	m_sStage;		// 阶段计划描述 如：吉他-》基础练习
	QString m_sDes;		// 附加描述，
	QTime	m_tmBegin;		// 开始时间
	QTime	m_tmEnd;		// 结束时间
	ENUM_STAGE_STATUS	m_eStatus;

private:
	int		m_nID;
	QTime	m_tmCreate;
	QTime	m_tmUpdate;
};

typedef std::vector<QStage*> VecStage;
typedef VecStage::iterator VecStageItr;

// 成绩
struct TAchievement 
{
	int			nID;
	int			nParentID;	// 所属
	QString		sAchievement;	// 文本描述		
	QString		sAttachment;	// 附件ID列表； 形如 "2;4;35;"		
};

enum ENUM_PLAN_STATUS
{
	PLAN_STATUS_OK	= 0,	// 进行中
	PLAN_STATUS_PAUSE = 1,		// 暂停
	PLAN_STATUS_ABORT = 2,		// 放弃
	PLAN_STATUS_FINISH = 3,		// 完成
};

class LPlanDlg;
class QPlan
{
	friend class QDBPlan;
    friend class LPlanDlg;

public:
    QPlan();
	QPlan(const QPlan& p);
	~QPlan(void);
	QPlan& operator=(const QPlan& o);

public:
	/**
	 * @brief	:	获得任务跟踪列表
	 */
// 	QLongPlanTrackList* GetTaskTrackList();
// 
// 	inline BOOL IsHasFlag(UINT flag) { return (m_nFlag & flag);}
// 	inline BOOL IsAborted()const { return (m_eStatus==TASK_STATUS_ABORT);}
// 	inline BOOL IsFinished()const { return (m_eStatus==TASK_STATUS_FINISH);}
// 	inline BOOL IsPaused()const { return (m_eStatus==TASK_STATUS_PAUSE);}
// 	inline ENUM_TASK_STATUS GetStatus()const { return m_eStatus; }

	inline int ID()const { return m_nID; }
	inline int IconID()const { return m_nIconID; }
    inline QString IconPath()const;
	inline LONG Flag()const { return m_nFlag;}		// 任务标志
	inline QTime BeginTime()const { return m_tmBegin;}		// 任务周期开始生效时间
	inline QTime EndTime()const { return m_tmEnd; }		// 任务周期，预期结束时间
	inline QString Plan()const { return m_sPlan; }		// 
	inline QString Des()const { return m_sDes; }			// 
	inline QTime CreationTime()const { return m_tmCreate; }
	inline QTime UpdateTime()const { return m_tmUpdate; }
	inline QTime FinishedTime()const { return m_tmFinish; }
	inline ENUM_PLAN_STATUS Status()const { return m_eStatus; }

	int GetStageCount();
	int GetFinishStageCount();
	int GetAllStage(__out VecStage & vs);

	// ret:0-100
	int GetPlanProcess();  // 阶段进展
	// ret:0-100
	int GetTimeProcess();  // 规划的日期进展
	// ret: 0年3月2天
	QString GetTimeFlee();  // 从计划开始到现在已经过去了多久

public:
    /** 添加一个stage
     *	return:
     *      QStage* 指针，必须使用delete删除
     *	params:
     *		-[in]
     *          s       数据
     *
    **/
    QStage* AddStage(const QStage &s);

    // 删除自己
    BOOL Delete();

    BOOL Edit(const QString& sPlan, const QString& sDes, QTime tmBegin, QTime tmEnd);

    BOOL DeleteStage(QStage *pStage);

protected:
	// 挂起任务
	BOOL HungupTask();
	BOOL IsRunning()const ;

public:
	LONG		m_nFlag;		// 任务标志
	QTime		m_tmBegin;		// 任务周期开始生效时间
	QTime		m_tmEnd;		// 任务周期，预期结束时间
	QString		m_sPlan;		// 
	QString		m_sDes;			// 
	int			m_nIconID;		// 关联的图标

private:
	int			m_nID;
	QTime		m_tmCreate;		// 创建时间
	QTime		m_tmFinish;		// 实际完成/放弃的时间
	QTime		m_tmUpdate;		// 任务最后更新时间
	ENUM_PLAN_STATUS	m_eStatus;	// 任务状态

//	ENUM_TASK_RUNNING_STATUS	m_eRunning;	// 任务运行状态
//	QLongPlanTrackList*	m_pTrackList;
};

typedef std::vector<QPlan*> VecPlan;
typedef VecPlan::iterator VecPlanItr;

class QPlanMan 
{
    SINGLETON_ON_DESTRUCTOR(QPlanMan)
    {
        ClearMem();
    }

public:
    BOOL Startup();
   
    /** 添加一个计划
     *	return:
     *      TRUE        成功
     *	params:
     *		-[in]
     *          plan       计划数据
     *
    **/
    QPlan* AddPlan( __in QPlan plan );

    QPlan* GetPlan(int nID);

    BOOL DeletePlan(int nID);

    inline VecPlan* GetAllPlan()
    {
        return &m_plans;
    }

    // 获取当前进行中的计划数目
    int GetWorkingPlanNum();
protected:
    VecPlanItr FindPlan(int nID);

    inline VecPlanItr _End() { return m_plans.end(); }
    inline VecPlanItr _Begin() { return m_plans.begin(); }

    void ClearMem();
private:
    VecPlan     m_plans;
};