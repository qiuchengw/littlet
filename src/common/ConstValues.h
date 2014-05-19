#pragma once

#include <list>
#include <vector>

#ifndef INVALID_ID
#define INVALID_ID (-1)
#endif

enum
{
	// 刷新autotask, 
	// lParam 
	//		- autoTaskID
	MWND_CMD_REFRESHAUTOTASK = 1,
    // 切换应用程序模式, 
    // lParam 
    //		- ENUM_APP_MODE
    MWND_CMD_ENTERAPPMODE,

	// 选择autotask项目
	// lParam
	//		- AutoTask ID
	EVENTWND_CMD_SELECTAUTOTASK,

    // 用户选择跳过一次任务执行
    // lParam
    //		- AutoTask ID
    EVENTWND_NOTIFY_USERJUMPEVENTEXEC,

	// 对autotask操作
	// LPQMSG_PARAM
	//		-wParam		ENUM_TASK_OPERATION
	//		-lParam		AutoTask ID
	//		-lData 
	//				- 当wParam为DELETE的时候，此值为删除原因，ENUM_TASK_RUNNING_STATUS
	//				- 当wParam为ToggleRemind，此值为启用或者禁用值（BOOL）
	MWND_CMD_AUTOTASKOPERATION,
	// lParam 
	//		LPCWSTR -- messag
	MWND_CMD_SHOWMESSAGE,
	// 关机命令
	// lParam  
	//		
	MWND_CMD_SHUTDOWN,
	// 重启命令
	// LParam
	// - 
	MWND_CMD_REBOOT,
	// 休息一会儿，显示屏保
	// LParam
	// - 显示时长	单位秒（s）
	MWND_CMD_SHOWSCREENSAVER,

	// 显示自动任务的提示框
	// lParam 
	//		- LPTASK_REMINDER_PARAM
	MWND_CMD_SHOWAUTOTASKREMIND,
	// 显示自动任务的提示框
	// lParam 不使用
	MWND_CMD_SHOWOVERDUETASK,
	// 做任务
	// lParam
	//  - LPQMSG_PARAM
	//		wParam ENUM_TASK_DOWHAT
	//		lParam 根据wParam 而定
	MWND_CMD_DOWORK,
	// 显示面板
	// lParam
	//		-string: funcbar button ID，例如：L"btn_funcbar_tasks"
	MWND_CMD_SHOWPANE,
	// 更新广告
	// lParam，wParam 
	MWND_CMD_UPDATEAD,

    //////////////////////////////////////////////////////////////////////////
    // notifys
    // todo 任务新建、删除和状态更改
    // lParam
    //		- 
    MWND_NOTIFY_TODOTASKCHANGED,
    // autotask 任务新建、删除
    // lParam
    //		- 
    MWND_NOTIFY_EVENTNUMCHANGED,

    // autoTask 启动 
    //		- lParam	  TaskID
    MWND_NOTIFY_AUTOTASKSTART,
    // autoTask 删除 
    //		- lParam	  TaskID
    MWND_NOTIFY_AUTOTASKDELETE,
    // 过期的任务删除
    //		- lParam	  TaskID
    MWND_NOTIFY_AUTOTASKOVERDUE,
    // autoTask 编辑 
    //		- lParam	  TaskID
    MWND_NOTIFY_AUTOTASKEDIT,
    // autoTask 暂停 
    //		- lParam	  TaskID
    MWND_NOTIFY_AUTOTASKPAUSE,
    // autotask
    MWND_NOTIFY_AUTOTASKTOGGLEREMINDER,
    // auto Task 执行了，这时候可以删除掉它的提示了（如果有的话）
    // lParam 
    //		- TaskID
    MWND_NOTIFY_AUTOTASKFIRED,
    // 过期任务删除，重置，新增
    // lParam 
    //		- QAutoTask* 可以为 NULL
    MWND_NOTIFY_OVERDUETASKCHANGED,
    // 新建了一个自动任务
    // lParam  
    //		- QAutoTask*
    MWND_NOTIFY_AUTOTASKADDED,
    // 新建了一个目录
    // lParam
    //		- QCate*
    MWND_NOTIFY_CATEGORYADDED,
    // 更改了一个目录
    // lParam
    //		- QCate*
    MWND_NOTIFY_CATEGORYCHANGED,
    // 删除了一个目录
    // lParam
    //		- QCate*
    MWND_NOTIFY_CATEGORYDELETED,

    //////////////////////////////////////////////////////////////////////////
    // 计划数目变化
    // lParam
    //		- int   新的plan数目
    MWND_NOTIFY_PLANNUMCHANGED,
    //////////////////////////////////////////////////////////////////////////
    // QPictureLoader 的播放图片定时器激发
    // lParam   
    //          int     播放图像索引
    PICTRUELOADER_PLAYTIMER_FIRED,

    //////////////////////////////////////////////////////////////////////////
    /** plan选中通知
     *	params:
     *		-[in]
     *          lParam  QPlan* 可以为null
    **/
    VIEWPLAN_NOTIFY_PLANSELCHANGED,

    /** 添加stage通知
     *	params:
     *		-[in]
     *          lParam  QStage*
    **/
    VIEWPLAN_NOTIFY_STAGEADDED,    // 

    /** 删除stage通知
     *	params:
     *		-[in]
     *          lParam  nStageID
    **/
    VIEWPLAN_NOTIFY_STAGEDELETED,    
    
    /** 添加goal通知
     *	params:
     *		-[in]
     *          lParam  QStage*
     *              可能是NULL
    **/
    VIEWPLAN_NOTIFY_STAGESELCHANGED,

   /** 添加goal通知
     *	params:
     *		-[in]
     *          lParam  QGoal*
    **/
    VIEWPLAN_NOTIFY_GOALADDED,    
    
    /** 添加goal通知
     *	params:
     *		-[in]
     *          lParam  GOAL ID
    **/
    VIEWPLAN_NOTIFY_GOALDELETED,   
    
    /** 添加goal通知
     *	params:
     *		-[in]
     *          lParam  QGoal*
     *              可能是NULL
    **/
    VIEWPLAN_NOTIFY_GOALSELCHANGED,   

   /** 添加goalitem通知
     *	params:
     *		-[in]
     *          lParam  QGoalitem*
    **/
    VIEWPLAN_NOTIFY_GOALSUBITEMADDED,    
    
    /** 删除goalitme通知
     *	params:
     *		-[in]
     *          lParam  QGoal* (goalitem的父亲指针)
    **/
    VIEWPLAN_NOTIFY_GOALSUBITEMDELETED,   

    /** goalitem状态变化通知
     *	params:
     *		-[in]
     *          lParam  QGoalitem*
    **/
    VIEWPLAN_NOTIFY_GOALSUBITEMSTATUSCHANGED,   
};

enum ENUM_SSTYLE_TYPE
{
    STYLE_TYPE_NOTSET = 0,
    STYLE_TYPE_COLOR = 1,
    STYLE_TYPE_GRADIENT = 2,
    STYLE_TYPE_IMAGE = 3,
    STYLE_TYPE_IMAGEANDCOLOR = 4, 
};

// QRes 数据库中的图像的用途
enum ENUM_DBFILE_USAGE
{
    FILE_USAGE_DATA = 1,	// 单纯的内存数据
    FILE_USAGE_IMAGEFIRST,	// IMAGE
    FILE_USAGE_CONTACTS_PHOTO = FILE_USAGE_IMAGEFIRST,	// 联系人照片
    FILE_USAGE_CATEGORY_ICON,		// 目录关联图标
    FILE_USAGE_SIMPLESTYLE_IMAGE,
    FILE_USAGE_IMAGELAST = FILE_USAGE_CATEGORY_ICON,
};

//////////////////////////////////////////////////////////////////////////
// 任务描述
enum ENUM_TASK_DOWHAT
{
    TASK_DO_NOTSET = 0,	// 未设置	
    TASK_DO_LONGPLAN = 1,	// 长期的单项计划
    TASK_DO_REMIND = 2, // 提示信息
    TASK_DO_EXECPROG = 3,  // 执行程序
    TASK_DO_SYSSHUTDOWN = 4,  // 关机
    TASK_DO_SYSREBOOT = 5,  // 重启
    TASK_DO_BREAKAMOMENT = 6, // 休息一会儿
};

enum ENUM_TASK_OPERATION
{
    TASK_OP_START,		// 启动
    TASK_OP_PAUSE,		// 暂停
    TASK_OP_DELETE,		// 删除
    TASK_OP_EDIT,		// 编辑
    TASK_OP_JUMPOVEREXEC, // 跳过本次执行
    TASK_OP_TOGGLEREMIND, // 禁止提示
};

enum ENUM_TASK_EXECFLAG
{
    TASK_EXEC_NOTSET	= 0,	// 系统启动
    // 相对时间
    TASK_EXEC_AFTERSYSBOOT	= 0x00000001,	// 系统启动
    TASK_EXEC_AFTERTASKSTART = 0x00000002,	// 任务启动 
    TASK_EXEC_AFTERMINDERSTART = 0x00000004,// 本程序启动
    TASK_EXEC_AFTERPROGSTART = 0x00000008,// 外部程序启动
    TASK_EXEC_AFTERPROGEXIT = 0x00000010,// 外部程序退出
    // 绝对时间标记
    TASK_EXEC_ATDATE = 0x00010000,	// 绝对日期 2011/11/11
    TASK_EXEC_ATDAILY = 0x00020000,	// 每隔x天
    TASK_EXEC_ATMONTHDAY = 0x00040000,	// 每月的x号 
    TASK_EXEC_ATWEEKDAY = 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]

    //////////////////////////////////////////////////////////////////////////
    TASK_EXEC_RELATE_EXECANDTHEN = 0x01000000,	// 相对时间之后再次执行多次间隔

};

// 任务状态
enum ENUM_TASK_STATUS
{
    TASK_STATUS_NOTBEGIN	= 0,	// 还未进行到任务周期
    TASK_STATUS_PROCESSING	= 1,	// 进行中
    TASK_STATUS_PAUSE = 2,		// 暂停
    TASK_STATUS_ABORT = 3,		// 完成
    TASK_STATUS_FINISH = 4,		// 放弃
    TASK_STATUS_OVERDUE = 5,	// 任务过期，CurrentTime > EndTime
};

// 任务运行时状态
enum ENUM_TASK_RUNNING_STATUS
{
    TASK_RUNNING_STATUS_BADTIMER = -2,	// 不能解析timer表达式
    TASK_RUNNING_STATUS_APPERROR = -1,	// 应用程序出现了错误
    TASK_RUNNING_STATUS_OK = 0,	// 任务正常启动
    TASK_RUNNING_STATUS_NOTSTARTUP = 1,	// 任务还未启动
    TASK_RUNNING_STATUS_OVERDUE,	// 任务过期了
    TASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT,	// 需要下次机器重启，任务才执行
    TASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT,	// 需要程序重启，任务才执行
    TASK_RUNNING_STATUS_BASEDONEXETERNALPROG,	// 依赖的外部程序并没有运行
    //////////////////////////////////////////////////////////////////////////
    // 绝对时间
    TASK_RUNNING_STATUS_TIMENOTMATCH,	// 无可执行的时间匹配
    TASK_RUNNING_STATUS_NOCHANCETOEXEC,	// 虽然任务未过期，但是余下的时间里，任务都没有机会再执行了
};

// 任务优先级
enum ENUM_TASK_PRIORITY
{
    TASK_PRIORITY_LOWEST = 1,	// 最低
    TASK_PRIORITY_LOW = 2,		// 低
    TASK_PRIORITY_NOMAL = 3,	// 正常
    TASK_PRIORITY_HIGH = 4,		// 高
    TASK_PRIORITY_HIGHEST = 5,	// 最高优先级
};

enum ENUM_WORK_TYPE
{
    WORK_TYPE_AUTOTASKFIRED,
};

enum ENUM_SSTYLE_WHICH
{
	STYLE_WHICH_NOTSET = 0,
	STYLE_WHICH_BACKGROUND = 1,
	STYLE_WHICH_FOREGROUND = 2,
};

enum ENUM_CONTACT_FLAG
{
	CONTACT_FLAG_REMINDBIRTHDAY = 0x00000001,
};

enum ENUM_EVENT_TRACK
{
	TRACK_EVENT_PROGSTARUP,
	TRACK_EVENT_PROGEXIT,
	TRACK_EVENT_SYSTEMSTARTUP,
	TRACK_EVENT_SYSTEMEXIT,
};


enum ENUM_AUTOTASK_FLAG
{
    AUTOTASK_FLAG_PAUSED = 0x1,		// 暂停执行

    AUTOTASK_FLAG_CONTACTBIRTHDAY = 0x10000000,		// 联系人生日提示器
};

//////////////////////////////////////////////////////////////////////////
// 任务描述
enum ENUM_AUTOTASK_DOWHAT
{
    AUTOTASK_DO_NOTSET = 0,	// 未设置	
    AUTOTASK_DO_REMIND = 2, // 提示信息
    AUTOTASK_DO_EXECPROG = 3,  // 执行程序
    AUTOTASK_DO_SYSSHUTDOWN = 4,  // 关机
    AUTOTASK_DO_SYSREBOOT = 5,  // 重启
    AUTOTASK_DO_BREAKAMOMENT = 6, // 休息一会儿
};

enum ENUM_AUTOTASK_OPERATION
{
    AUTOTASK_OP_START,		// 启动
    AUTOTASK_OP_PAUSE,		// 暂停
    AUTOTASK_OP_DELETE,		// 删除
    AUTOTASK_OP_EDIT,		// 编辑
    AUTOTASK_OP_JUMPOVEREXEC, // 跳过本次执行
    AUTOTASK_OP_TOGGLEREMIND, // 禁止提示
};


// 一天的秒数
const double SECONDS_OF_DAY = 86400.0f;

enum ENUM_AUTOTASK_EXECFLAG
{
    AUTOTASK_EXEC_NOTSET	= 0,	// 系统启动
    // 相对时间
    AUTOTASK_EXEC_AFTERSYSBOOT	= 0x00000001,	// 系统启动
    AUTOTASK_EXEC_AFTERTASKSTART = 0x00000002,	// 任务启动 
    AUTOTASK_EXEC_AFTERMINDERSTART = 0x00000004,// 本程序启动
    AUTOTASK_EXEC_AFTERPROGSTART = 0x00000008,// 外部程序启动
    AUTOTASK_EXEC_AFTERPROGEXIT = 0x00000010,// 外部程序退出
    // 绝对时间标记
    AUTOTASK_EXEC_ATDATE = 0x00010000,	// 绝对日期 2011/11/11
    AUTOTASK_EXEC_ATDAILY = 0x00020000,	// 每隔x天
    AUTOTASK_EXEC_ATMONTHDAY = 0x00040000,	// 每月的x号 
    AUTOTASK_EXEC_ATWEEKDAY = 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
    AUTOTASK_EXEC_ATYEARDAY = 0x00100000,	// 每年的某个日期 2011~2020 的 [11/11]，可以有一个提前量。类似于生日提醒

    //////////////////////////////////////////////////////////////////////////
    AUTOTASK_EXEC_RELATE_EXECANDTHEN = 0x01000000,	// 相对时间之后再次执行多次间隔

};

// 任务运行时状态
enum ENUM_AUTOTASK_RUNNING_STATUS
{
    AUTOTASK_RUNNING_STATUS_BADTIMER = -2,	// 不能解析timer表达式
    AUTOTASK_RUNNING_STATUS_APPERROR = -1,	// 应用程序出现了错误
    AUTOTASK_RUNNING_STATUS_OK = 0,	// 任务正常启动
    AUTOTASK_RUNNING_STATUS_NOTSTARTUP = 1,	// 任务还未启动
    AUTOTASK_RUNNING_STATUS_PAUSED,	// 任务还未启动
    AUTOTASK_RUNNING_STATUS_OVERDUE,	// 任务过期了
    AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT,	// 需要下次机器重启，任务才执行
    AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT,	// 需要程序重启，任务才执行
    AUTOTASK_RUNNING_STATUS_BASEDONEXETERNALPROG,	// 依赖的外部程序并没有运行
    //////////////////////////////////////////////////////////////////////////
    // 绝对时间
    AUTOTASK_RUNNING_STATUS_TIMENOTMATCH,	// 无可执行的时间匹配
    AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC,	// 虽然任务未过期，但是余下的时间里，任务都没有机会再执行了
};


class QDBNote;
class QResDB;
class QCateMgr; 
class QAutoTaskMan; 
class QIdeaMgr;
class QTimerMan;
class QNoteTaskMgr;
class QUIMgr;
class QResData;

class QIdea;
typedef std::vector<QIdea*> IdeaVec;
typedef IdeaVec::iterator IdeaVecItr;

#define MAX_CATE_LEN 10		// 目录最大字符数

// 图标缩略图的宽度，单位px
#ifndef ICON_THUMB_WIDTH
#   define ICON_THUMB_WIDTH 80
#endif

// 图标缩略图的高度，单位px
#ifndef ICON_THUMB_HEIGHT
#   define ICON_THUMB_HEIGHT 80
#endif