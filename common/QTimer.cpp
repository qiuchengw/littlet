#pragma warning(disable:4244 4018)

#include "QTimer.h"
#include "QHelper.h"
#include "ProcessMan.h"
#include "QTimerMan.h"

static QTimerEventHandler       g_DefaultTimerEH;

// const WORD TIME_000000 = QTime(2000,)::MakeTime(0,0,0);
// const WORD TIME_235959 = QTime::MakeTime(23,59,59);
#ifdef _DEBUG
	void TraceReminderSet(const QTime& tmRemind)
	{
		QTime tmNow = QTime::GetCurrentTime();
		QString str;
		str.Format(L"【%s】 设定了提醒定时器，【%s】 时间执行\n",
			tmNow.Format(L"%c"),tmRemind.Format(L"%c"));
		TRACE(str);
	}
#endif

#ifdef _DEBUG
#	define TRACE_REMIND(x) TraceReminderSet(x);
#else
#	define TRACE_REMIND(x) 
#endif

VOID CALLBACK TaskCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired);
// 自动任务提前提示的的回调函数
VOID CALLBACK TaskRemindCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired);

QString GetRunningStatusDescription( ENUM_AUTOTASK_RUNNING_STATUS eStatus )
{
	switch(eStatus)
	{
	case AUTOTASK_RUNNING_STATUS_BADTIMER://-2://	// 不能解析timer表达式
        return L"无效的定时器";
	case AUTOTASK_RUNNING_STATUS_APPERROR://-1://	// 应用程序出现了错误
        return L"应用程序错误";
	case AUTOTASK_RUNNING_STATUS_OK://0://	// 任务正常启动
        return L"一切正常";
	case AUTOTASK_RUNNING_STATUS_NOTSTARTUP://1://	// 任务还未启动
        return L"任务还未启动";
	case AUTOTASK_RUNNING_STATUS_PAUSED: // 暂停中
        return L"任务暂停";
	case AUTOTASK_RUNNING_STATUS_OVERDUE://	// 任务过期了
        return L"任务过期";
	case AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT://	// 需要下次机器重启，任务才执行
        return L"下次开机执行";
	case AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT:	// 需要程序重启，任务才执行
        return L"下次启动程序运行";
	case AUTOTASK_RUNNING_STATUS_BASEDONEXETERNALPROG://	// 依赖的外部程序并没有运行
        return L"依赖的外部程序并没有运行";
		//////////////////////////////////////////////////////////////////////////
		// 绝对时间
	case AUTOTASK_RUNNING_STATUS_TIMENOTMATCH://	// 无可执行的时间匹配
        return L"无可执行的时间匹配";
	case AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC://	// 虽然任务未过期，但是余下的时间里，任务都没有机会再执行了
        return L"任务没有机会再执行";
	default: return L"未知标志";
	}
}

QString GetExecFlagText( ENUM_AUTOTASK_EXECFLAG eFlag )
{
	switch (eFlag)
	{
	case AUTOTASK_EXEC_NOTSET: // 0,	// 未设置
		return L"未设置";
		// 相对时间
	case AUTOTASK_EXEC_AFTERSYSBOOT: // 0x00000001,	// 系统启动
		return L"系统启动";
	case AUTOTASK_EXEC_AFTERTASKSTART : // 0x00000002,	// 任务启动 
        return L"任务启动后";
	case AUTOTASK_EXEC_AFTERMINDERSTART : // 0x00000004,// 本程序启动
        return L"本程序启动";
	case AUTOTASK_EXEC_AFTERPROGSTART : // 0x00000008,// 外部程序启动
        return L"相对于外部程序启动";
	case AUTOTASK_EXEC_AFTERPROGEXIT : // 0x00000010,// 外部程序退出
        return L"相对于外部程序退出";
		// 绝对时间标记
	case AUTOTASK_EXEC_ATDATE : // 0x00010000,	// 绝对日期 2011/11/11
        return L"日期";
	case AUTOTASK_EXEC_ATDAILY : // 0x00020000,	// 每隔x天
        return L"每隔x天";
	case AUTOTASK_EXEC_ATMONTHDAY : // 0x00040000,	// 每月的x号 
        return L"每月的x号";
	case AUTOTASK_EXEC_ATWEEKDAY : // 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
        return L"每月的x周";

		//////////////////////////////////////////////////////////////////////////
	case AUTOTASK_EXEC_RELATE_EXECANDTHEN : // 0x01000000,	// 相对时间之后再次执行多次间隔
        return L"相对时间之后再次执行多次间隔";
	}
    ASSERT(FALSE);
	return L"GetExecFlagText->未知参数";
}

QTimer::QTimer()
{
	ResetAllFiled();
}

QTimer::QTimer( int nID,QTime tmBegin,QTime tmEnd, 
	LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXFiled )
{
	ResetAllFiled();

    VERIFY(Update(nID, tmBegin, tmEnd, szWhen, szReminder, szXFiled));
}

QTimer::~QTimer(void)
{
    ASSERT(!IsStarted());
}

// 此函数执行流程让人容易迷糊。
// 可以在纸上画一条横坐标，标出来3个时间点来清晰思路：提示时间，任务执行时间，现在时间
// 移动现在时间就明白啦
BOOL QTimer::SetRemindTimer( HANDLE hTimerQueue ,int nTaskID,const QTime& tmExec)
{
    QTime tmNow = QTime::GetCurrentTime();
    if ((tmExec - tmNow).GetTotalSeconds() < 5) 
    {
        // 距离任务执行时间太短了，就不提示了。
        return FALSE;
    }

    WCHAR cUnit;
    int nA;
    QString sSound,sMsg;
    if (!QTimer::ParseRemindExp(m_sExpRemind,nA,cUnit,sSound,sMsg))
    {
        return FALSE;
    }

    m_stTRP.nSeconds = QHelper::HowManySeconds(nA,cUnit);
    ASSERT(m_stTRP.nSeconds > 0);
    m_stTRP.sSound = sSound;
    m_stTRP.sMsg = sMsg;
    m_stTRP.nTaskID = nTaskID;
    m_stTRP.tmExec = tmExec;

    // 何时应该提示时间
    QTime tmRemind = tmExec - QTimeSpan( (m_stTRP.nSeconds) / SECONDS_OF_DAY );
    if ( tmNow > tmRemind )
    {   // 已经过了提示时间了
        // 应该立即提示
        m_stTRP.nSeconds = (tmExec - tmNow).GetTotalSeconds();   // 距离任务执行时间有多少秒
        if (NULL != m_pTEH)
        {
            m_pTEH->OnTimerReminderSetted(&m_stTRP);
        }
        return TRUE;
    }

    // 还没有到提示时间 
    // 具体还有多少秒去提示呢？
    DWORD dwSecToRemind = (tmRemind - tmNow).GetTotalSeconds();
    if (dwSecToRemind < 5)
    {   // 距离提示时间还有5秒，太短啦，直接显示提示框吧
        // 但是倒计时应该加上这段时间
        m_stTRP.nSeconds += dwSecToRemind;
        if (NULL != m_pTEH)
        {
            m_pTEH->OnTimerReminderSetted(&m_stTRP);
        }
        return TRUE;
    }

    m_stTRP.nSeconds = QHelper::HowManySeconds(nA,cUnit);
    // 如果距离该提示的时间还足够长，那么需要创建一个定时器回调函数
    // 当回调发生时再向窗口发消息，通知显示提示窗口
    // 创建单次定时器来执行提示回调
    return CreateTimerQueueTimer(&m_hTimerReminder,hTimerQueue,
            TaskRemindCallback,(PVOID)&m_stTRP, dwSecToRemind * 1000,
            0, WT_EXECUTEDEFAULT); // 单次定时器
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::Start(HANDLE hTimerQueue ,int nTaskID)
{
	return StartFrom(QTime::GetCurrentTime(), hTimerQueue, nTaskID);
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::StartFrom(QTime &tmBegin,HANDLE hTimerQueue ,int nTaskID)
{
	if (IsStarted())
	{
		return AUTOTASK_RUNNING_STATUS_OK;
	}
	double lHowLongToExec;
	ENUM_AUTOTASK_RUNNING_STATUS eStatus;
	while (true)
	{
		if ((eStatus = GetNextExecTimeFrom(tmBegin,m_tmNextExec))
			!= AUTOTASK_RUNNING_STATUS_OK)
		{
			break;
		}
		lHowLongToExec = (m_tmNextExec-QTime::GetCurrentTime()).GetTotalSeconds(); 
		if (lHowLongToExec <= 3.0f) // 如果离执行时间只有3秒，则查找下次执行时间点
		{
			tmBegin = m_tmNextExec + QTimeSpan( 1 / SECONDS_OF_DAY );
			continue;
		}
		BOOL bOK = CreateTimerQueueTimer(&m_hTimerTask, hTimerQueue,TaskCallback,
			(PVOID)nTaskID,lHowLongToExec * 1000,0,WT_EXECUTEDEFAULT);
		if (!bOK)
		{
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}

		// 提前提醒
		if (IsReminderEnabled())
		{
			SetRemindTimer(hTimerQueue,nTaskID,m_tmNextExec);
		}
		break;
	}
	return eStatus;
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::TestStart()
{
	double lHowLongToExec;
	QTime tmBegin = QTime::GetCurrentTime();
	ENUM_AUTOTASK_RUNNING_STATUS eStatus;
	while (true)
	{
		if ((eStatus = GetNextExecTimeFrom(tmBegin,m_tmNextExec))
			!= AUTOTASK_RUNNING_STATUS_OK)
		{
			break;
		}
		lHowLongToExec = (m_tmNextExec-tmBegin).GetTotalSeconds(); 
		if (lHowLongToExec <= 3.0f) // 如果离执行时间只有3秒，则查找下次执行时间点
		{
			tmBegin = m_tmNextExec + QTimeSpan( 1 / SECONDS_OF_DAY );
			continue;
		}
		break;
	}
	return eStatus;
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::JumpoverThisExec(HANDLE hTimerQueue,int nTaskID)
{
	if (!Stop(hTimerQueue, nTaskID))
	{
		return AUTOTASK_RUNNING_STATUS_APPERROR;
	}
    // 从当前执行任务的下一个30秒开始计算下次的执行时间
	return StartFrom(m_tmNextExec + QTimeSpan((DWORD)30),hTimerQueue,nTaskID);

    //return AUTOTASK_RUNNING_STATUS_OK;
}

BOOL QTimer::Stop(HANDLE hTimerQueue, int nTaskID)
{
	if (IsStarted())
    {
        BOOL bHasReminder = FALSE;
        // Sleep(100);	// 把线程时间让出来，给定时器函数执行完成。
        if (m_hTimerReminder != NULL)
        {
            if (!DeleteTimerQueueTimer(hTimerQueue,m_hTimerReminder,INVALID_HANDLE_VALUE))
            {
                ASSERT(FALSE);
                return FALSE;
            }
            m_hTimerReminder = NULL;
            bHasReminder = TRUE;
        }
        if (NULL != m_hTimerTask)
        {
            // 等待，直到删除
            if (!DeleteTimerQueueTimer(hTimerQueue,m_hTimerTask,INVALID_HANDLE_VALUE))
            {
                if (bHasReminder && (INVALID_ID != nTaskID))
                {
                    SetRemindTimer(hTimerQueue, nTaskID, NextExecTime());
                }
                ASSERT(FALSE);
                return FALSE;
            }
            m_hTimerTask = NULL;
        }
    }
    return TRUE;
}

BOOL QTimer::SetLifeTime(QTime tmLifeBegin,QTime tmLifeEnd)
{
    if (tmLifeEnd <= tmLifeBegin)
    {
        return FALSE;
    }
	m_tmLifeBegin = tmLifeBegin;
	m_tmLifeEnd = tmLifeEnd;
	return TRUE;
}

void QTimer::ResetAllFiled()
{
	m_hTimerTask = NULL;
	m_hTimerReminder = NULL;
    m_pTEH = &g_DefaultTimerEH;

	m_nID = INVALID_ID;
	m_eExecFlag = AUTOTASK_EXEC_NOTSET;
	m_eTimerType = TIMER_TYPE_NOTSET;
	m_dwTimerFlag = 0;

	m_sXFiledExp = L"";

	m_dwSpan = 0;
	m_cSpanUnit = L'';
	m_dwSpan2 = 0;	// 第二个时间间隔
	m_cSpanUnit2 = L''; 
	m_iExecCount = 0;	// 执行次数

	m_arX.clear();
	m_arTime.clear();
	m_wTimeBegin = 0;
	m_wTimeEnd = 0;
	m_dwSpan = 0; // 时间间隔 ，单位s
}

BOOL QTimer::ParseExp( const QString& sExp )
{
	if (sExp.IsEmpty())
		return FALSE;
	switch(sExp.GetAt(0))
	{
	case L'R':
		return ParseRelateExp(sExp);
	case L'A':
		return ParseAbsoluteExp(sExp);
	}
	return FALSE;
}

BOOL QTimer::ParseAbsoluteExp( const QString& sExp )
{
	if (sExp.IsEmpty())
		return FALSE;
	QString sExpTest = sExp, sValue;
	WCHAR cProp;
	while (!sExpTest.IsEmpty())
	{
		if (!_Parse(sExpTest,cProp,sValue))
			return FALSE;
		switch (cProp)
		{
		case L'A':
			{
				m_eTimerType = TIMER_TYPE_ABSOLUTE;
				m_eExecFlag = (ENUM_AUTOTASK_EXECFLAG)StrToInt(sValue);
				break;
			}
		case L'X':
			{
				if (!_ParseToIntArray(sValue,m_arX))
					return FALSE;
				std::stable_sort(_BeginItr(m_arX),_EndItr(m_arX));
				break;
			}
		case L'S': // 时间
			{
				m_wTimeBegin = StrToInt(sValue);
				break;
			}
		case L'E': // 时间
			{
				m_wTimeEnd = StrToInt(sValue);
				break;
			}
		case L'P': // 间隔时间执行
			{
				if (!_ParseToIntArray(sValue,m_arX))
					return FALSE;
				if (m_arX[0] < 1)
					return FALSE;
				break;
			}
		case L'T': // 时间点执行
			{
				if (!_ParseToIntArray(sValue,m_arTime))
					return FALSE;
// #ifdef _DEBUG
// 				QTime tmT;
// 				for (int i = 0; i < m_arTime.size(); i++)
// 				{
// 					tmT = QTime::ParseTime(m_arTime[i]);
// 					TRACE(tmT.Format(L"%H:%M:%S\n"));
// 				}
// #endif
				// 从小到大排序
				std::stable_sort(_BeginItr(m_arTime),_EndItr(m_arTime));
				break;
			}
		default: return FALSE;
		}
	}
	return TRUE;
}

BOOL QTimer::ParseRemindExp( const QString&sExp,__out int &nA, 
    __out WCHAR&cAUnit,__out QString&sSound,__out QString&sMsg )
{
    WCHAR cProp;
    QString sPart,sValue,sTemp = sExp;
    int idx;
    while (true)
    {
        idx = sTemp.Find(L";\n");
        if (-1 == idx)
            return FALSE;
        sPart = sTemp.Left(idx + 1);
        sTemp = sTemp.Mid(idx + 2);
        if (!_Parse(sPart,cProp,sValue))
            return FALSE;
        switch (cProp)
        {
        case L'A':
            {
                if ( !QHelper::ParseUnitTime(sValue, nA, cAUnit) || (nA <= 0))
                    return FALSE;
                break;
            }
        case L'S':	// sound
            {
                sSound = sValue;
                break;
            }
        case L'M':
            {
                sMsg = sValue;
                break;
            }
        default:return FALSE;
        }
        if (sTemp.IsEmpty())
            break;
    }
    return TRUE;
}

BOOL QTimer::ParseRelateExp( const QString& sExp )
{
	if (sExp.IsEmpty())
		return FALSE;
	QString sExpTest = sExp;
	WCHAR cProp;
	QString sValue;
	while (!sExpTest.IsEmpty())
	{
		if (!_Parse(sExpTest,cProp,sValue))
			return FALSE;
		switch (cProp)
		{
		case L'R':
			{
				m_eTimerType = TIMER_TYPE_RELATE;
				m_eExecFlag = (ENUM_AUTOTASK_EXECFLAG)StrToInt(sValue);
				break;
			}
		case L'P':
			{
				if (!_ParseSpanTime(sValue,m_cSpanUnit,m_dwSpan))
					return FALSE;
				break;
			}
		case L'Q': // 第二个时间间隔
			{
				if (!_ParseSpanTime(sValue,m_cSpanUnit2,m_dwSpan2))
					return FALSE;
				break;
			}
		case L'C': // 执行次数
			{
				m_iExecCount = StrToInt(sValue);
				break;
			}
		default: return FALSE;
		}
	}
	return TRUE;
}

BOOL QTimer::_Parse( __inout QString&sExp ,
	__out WCHAR& cProp, __out QString& sValue )
{
	int idx = sExp.Find(L'=');
	if ((-1 == idx) || (idx != 1) || ((idx = sExp.Find(L';')) == -1))
		return FALSE;
	cProp = sExp[0];
	sValue = sExp.Mid(2,idx-2);
	sExp = sExp.Mid(idx + 1);	
	return TRUE;
}

BOOL QTimer::_ParseSpanTime( __in const QString &sExp , 
		__out WCHAR& cUnit, __out DWORD& dwSpan )
{
	int len = sExp.GetLength();
	cUnit = tolower(sExp.back());
	dwSpan = StrToInt(sExp.Mid(0,len-1));
	switch(cUnit)
	{
	case L'm':case L'M':case L's':case L'S':case L'H':case L'h':
		return TRUE;
	}
	ASSERT(FALSE);
	return FALSE;
}

BOOL QTimer::_ParseToIntArray( __inout QString& sExp,__out IntArray & ar )
{
	ar.clear();
	int idx;
	while (!sExp.IsEmpty())
	{
		idx = sExp.Find(L',');
		if (idx != -1)
		{
			ar.push_back(StrToInt(sExp.Mid(0,idx)));
			sExp = sExp.Mid(idx+1);
		}
		else
		{
			ar.push_back(StrToInt(sExp));
			break;
		}
	}
	return ar.size();
}

BOOL QTimer::IsTheDate( const QTime& d )
{
	ASSERT(!IsRelateTimer());
	
	switch (m_eExecFlag)
	{
	case AUTOTASK_EXEC_ATDATE:	// = 0x00010000,	// 绝对日期 2011/11/11
		{
			return m_arX.contain(d.MakeDate());
		}
	case AUTOTASK_EXEC_ATDAILY:	// = 0x00020000,	// 每x天,
		{
			if (m_arX.size())
			{
				QTime t2 = d;
				t2.SetTime(m_tmLifeBegin.GetHour(),m_tmLifeBegin.GetMinute(),0);
				// 余数为0则执行
				return !((DWORD)((t2 - m_tmLifeBegin).GetTotalDays()) % (m_arX[0])); 
			}
			ASSERT(FALSE); return FALSE;
		}
	case AUTOTASK_EXEC_ATMONTHDAY:	// = 0x00040000,	// 每月的x号 
		{	
			if (m_arX.size())
			{
				return (m_arX[0] & (0x1<<(d.GetDay()-1)));
			}
			ASSERT(FALSE);return FALSE;
		}
	case AUTOTASK_EXEC_ATWEEKDAY:	// = 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
		{	
			if (m_arX.size())
			{// 1 - sunday , 7 - saturday
				return ((m_arX[0]) & (0x1<<(d.GetDayOfWeek()-1)));
			}
			ASSERT(FALSE); return FALSE;
		}
	default: { ASSERT(FALSE);  return FALSE; }
	}
	return FALSE;
}

BOOL QTimer::IsTheTime(WORD wTime)
{
	ASSERT(!IsRelateTimer());
// 	if ((m_wTimeBegin > wTime) || (m_wTimeEnd < wTime) )
// 		return FALSE;
	return m_arTime.contain(wTime);
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::_RelateTime_CheckWith(
	const QTime& tmX,const QTime& tmTest,__out QTime& tmExec)
{
	if (tmTest >= m_tmLifeEnd)
	{
		return AUTOTASK_RUNNING_STATUS_OVERDUE;
	}
	QTime tmFirstExec = tmX + QTimeSpan(GetExecSpanSeconds()/SECONDS_OF_DAY);
	if (tmFirstExec <= tmTest) // 等于也算是错过了执行时间
	{	// 错过了第一次执行时间
		if (!IsExecSpan2())
		{ // 错过第一次执行时间，并且非多次执行
			if (AUTOTASK_EXEC_AFTERSYSBOOT == m_eExecFlag)
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT; // 等待系统重启
			else if (AUTOTASK_EXEC_AFTERMINDERSTART == m_eExecFlag) 
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT; // 等待程序重启
			else // 非多次可执行，过期
				return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC; 
		}
		// 多次间隔执行
		// 自从第一次可执行时间到tmTest已经过去了多长时间
		double dTotalSeconds = (tmTest - tmFirstExec).GetTotalSeconds(); // 
		// 在过去的这么长时间里可以执行多少次？
		DWORD dwExec = dTotalSeconds / GetExecSpanSeconds2(); //执行次数
		if (IsExecCount() && (dwExec >= m_iExecCount))
		{ // 可执行次数已经超过了总共需要执行的次数
			if (AUTOTASK_EXEC_AFTERSYSBOOT == m_eExecFlag)
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT; // 等待系统重启
			else if (AUTOTASK_EXEC_AFTERMINDERSTART == m_eExecFlag) 
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT; // 等待程序重启
			else // 非多次可执行，过期
				return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC; 
		}
		else
		{ // 可执行次数还没有超过了总共需要执行的次数
			tmExec = tmFirstExec + QTimeSpan(((dwExec + 1) * GetExecSpanSeconds2()) / SECONDS_OF_DAY);
			if (tmExec >= m_tmLifeEnd)
			{
				return AUTOTASK_RUNNING_STATUS_OVERDUE;
			}
			return AUTOTASK_RUNNING_STATUS_OK;
		}
	}
	else
	{
		tmExec = tmFirstExec;
		return AUTOTASK_RUNNING_STATUS_OK;
	}
}

// tmTest 将被调整，毫秒级别将会忽略置为0
ENUM_AUTOTASK_RUNNING_STATUS QTimer::GetNextExecTimeFrom( 
	__inout QTime& tmTest,
	__out QTime& tmExec )
{
	if (tmTest >= m_tmLifeEnd)
		return AUTOTASK_RUNNING_STATUS_OVERDUE;	// 过期
	switch (m_eExecFlag)
	{
	//////////////////////////////////////////////////////////////////////////
	// 相对时间
	case AUTOTASK_EXEC_AFTERSYSBOOT:	//= 0x00000001,	// 系统启动
		{
			return _RelateTime_CheckWith(QProcessMan::GetSystemStartupTime(),tmTest,tmExec);
		}
	case AUTOTASK_EXEC_AFTERMINDERSTART:	// = 0x00000004,// 本程序启动
		{
			return _RelateTime_CheckWith(QProcessMan::GetCurrentProcessStartupTime(),tmTest,tmExec);
		}
/*	case TASK_EXEC_AFTERPROGSTART:	// = 0x00000008,// 外部程序启动
		{
			QTime tmProgStart;
			if (QProcessMgr::IsExeRun(m_sXFiledExp,tmProgStart))
			{
				return _RelateTime_CheckWith(tmProgStart,tmTest,tmExec);
			}
			return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
		}
	case TASK_EXEC_AFTERPROGEXIT:	// = 0x00000010,// 外部程序退出
		{ 
			return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
		}
*/	case AUTOTASK_EXEC_AFTERTASKSTART:	// = 0x00000002,	// 任务启动 
		{
			return _RelateTime_CheckWith(m_tmLifeBegin,tmTest,tmExec);
		}
	//////////////////////////////////////////////////////////////////////////
	// 绝对时间
	case AUTOTASK_EXEC_ATDATE:	// = 0x00010000,	// 绝对日期 2011/11/11
	case AUTOTASK_EXEC_ATDAILY:	// = 0x00020000,	// 每x天,
	case AUTOTASK_EXEC_ATMONTHDAY:	// = 0x00040000,	// 每月的x号 
	case AUTOTASK_EXEC_ATWEEKDAY:	// = 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
		{
			DWORD dwNextExecDate,dwNextExecTime;
			ENUM_AUTOTASK_RUNNING_STATUS eStatus;
			QTime tmTempTest = tmExec = tmTest;
			while(true)
			{
				dwNextExecDate = tmExec.MakeDate();
				eStatus = AbsTime_NextExecDate(dwNextExecDate); // 执行日期
				if (AUTOTASK_RUNNING_STATUS_OK == eStatus)
				{
					tmExec = QTime::ParseDate(dwNextExecDate);
					eStatus = _AbsTime_NextRightTimeFrom(tmTempTest,tmExec,dwNextExecTime);
					if (AUTOTASK_RUNNING_STATUS_OK == eStatus)
					{ // 执行时间
						tmExec = QTime::CombineTime(dwNextExecDate,dwNextExecTime);
						if (tmExec > m_tmLifeEnd) // 必须检查合成的时间是否超过了任务周期
							return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
						else
							return AUTOTASK_RUNNING_STATUS_OK;
					}
					else if (AUTOTASK_RUNNING_STATUS_TIMENOTMATCH == eStatus)
					{ // 说明当前测试的日期是不可能执行的，只能比今天晚的日期执行
						// 这时时间可以是最小的
						tmExec += QTimeSpan(1,0,0,0);
						tmExec.SetTime(0,0,0);
						tmTempTest.SetTime(0,0,0);
						continue;
					}
					else
					{
						return eStatus;
					}
				}
				return eStatus;
			}
		}
	default:
		{
			ASSERT(FALSE);
			return AUTOTASK_RUNNING_STATUS_BADTIMER;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// AtDate
ENUM_AUTOTASK_RUNNING_STATUS QTimer::AbsTime_NextExecDate(__inout DWORD& dwDate)
{
	if (dwDate > m_tmLifeEnd.MakeDate())
		return AUTOTASK_RUNNING_STATUS_OVERDUE;	// 过期
	if (m_arX.size() < 1)
		return AUTOTASK_RUNNING_STATUS_BADTIMER;
	switch (m_eExecFlag)
	{
	case AUTOTASK_EXEC_ATDATE:
		{
			int idx = m_arX.find_first_lgoreq(dwDate);
			if (-1 != idx)
			{
				dwDate = m_arX[idx];
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC; // 无机会再执行
		}
	case AUTOTASK_EXEC_ATDAILY:
		{
			ASSERT(m_arX.size() == 1);
			QTimeSpan tsDate = QTime::ParseDate(dwDate) - QTime::ParseDate(m_tmLifeBegin.MakeDate());
			DWORD xDay = (DWORD)(tsDate.GetTotalDays()) % m_arX[0]; // 还有几天下次执行
			if (xDay > 0)
			{
				QTime tmNextExecDate = QTime::ParseDate(dwDate) + QTimeSpan(xDay,0,0,0);
				if (tmNextExecDate.CompareDate(m_tmLifeEnd) > 0) // 计算出来的时间大于生命期之后
					return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;	// 无机会执行了。
				dwDate = tmNextExecDate.MakeDate();
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			else if (0 == xDay) //dwDate 本身即为执行日期
			{
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			// 不应该执行到这儿
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	case AUTOTASK_EXEC_ATWEEKDAY:
		{ // 星期x执行
			ASSERT(m_arX.size() == 1); 
			if (0 == m_arX[0]) // 必须至少有一个工作日是可以执行的
				return AUTOTASK_RUNNING_STATUS_BADTIMER;
			QTime tmTest = QTime::ParseDate(dwDate);
			for (int iTestCount = 0; iTestCount < 7; iTestCount++)
			{// 测试7天之内的执行情况,
				if (m_arX[0] & (0x01<<(tmTest.GetDayOfWeek()-1))) // 0-sunday,1-monday...6-saturday
				{
					dwDate = tmTest.MakeDate();
					return AUTOTASK_RUNNING_STATUS_OK;
				}
				tmTest += QTimeSpan(1,0,0,0);
				if (tmTest.CompareDate(m_tmLifeEnd) > 0)
				{
					return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
				}
			}
			// 不应该执行到这儿,当前的设计是一周之内必有一天是可以执行的
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	case AUTOTASK_EXEC_ATMONTHDAY:
		{
			ASSERT(m_arX.size() == 1); 
			if (0 == m_arX[0]) // 必须至少有一个工作日是可以执行的
				return AUTOTASK_RUNNING_STATUS_BADTIMER;
			QTime tmTest = QTime::ParseDate(dwDate);
			for (int iTestCount = 0; iTestCount < 31; iTestCount++)
			{
				if (m_arX[0] & (0x1 << (tmTest.GetDay())))
				{
					dwDate = tmTest.MakeDate();
					return AUTOTASK_RUNNING_STATUS_OK;
				}
				tmTest += QTimeSpan(1,0,0,0); // 下一天
				if (tmTest.CompareDate(m_tmLifeEnd) > 0)
				{
					return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
				}
			}
			// 不应该执行到这儿，因为在31天之内，必有一天是可以执行的
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	default:
		{
			ASSERT(FALSE);
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	}
	return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
}

// 如果tmExec的日期大于tmTest的日期，tmNext设定为m_arTime[0],返回TASK_RUNNING_STATUS_OK
// 如果tmExec的日期等于tmTest的日期，tmNext设定为大于等于tmTest.MakeTime()的值，
//		如果m_arTime中包含这样的时间，返回TASK_RUNNING_STATUS_OK
//		如果不包含这样的时间，返回TASK_RUNNING_STATUS_TIMENOTMATCH
// 如果tmExec的日期小于tmTest的日期，则是逻辑错误发生，返回TASK_RUNNING_STATUS_OVERDUE
ENUM_AUTOTASK_RUNNING_STATUS QTimer::_AbsTime_NextRightTimeFrom(__in const QTime&tmTest,
			__in const QTime& tmExec,__inout DWORD &dwNextExecTime)
{
	ASSERT(!IsRelateTimer());
	ASSERT(m_arTime.size());
	if (m_arTime.size())
	{ // 绝对时间值
		int iCmp = tmExec.CompareDate(tmTest);
		if ( iCmp > 0)
		{
			dwNextExecTime = m_arTime[0];
			return AUTOTASK_RUNNING_STATUS_OK;			
		}
		else if ( 0 == iCmp)
		{
			int idx = m_arTime.find_first_lgoreq(tmTest.MakeTime());
			if (-1 != idx)
			{
				dwNextExecTime = m_arTime[idx];
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			return AUTOTASK_RUNNING_STATUS_TIMENOTMATCH;
		}
		else
		{
			return AUTOTASK_RUNNING_STATUS_OVERDUE;
		}
	}
	// 这儿不应该被执行到
	ASSERT(FALSE); 
	return AUTOTASK_RUNNING_STATUS_BADTIMER;
}

BOOL QTimer::SetExp( const QString& sExp )
{
	if (sExp.IsEmpty())
	{
		ASSERT(FALSE);
		return FALSE;
	}
	m_sExpWhen = sExp;
	return ParseExp(sExp);
}

BOOL QTimer::Update(int nTimerID, QTime tmB, QTime tmEnd, LPCWSTR szExpWhen, 
    LPCWSTR szExpRemind, LPCWSTR szExpXFiled)
{
	ASSERT(!IsStarted());
    m_nID = nTimerID;
    m_sXFiledExp = szExpXFiled;
    if ( !SetLifeTime(tmB, tmEnd) )
        return FALSE;

    if (!SetExp(szExpWhen) )
        return FALSE;

    if (!SetRemindExp(szExpRemind))
        return FALSE;

    return TRUE;
}

BOOL QTimer::SetRemindExp(LPCWSTR pszRmdExp)
{
	if (IsStarted())
	{
		ASSERT(FALSE);
		return FALSE;
	}
	m_sExpRemind = pszRmdExp;
    return TRUE;
}

int QTimer::GetExecTimeSpot(__out std::vector<QTime>& vTimes)
{
    if (IsRelateTimer())
    {
        ASSERT(FALSE);
        return 0;
    }
    vTimes.clear();
    for (int i = 0; i < m_arTime.size(); ++i)
    {
        vTimes.push_back(QTime::ParseTime(m_arTime[i]));
    }
    ASSERT(vTimes.size());      // 至少要有一个执行时间点
    return vTimes.size();
}

DWORD QTimer::GetExecDate(  )
{
	ASSERT(!IsRelateTimer());
	if (m_arX.size())
	{
		return m_arX[0];
	}
	return 0;
}

BOOL QTimer::GetRemindString(__out QString& sReminderDes)
{
	int nA;
	WCHAR cUnit;
	QString sSound,sMsg;
	BOOL bOK = QTimer::ParseRemindExp(m_sExpRemind,nA,cUnit,sSound,sMsg);
	if (bOK)
	{
		sReminderDes.Format(
            L"<b .back>在执行前:</b> <b .yellow>%d %s</b><br/>"
            L"<b .back>播放声音:</b>%s<br/><b .back>提示消息:</b>%s",
            nA, QHelper::GetTimeUnitString(cUnit),
            sSound.IsEmpty() ? L"无" : sSound,
            sMsg.IsEmpty() ? L"无" : sMsg);
        return TRUE;
	}
	return FALSE;
}

DWORD QTimer::GetExecSpanSeconds() const
{
	return QHelper::HowManySeconds(m_dwSpan,m_cSpanUnit);
}

DWORD QTimer::GetExecSpanSeconds2() const
{
	return QHelper::HowManySeconds(m_dwSpan2,m_cSpanUnit2);
}

BOOL QTimer::GetWhenDoString(QString &sWhenDoDes)
{
	QString sPart;
	if (IsRelateTimer())
	{
		// within [a~b]
// 		sRet.Format(QGetTextString(L"4"),
// 			m_tmLifeBegin.Format(L"%Y/%m/%d %H:%M"),
// 			m_tmLifeEnd.Format(L"%Y/%m/%d %H:%M"));
		// after
		sWhenDoDes.Format(L"在<b .yellow>[%s] [%d][%s]</b>之后",
			GetExecFlagText(m_eExecFlag),
			GetExecSpan(),
			QHelper::GetTimeUnitString(GetExecSpanUnit()));
		// then every
		if (IsExecSpan2())
		{
			sPart.Format(L"然后每 <b .yellow>[%d][%s]</b>执行",
				GetExecSpan2(),
				QHelper::GetTimeUnitString(GetExecSpanUnit2()));
			sWhenDoDes += L"<br/>" + sPart;
			// after x times stop
			if (IsExecCount())
			{
				sPart.Format(L"在 <b .yellow>[%d]</b> 次后停止",
					GetExecCount());
				sWhenDoDes += L"<br />" + sPart;
			}
		}
	}
	else
	{
		QTime tmTest;
        for (int i = 0; i < m_arTime.size(); ++i)
        {
            tmTest = QTime::ParseTime(m_arTime[i]);
            sPart.AppendFormat(tmTest.Format(L"%H:%M:%S,"));
        }
        sPart.TrimRight(L',');
		switch (m_eExecFlag)
		{
		case AUTOTASK_EXEC_ATDATE:
			{
				tmTest = QTime::ParseDate(m_arX[0]);
				sWhenDoDes.Format(L"在 <b .yellow>[%s] [%s]</b>",
                    tmTest.Format(L"%Y/%m/%d"),sPart);
				break;
			}
		case AUTOTASK_EXEC_ATDAILY:
			{
				ASSERT(m_arX.size() == 1);
				sWhenDoDes.Format(L"每 <b .yellow>[%d]</b> 天的 <b .yellow>[%s]</b>",
                    m_arX[0],sPart);
				break;
			}
		case AUTOTASK_EXEC_ATWEEKDAY:
			{ // 星期x执行
				ASSERT(m_arX.size() == 1); 
				QString sWeekdays,sTemp;
				for (int iWeekday = 0; iWeekday < 7; iWeekday++)
				{// 测试7天之内的执行情况,
					if (m_arX[0] & (0x01<<iWeekday)) // 0-sunday,1-monday...6-saturday
					{
						sTemp.Format(L"%d,",iWeekday);
						sWeekdays += sTemp;
					}
				}
				sWeekdays = sWeekdays.Left(sWeekdays.GetLength() - 1);
				sWhenDoDes.Format(L"<b .yellow>星期[%s]</b>(0-周日,...,6-周六)的 [%s]<br />",sWeekdays,sPart);
				break;
			}
		case AUTOTASK_EXEC_ATMONTHDAY:
			{
				ASSERT(m_arX.size() == 1); 
				QString sDays,sTemp;
				for (int iDay = 0; iDay < 31; iDay++)
				{
					if (m_arX[0] & (0x1 << iDay))
					{
						sTemp.Format(L"%d,",iDay);
						sDays += sTemp;
					}
				}
				sDays = sDays.Left(sDays.GetLength() - 1);
				sWhenDoDes.Format(L"每 <b .yellow>月[%s]</b> 的 [%s]",sDays,sPart);
				break;
			}
        default:
            return FALSE;
		}
	}
	return TRUE;
}

BOOL QTimer::EnableReminder( HANDLE hTimerQueue,int nTaskID,BOOL bEnable/*=TRUE*/ )
{
    BOOL bRet;
	if (bEnable)
	{
		if (IsStarted())
		{
			bRet = SetRemindTimer(hTimerQueue,nTaskID,m_tmNextExec);
		}
		m_dwTimerFlag &= ~TIMER_FLAG_REMINDERDISABLED;
	}
	else
	{
		if (m_hTimerReminder != NULL)
		{
			if (!DeleteTimerQueueTimer(hTimerQueue,m_hTimerReminder,NULL))
			{
				return FALSE;
			}
			m_hTimerReminder = NULL;
		}
		m_dwTimerFlag |= TIMER_FLAG_REMINDERDISABLED;
	    bRet = TRUE;
    }
	return bRet;
}
