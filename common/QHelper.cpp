#include "QHelper.h"

//////////////////////////////////////////////////////////////////////////
// 忽略毫秒
/*
const QHelper::_CONTACT_RELATION QHelper::sm_arrRelationMap[]=
{
	{CONTACT_RELSHIP_FAMILY,L"family"},
	{CONTACT_RELSHIP_LOVER,L"lover"},
	{CONTACT_RELSHIP_FRIEND,L"friend"},
	{CONTACT_RELSHIP_MATE,L"mate"},
	{CONTACT_RELSHIP_WORK,L"work"},
	{CONTACT_RELSHIP_OTHER,L"other"},
};

QString QHelper::GetRelationText(ENUM_CONTACT_RELATIONSHIP eShip)
{
	for (int i = 0; i < ARRAYSIZE(sm_arrRelationMap); i++)
	{
		if (sm_arrRelationMap[i].eShip == eShip)
			return sm_arrRelationMap[i].sShip;
	}
	return L"";
}
ENUM_CONTACT_RELATIONSHIP QHelper::GetRelateFlag(const QString& sShip)
{
	for (int i = 0; i < ARRAYSIZE(sm_arrRelationMap); i++)
	{
		if (sm_arrRelationMap[i].sShip == sShip)
			return sm_arrRelationMap[i].eShip;
	}
	return CONTACT_RELSHIP_OTHER;
}
*/

QString QHelper::UniqueString()
{
	QString str;
	QTime tmNow = QTime::GetCurrentTime();
	str.Format(L"%lf",tmNow.GetTime());
	str.Remove(L'.');
	return str;
}

// t2 - t1 的差值，相距的秒数
DWORD QHelper::TimeSpan(WORD t1,WORD t2)
{
	return (QTime::ParseTime(t2) - QTime::ParseTime(t1)).GetTotalSeconds();
}

// 返回t1 + dwSecs 秒得到的时间值，
// 如果超过了23：59 ,bOverflow 为真
WORD QHelper::TimePlus(WORD t1,DWORD dwSecs,BOOL &bOverflow)
{
	ASSERT(FALSE);
	return 0;
}

QTime QHelper::CombineDateTime(DWORD dwDate,WORD wTime)
{
	QTime tmDate = QTime::ParseDate(dwDate);
	QTime tmTime = QTime::ParseTime(wTime);
	tmDate.SetTime(tmTime.GetHour(),tmTime.GetMinute(),tmTime.GetSecond());
	return tmDate;
}

DWORD QHelper::HowManySeconds( int nX,WCHAR cUnit )
{
	switch (cUnit)
	{
	case L's':case L'S': return nX;
	case L'm':case L'M': return nX * 60;
	case L'h':case L'H': return nX * 3600;
	default:ASSERT(FALSE); return 0;
	}
}

QString QHelper::GetTimeUnitString(WCHAR cUnit)
{
	switch (cUnit)
	{
	case L's':case L'S': return L"秒";
	case L'm':case L'M': return L"分";
	case L'h':case L'H': return L"时";
	default:ASSERT(FALSE); return L"无效单位";
	}
}

QString QHelper::MakeReleateExp( ENUM_AUTOTASK_EXECFLAG eExec, 
    int nSpan, wchar_t cUnit, /* 相对于eExec?定时间 */ 
    int nSpan1 /*= 0*/, wchar_t cUnit1 /*= L's'*/, /* ?后间隔， 0 为无间隔 */ 
    int nExecCount /*= 0*/ )
{
    QString sExp;
    // R=1;P=20m;Q=22m;C=100;
    // R
    sExp.Format(L"R=%d;", eExec);

    //P - span1
    sExp.AppendFormat(L"P=%d%c;", nSpan, cUnit);

    // Q - span2
    if (nSpan1 > 0)
    { // span2
        sExp.AppendFormat(L"Q=%d%c;",nSpan1, cUnit1);
        // C - exec count
        if (nExecCount > 0)
        {
            sExp.AppendFormat(L"C=%d;",nExecCount);
        }
    }
    return sExp;
}

BOOL QHelper::ParseInt(__in const CStdStringW& src, __out int& nT)
{
    CStdStringW tmp = src;
    if ( tmp.Trim().IsEmpty() )
        return FALSE;

    if (-1 == tmp.find_first_not_of(L"0123456789"))
    {
        nT = StrToInt(tmp);
        return TRUE;
    }
    return FALSE;
}

BOOL QHelper::MakeAbsExp(ENUM_AUTOTASK_EXECFLAG eFlag,QTime& tmB, QTime& tmE,
    StringArray& arDatePots, StringArray& arTimePots,
    __out QString& sResultExp, __out QString& sError)
{
    if (arTimePots.empty() || arDatePots.empty())
    {
        ASSERT(FALSE);
        sError = L"必须至少指定一个日期和一个时间";
        return FALSE;
    }

    // 先获取时间点
    QString sTimePots;
    QTime tTime;
    std::vector<DWORD> vExist;
    for (int i = 0; i < arTimePots.size(); ++i)
    {
        if ( !tTime.ParseDateTime(arTimePots[i], VAR_TIMEVALUEONLY))
        {
            sError = L"执行时间点错误";
            return FALSE;
        }
        DWORD dwT = tTime.MakeTime();
        if (std::find(vExist.begin(), vExist.end(), dwT) == vExist.end())
        {
            vExist.push_back(dwT);
            sTimePots.AppendFormat(L"%u,", dwT);
        }
    }
    // 去掉最后一个字符','
    sTimePots.TrimRight(L',');

    // 日期部分
    if (AUTOTASK_EXEC_ATDATE == eFlag)
    { // 单个日期和时间点执行
        QTime tmNow = QTime::GetCurrentTime();
        QTime tDate;
        tDate.ParseDateTime(arDatePots[0]);
        TRACE(L"parsedate string:-> %s  result:-> %s\n",
            (LPCWSTR)arDatePots[0], (LPCWSTR)tDate.Format(L"%c"));
        TRACE(L"time now: %s\n", (LPCWSTR)tmNow.Format(L"%c"));
        if (tDate.CompareDate(tmNow) < 0)
        {
            sError = L"执行日期已经过去了";
            return FALSE;
        }
        // 自动调整任务的生命期为合适的时间
        if ((tmB >= tmE) || (tmE <= tmNow) || (tmB <= tmNow)
            || (tmB > tDate) || (tmE < tDate))
        {
            tmB = tmNow;
            tmE = tDate + QTimeSpan(1,0,0,0);
        }
        sResultExp.Format(L"A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATDATE, tDate.MakeDate(), sTimePots);
        return TRUE;
    }
    else if (AUTOTASK_EXEC_ATDAILY == eFlag)
    { // 每隔x天执行一次
        if (arDatePots.size() > 1)
        {
            sError = L"按日间隔执行   参数太多";
            return FALSE;
        }
        int nXDay;
        if (!QHelper::ParseInt(arDatePots[0], nXDay))
        {
            sError = L"按日间隔执行   参数解析错误      应该指定一个整数间隔";
        }
        if ((nXDay < 1) || (nXDay > 30))
        {
            sError = L"按日间隔执行   间隔范围应该在[1,30]之间";
            return FALSE;
        }
        sResultExp.Format(L"A=%d;P=%u;T=%s;", AUTOTASK_EXEC_ATDAILY, nXDay, sTimePots);
        return TRUE;
    }
    else if (AUTOTASK_EXEC_ATWEEKDAY == eFlag)
    { 
        // 周几
        int v , weeks = 0;
        for (int i = 0; i < arDatePots.size(); ++i)
        {
            if (!QHelper::ParseInt(arDatePots[i], v) || (v > 6))
            {
                sError = L"按周执行   参数解析错误    应该指定一个[0, 6]的整数";
                return FALSE;
            }
            weeks |= (0x1<<v);
        }
        if (0 == weeks)
        {
            sError = L"按周执行   参数解析错误    应该至少指定一个[0, 6]的整数";
            return FALSE;
        }
        sResultExp.Format(L"A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATWEEKDAY, weeks, sTimePots);
        return TRUE;
    }
    else if (AUTOTASK_EXEC_ATMONTHDAY == eFlag)
    {
        int d, days = 0;
        for (int i = 0; i < arDatePots.size(); ++i)
        {
            if (!QHelper::ParseInt(arDatePots[i], d) || (d > 31))
            {
                sError = L"按日执行   参数解析错误    参数范围[0, 31], 0 - 每日都执行";
                return FALSE;
            }
            else if (0 == d)
            {
                days = 0x0fffffff;  // 全日执行
                break;
            }
            days |= (0x1 << d);
        }
        if (0 == days)
        {
            sError = L"按日执行   参数解析错误    应该至少指定一个[0, 31]的整数,  0 - 每日都执行";
            return FALSE;
        }
        sResultExp.Format(L"A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATMONTHDAY,  days, sTimePots);
        return TRUE;
    }
    sError = L"绝对表达式    执行参数错误";
    return FALSE;
}

QString QHelper::MakeRemindExp( int nA, WCHAR cAUnit, 
    const QString&sSound,const QString&sMsg )
{
    cAUnit = tolower(cAUnit);
    if ((nA <= 0) || ((cAUnit != L's') && (cAUnit != L'm') && (cAUnit != L'h')))
        return L"";
    QString sRet;
    sRet.Format(L"A=%d%c;\nS=%s;\nM=%s;\n",nA,cAUnit,sSound,sMsg);
    return sRet;
}

BOOL QHelper::ParseUnitTime(__in const CStdStringW& src, __out int &nT, __out wchar_t& cUnit)
{
    CStdStringW tmp = src;
    if (tmp.Trim().IsEmpty())
        return FALSE;

    cUnit = tolower(tmp.back());
    if ( (L's' != cUnit) && (L'm' != cUnit) && (L'h' != cUnit))
        return FALSE;
    tmp.pop_back();

    return QHelper::ParseInt(tmp, nT);
}
