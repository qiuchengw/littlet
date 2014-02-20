#pragma once

#include <list>
#include <algorithm>
#include <iterator>
#include "ConstValues.h"
#include "QTimer.h"

class QHelper
{
public:
	// s/m/h
	static QString GetTimeUnitString(WCHAR cUnit);
	static DWORD HowManySeconds( int nX,WCHAR cUnit );

	// 获取一个唯一的字符串
	static QString UniqueString();

	// t2 - t1 的差值，相距的秒数
	static DWORD TimeSpan(WORD t1,WORD t2);

	// 返回t1 + dwSecs 秒得到的时间值，
	// 如果超过了23：59 ,bOverflow 为真 
	static WORD TimePlus(WORD t1,DWORD dwSecs,BOOL &bOverflow);
	// 结合Date和Time
	static QTime CombineDateTime(DWORD dwDate,WORD wTime);

    static BOOL ParseInt(__in const CStdStringW& src, __out int& nT);

    static QString MakeReleateExp( ENUM_AUTOTASK_EXECFLAG eExec,
        int nSpan, wchar_t cUnit,   // 相对于eExec一定时间
        int nSpan1 = 0, wchar_t cUnit1 = L's', // 然后间隔， 0 为无间隔
        int nExecCount = 0); // 总共能执行的次数, 0为不限制

    static BOOL MakeAbsExp(ENUM_AUTOTASK_EXECFLAG eFlag,QTime& tmB, QTime& tmE, 
            StringArray& arDatePots, /* 执行日期点 */ StringArray& arTimePots, 
            __out QString& sResultExp, __out QString& sError);

    static QString MakeRemindExp( int nA, WCHAR cAUnit, const QString&sSound,const QString&sMsg );

    /** 10h，10s，10m 这样的时间格式解析
     *	return:
     *      TRUE    数据格式正确
     *	params:
     *		-[in]
     *          src     被解析的字符串
     *		-[out]
     *          nT      数字部分
     *          cUnit   单位部分
    **/
    static BOOL ParseUnitTime(__in const CStdStringW& src, __out int &nT, __out wchar_t& cUnit);
};
