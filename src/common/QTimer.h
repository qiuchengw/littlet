#pragma once

#include <list>
#include "exarray.h"
#include "time/QTime.h"
#include "ConstValues.h"
#include "QTimerListener.h"
#include "AppHelper.h"

// enum ENUM_AUTOTASK_EXECFLAG
// {
//     AUTOTASK_EXEC_NOTSET	= 0,	// 系统启动
//     // 相对时间
//     AUTOTASK_EXEC_AFTERSYSBOOT	= 0x00000001,	// 系统启动
//     AUTOTASK_EXEC_AFTERTASKSTART = 0x00000002,	// 任务启动
//     AUTOTASK_EXEC_AFTERMINDERSTART = 0x00000004,// 本程序启动
//     AUTOTASK_EXEC_AFTERPROGSTART = 0x00000008,// 外部程序启动
//     AUTOTASK_EXEC_AFTERPROGEXIT = 0x00000010,// 外部程序退出
//     // 绝对时间标记
//     AUTOTASK_EXEC_ATDATE = 0x00010000,	// 绝对日期 2011/11/11
//     AUTOTASK_EXEC_ATDAILY = 0x00020000,	// 每隔x天
//     AUTOTASK_EXEC_ATMONTHDAY = 0x00040000,	// 每月的x号
//     AUTOTASK_EXEC_ATWEEKDAY = 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
//
//     //////////////////////////////////////////////////////////////////////////
//     AUTOTASK_EXEC_RELATE_EXECANDTHEN = 0x01000000,	// 相对时间之后再次执行多次间隔
//
// };
//
// // 任务运行时状态
// enum ENUM_AUTOTASK_RUNNING_STATUS
// {
//     AUTOTASK_RUNNING_STATUS_BADTIMER = -2,	// 不能解析timer表达式
//     AUTOTASK_RUNNING_STATUS_APPERROR = -1,	// 应用程序出现了错误
//     AUTOTASK_RUNNING_STATUS_OK = 0,	// 任务正常启动
//     AUTOTASK_RUNNING_STATUS_NOTSTARTUP = 1,	// 任务还未启动
//     AUTOTASK_RUNNING_STATUS_PAUSED,	// 任务还未启动
//     AUTOTASK_RUNNING_STATUS_OVERDUE,	// 任务过期了
//     AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT,	// 需要下次机器重启，任务才执行
//     AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT,	// 需要程序重启，任务才执行
//     AUTOTASK_RUNNING_STATUS_BASEDONEXETERNALPROG,	// 依赖的外部程序并没有运行
//     //////////////////////////////////////////////////////////////////////////
//     // 绝对时间
//     AUTOTASK_RUNNING_STATUS_TIMENOTMATCH,	// 无可执行的时间匹配
//     AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC,	// 虽然任务未过期，但是余下的时间里，任务都没有机会再执行了
// };

//
// // 任务优先级
// enum ENUM_TASK_PRIORITY
// {
// 	TASK_PRIORITY_LOWEST = 1,	// 最低
// 	TASK_PRIORITY_LOW = 2,		// 低
// 	TASK_PRIORITY_NOMAL = 3,	// 正常
// 	TASK_PRIORITY_HIGH = 4,		// 高
// 	TASK_PRIORITY_HIGHEST = 5,	// 最高优先级
// };
//

/************************************************************************/
/*
相对时间表达式：
	R=1;P=20m;Q=22m;C=100;
	R: 相对于1（TASK_EXEC_AFTERSYSBOOT）
	P: 20minutes后执行任务 P[1s~24h]
	Q: 然后每个22minutes执行一次 Q[1s~24h]
	C: 执行100次后停止，C[1~86400]

绝对时间表达式：
	A=d;S=d;E=d;P=d;T=d1,d2,d3...;X=s;
	
    A: 执行时间标志ENUM_TASK_EXECFLAG
	
    S,E: 起止日期，任务的有效期

    P: 单个INT值
		间隔日期：A=TASK_EXEC_ATDAILY有效，单位天。 比如每隔3天 P=3;

    Q: 单个INT值
        提前天数：A=AUTOTASK_EXEC_ATYEARDAY有效，单位天，比如提前3天

	T: 执行时间点，一个或多个INT值，此值可解析为绝对时间
		比如一天中的13：30，14：30，16：20。。。

	X: 1个多个整数值，以‘,’分隔，具体含义由A决定
		1>A=TASK_EXEC_ATDATE, X为多个日期点，可解析为日期
		2>A=TASK_EXEC_ATDAILY, X为一个单个值，保存P间隔的天数
		3>A=TASK_EXEC_ATMONTHDAY, X一个整数值代表为月的日号
				此数值为哪一天可执行，使用位标示，共32位，最低位为1号..
		4>A=TASK_EXEC_ATWEEKDAY, X包含一个整数值，
				此值表示哪一天（星期日到星期六）可执行，使用位标识表示，ENUM_NTH_WEEKDAY
        5>A=AUTOTASK_EXEC_ATYEARDAY，X为一个整数值，代表 [月-日] 
                每年的某个日期 2011~2020 的 [11/11]，可以有一个提前量。类似于生日提醒
*/
/************************************************************************/

#define MONTH_LAST_DAY 32

enum ENUM_NTH_WEEK
{
    FIRST_WEEK = 0x1,
    SECOND_WEEK = 0x2,
    THIRD_WEEK = 0x4,
    FORTH_WEEK = 0x8,
    FIVETH_WEEK = 0x10,
};

enum ENUM_NTH_WEEKDAY
{
    SUNDAY = 0x1,
    MONDAY= 0x2,
    TUESDAY	= 0x4,
    WEDNESDAY = 0x8,
    THURSDAY = 0x10,
    FRIDAY = 0x20,
    SATURDAY = 0x40,
};

enum ENUM_TIMER_FLAG
{
    TIMER_FLAG_REMINDERDISABLED = 0x00000001,	// 启用提示定时器
};

CStdString GetRunningStatusDescription(ENUM_AUTOTASK_RUNNING_STATUS eStatus);
CStdString GetExecFlagText(ENUM_AUTOTASK_EXECFLAG eFlag);

class QTimerMan;
class QTimer
{
    friend class QTimerMan;
    enum ENUM_TIMER_TYPE
    {
        TIMER_TYPE_NOTSET = 0,
        TIMER_TYPE_RELATE,
        TIMER_TYPE_ABSOLUTE,
    };
public:
    QTimer(void);
    QTimer(int nID,QTime tmBegin,QTime tmEnd,
           LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXFiled);
    ~QTimer(void);

    // 更新数据
    BOOL Update(int nTimerID, QTime tmB, QTime tmEnd, LPCWSTR szExpWhen,
                LPCWSTR szExpRemind, LPCWSTR szExpXFiled);

    inline void SetEventHandler(QTimerEventHandlerBase* pHandler)
    {
        m_pTEH = pHandler;
    }

    // 执行任务前的提示表达式
    // 格式：
    // A=15[s|h|m];\nsound=path;\nmsg=message;\n
    // A=15[s|h|m] 执行任务前15[秒，分，时]提示
    // S=path; path 为声音文件路径 ,空为不播放声音
    // M=message; messsage 为自定义消息，空为无自定义消息
    static BOOL ParseRemindExp( const CStdString&sExp,__out int &nA,
                                __out WCHAR&cAUnit,__out CStdString&sSound,__out CStdString&sMsg );

    BOOL GetRemindString(__out CStdString& sReminderDes);
    //----------------------------------------------------------
    // status
    // 定时器是否已经启动
    BOOL IsStarted()const
    {
        return m_hTimerTask != NULL;
    }
    // nTaskID 如果不为无效id，那么如果停止失败，则尝试重启任务
    BOOL Stop(HANDLE hTimerQueue, int nTaskID = INVALID_ID);
    ENUM_AUTOTASK_RUNNING_STATUS Start( HANDLE hTimerQueue ,int nTaskID);
    // 跳过此次任务的执行，直接到下一次执行时间执行
    ENUM_AUTOTASK_RUNNING_STATUS JumpoverThisExec(HANDLE hTimerQueue,int nTaskID);
    QTime NextExecTime()const
    {
        ASSERT(IsStarted());
        return m_tmNextExec;
    }
    // 检查任务是否合法或过期，并不真正设定定时器
    ENUM_AUTOTASK_RUNNING_STATUS TestStart( );
    BOOL IsReminderEnabled()const
    {
        return !(m_dwTimerFlag & TIMER_FLAG_REMINDERDISABLED);
    }
    BOOL EnableReminder( HANDLE hTimerQueue,int nTaskID,BOOL bEnable=TRUE );

    //----------------------------------------------------------
    BOOL IsTheYear(int y)const;
    int ID()const
    {
        return m_nID;
    }
    inline BOOL IsValid()const
    {
        return m_nID != INVALID_ID;
    }
    inline BOOL IsRelateTimer()
    {
        return m_eTimerType == TIMER_TYPE_RELATE;
    }
    inline ENUM_AUTOTASK_EXECFLAG GetExecFlag()const
    {
        return eflag_exec_;
    }

    // tmTest之后的下次执行时间
    // tmTest 将被调整，毫秒级别将会忽略置为0
    ENUM_AUTOTASK_RUNNING_STATUS GetNextExecTimeFrom( __inout QTime& tmTest, __out QTime& tmExec );
    BOOL SetLifeTime(QTime tmLifeBegin,QTime tmLifeEnd);
    QTime GetLifeBegin()
    {
        return m_tmLifeBegin;
    }
    QTime GetLifeEnd()
    {
        return m_tmLifeEnd;
    }

    BOOL SetExp(const CStdString& sExp);

    int GetTimerID()const
    {
        return m_nID;
    }

protected:
    // 从时间tmBegin开始设定任务，
    // 这个函数只能在QTimer内部调用
    ENUM_AUTOTASK_RUNNING_STATUS StartFrom(QTime &tmBegin,HANDLE hTimerQueue ,int nTaskID);

public: // relate
    DWORD GetExecSpan()const
    {
        return m_dwSpan;
    }
    // 解析为秒返回
    DWORD GetExecSpanSeconds()const ;
    WCHAR GetExecSpanUnit()const
    {
        return m_cSpanUnit;
    }
    DWORD GetExecSpan2()const
    {
        return m_dwSpan2;
    }
    DWORD GetExecSpanSeconds2()const ;
    WCHAR GetExecSpanUnit2()const
    {
        return m_cSpanUnit2;
    }
    DWORD GetExecCount()const
    {
        return m_iExecCount;
    }
    // 执行第一次后是否再间隔执行
    BOOL IsExecSpan2()const
    {
        return (m_dwSpan2>0);
    }
    BOOL IsExecCount()const
    {
        return m_iExecCount > 0;
    }

public: // abs
    /** 绝对时间的，执行时间点
     *	return:
     *      时间点个数  >= 1
     *      如果错误，== 0
     *	params:
     *		-[out]
     *      vTimes      时间点
    **/
    // 执行时间点
    int GetExecTimeSpot(__out std::vector<QTime>& vTimes);
    DWORD GetExecDate( );
    CStdString GetXFiled()const
    {
        return m_sXFiledExp;
    }
    CStdString GetRemindExp()const
    {
        return m_sExpRemind;
    }
    CStdString GetWhenExp()const
    {
        return m_sExpWhen;
    }
    BOOL GetWhenDoString(CStdString &sWhenDoDes);

protected:
    HANDLE GetTimerHandle()
    {
        return m_hTimerTask;
    }
    ENUM_AUTOTASK_RUNNING_STATUS AbsTime_NextExecDate(__inout DWORD& dwDate);
    ENUM_AUTOTASK_RUNNING_STATUS _AbsTime_NextRightTimeFrom(__in const QTime&tmTest,
            __in const QTime& tmExec,__inout DWORD &dwNextExecTime);
    // 这两个函数用于检测绝对时间执行
    // BOOL IsTheDate( const QTime& d );
//     BOOL IsTheTime(WORD wTime);
//     BOOL IsTheTime(const QTime& t);
    // 这个函数用于相对时间执行的辅助函数
    // 相对于tmX，找出大于等于tmTest的执行时间，放入到tmExec中
    ENUM_AUTOTASK_RUNNING_STATUS _RelateTime_CheckWith( const QTime& tmX,
            const QTime& tmTest,__out QTime& tmExec);

protected:
    inline BOOL IsInArrayX(DWORD dw)const;
    void ResetAllFiled();

    BOOL ParseExp(const CStdString& sExp);
    BOOL ParseRelateExp(const CStdString& sExp);
    BOOL ParseAbsoluteExp(const CStdString& sExp);

    // sExp: r=1;b=2;c=3
    // result:
    //	sExp = b=2;c=3;
    //	sProp = r;
    //	sValue = 1;
    static BOOL _Parse(__inout CStdString&sExp ,__out WCHAR& sProp, __out CStdString& sValue);
    // 200[s|m|h]
    // 200m = 200 * 60s;  cUnit = m;
    // 200h = 200 * 3600s	cUnit = h;
    // 200s = 200s;		cUnit = s;
    BOOL _ParseSpanTime(__in const CStdString &sExp , __out WCHAR& cUnit, __out DWORD& dwSpan );

    // 2,4,8,9
    // 以 英文‘,’ 分隔的数字字符，转换到整数值
    BOOL _ParseToIntArray( __inout CStdString& sExp,__out IntArray & ar );

private:
    // 启动定时器的时候调用，以设定提前提示任务
    // tmExec 任务的执行时间
    BOOL SetRemindTimer( HANDLE hTimerQueue ,int nTaskID,const QTime& tmExec);
    // 设置提示定时器表达式
    BOOL SetRemindExp(LPCWSTR pszRmdExp);
private:
    int				m_nID;	// 数据库中的id
    CStdString			m_sExpWhen;		// 何时做
    CStdString			m_sExpRemind;	// 何时提醒
    ENUM_AUTOTASK_EXECFLAG	eflag_exec_;
    ENUM_TIMER_TYPE		m_eTimerType;		// 相对时间还是绝对时间任务
    QTime			m_tmLifeBegin;	// 任务的生效时间
    QTime			m_tmLifeEnd;	// 任务过期时间
    CStdString			m_sXFiledExp;	//
    HANDLE			m_hTimerTask;	// 队列定时器句柄
    HANDLE			m_hTimerReminder;	// 提前提示定时器
    QTime			m_tmNextExec; // 下次执行时间
    DWORD			m_dwTimerFlag;
    TASK_REMINDER_PARAM   m_stTRP; // 任务提示的数据
    QTimerEventHandlerBase*   m_pTEH;  // QTimer发生的事件交给它处理

private:	// Relate;
    // 相对于ENUM_TASK_EXECFLAG所指示的任务的
    // 发生后多少时间执行第一次任务，单位换算为秒
    // 此时间限制为24小时之内
    DWORD			m_dwSpan;		// 间隔时间或间隔日期
    WCHAR			m_cSpanUnit;	// 原始单位s/m/h
    // 相对于第一次任务执行后多少时间执行
    DWORD			m_dwSpan2;	// 第二个时间间隔
    WCHAR			m_cSpanUnit2;
    int				m_iExecCount;	// 第一次执行后，再执行的次数

private: // absolute
    //----------------------------------------
    IntArray		m_arX;		// 执行日期点，需配合ENUM_TASK_EXECFLAG才能确定其值的意义
    IntArray		m_arTime;	// 执行时间点，
    //-----------------------------------------------
    // 在一天中的 [03:00 ~ 20:00] 每隔 30m 执行一次
    WORD			m_wTimeBegin;	// 执行时间
    WORD			m_wTimeEnd;
};
