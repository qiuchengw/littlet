#pragma once

#include "Console.h"
#include <iostream>
#include <vector>
#include "time/QTime.h"
#include "../common/QHelper.h"
#include "stdstring.h"

using namespace std;

namespace con = JadedHoboConsole;

class QArgvParser
{
    typedef vector<CStdStringW> StrArray;
    typedef StrArray::iterator StrArrayItr;
    struct  
    {
        
    };

public:
    // 第一步：Set
    BOOL SetArgv(_TCHAR* argv[], int nArgc)
    {
        if (nArgc < 3)
        {
            m_arErrors.push_back(L"参数太少！");
            return FALSE;
        }

        m_args.clear();

        // 从第二个参数进行记录
        CStdStringW sArgv;
        for (int i = 1; i < nArgc; i++)
        {
            sArgv = argv[i];
            sArgv.Trim();
            if (!sArgv.IsEmpty())
            {
                m_args.push_back(CStdStringW(argv[i]).Trim());
            }
        }

        if (m_args.size() < 2)
        {
            m_arErrors.push_back(L"参数太少！");
            return FALSE;
        }
        m_arErrors.clear();
        return TRUE;
    }

    // 第二步：解析
    BOOL ParseArgs(__out ENUM_AUTOTASK_DOWHAT& eDoWhat, __out CStdStringW& sDoWhatParam,
            __out CStdStringW& sWhenDo, __out CStdStringW& sRemindExp,
            __out QTime& tmBegin, __out QTime& tmEnd)
    {
        BOOL bOK = FALSE;
        do 
        {
            // 做什么
            eDoWhat = GetDoWhat(sDoWhatParam);
            if (AUTOTASK_DO_NOTSET == eDoWhat)
                break;

            // 何时做
            if (!GetExecTime(sWhenDo, tmBegin, tmEnd))
                break;

            // 关于任务提示
            if (!GetTaskRemind(sRemindExp))
                break;

            bOK = TRUE;
        } while (false);

        return bOK;
    }

    /** 在控制台输出错误
     *	params: none
    **/
    void EchoErrors()
    {
        std::cout<<con::bg_black<<con::fg_red;
        for (auto i = m_arErrors.begin(); i != m_arErrors.end(); ++i)
        {
            std::wcout<<*i<<std::endl;
        }
        std::wcout<<con::fg_white;
    }

#ifdef _DEBUG
    void TestString(const CStdString& src)
    {
        StrArray ar;
        SplitString(src, ar);

        std::wcout<<src<<L" :  size: "<<ar.size()<<"\n------------"<<endl;
        for (auto i = ar.begin(); i != ar.end(); ++i)
        {
            std::wcout<<*i<<std::endl;
        }
        std::wcout<<L"-----------------"<<endl;
    }
    
    // 测试代码
    void TestSplitString()
    {
        TestString(L"a;b;c");

        TestString(L"a;;c");

        TestString(L"a;b;");

        TestString(L";b;c");

        TestString(L";b;");

        TestString(L";;");
    }
#endif

protected:
    /** 获取执行命令
     *	return:
     *      AUTOTASK_DO_NOTE    无效命令或help命令。
    **/
    ENUM_AUTOTASK_DOWHAT GetDoWhat(__out CStdStringW& sDoWhat)
    {
        ASSERT(m_args.size() >= 2);

        ENUM_AUTOTASK_DOWHAT eRet = AUTOTASK_DO_NOTSET;

        StrArray::iterator iB = m_args.begin();
        CStdStringW s1 = *iB;
        m_args.erase(iB);

        BOOL bNeedParam = FALSE;
        if (s1.CompareNoCase(L"-m") == 0)
        {
            bNeedParam = TRUE;
            eRet = AUTOTASK_DO_REMIND; // remind message
        }
        else if (s1.CompareNoCase(L"-e") == 0)
        {
            bNeedParam = TRUE;
            eRet = AUTOTASK_DO_EXECPROG;
        }
        else if (s1.CompareNoCase(L"-s") == 0)
        {
            eRet = AUTOTASK_DO_SYSSHUTDOWN;
        }
        else if (s1.CompareNoCase(L"-b") == 0)
        {
            eRet = AUTOTASK_DO_BREAKAMOMENT;
        }

        if (AUTOTASK_DO_NOTSET  == eRet)
        {
            m_arErrors.push_back(L"无效的任务执行类型！");
        }
        else if (bNeedParam)
        {
            iB = m_args.begin();
            CStdStringW s2 = *iB;
            m_args.erase(iB);

            if (!IsCommand(s2))
            {
                sDoWhat = s2;
            }
            else
            {
                CStdString sTmp;
                sTmp.Format(L"命令 %s 缺少参数", s1);
                m_arErrors.push_back(sTmp);
                eRet = AUTOTASK_DO_NOTSET;
            }
        }

        return eRet;
    }

    /** 任务提示
     *	return:
     *      TRUE    搞定
     *	params:
     *		-[out]
     *      sRemind 
    **/
    BOOL GetTaskRemind(__out CStdStringW& sRemind)
    {
        // -w
        StrArrayItr i = FindX(L"-w");
        if (_End() == i)
        {
            return TRUE;
        }

        // 下一个就是参数
        ++i;
        if (_End() == i)
        {
            m_arErrors.push_back(L"-w   任务提示    未指定参数");
            return FALSE;
        }

        StrArray arP;
        if (SplitString(*i, arP) != 3)
        {
            m_arErrors.push_back(L"-w   任务提示    参数个数错误：" + *i);
            return FALSE;
        }

        // 时间
        int nT;
        wchar_t cUnit;
        if (!QHelper::ParseUnitTime(arP[0], nT, cUnit))
        {
            m_arErrors.push_back(L"-w   任务提示    提示时间错误：" + *i);
            return FALSE;
        }
        // 提示时间不能过长
        DWORD nSecs = QHelper::HowManySeconds(nT, cUnit);
        if ((nSecs > 86400) || (nSecs < 5))
        {
            m_arErrors.push_back(L"-w   任务提示    提示时间范围[10s, 24h)：" + *i);
            return FALSE;
        }

        sRemind = QHelper::MakeRemindExp(nT, cUnit, arP[1], arP[2]);
        return !sRemind.IsEmpty();
    }

    /** 任务的执行时间
     *	return:
     *      TRUE    解析成功
     *	params:
     *		-[out]
     *      sTime   解析过后的表达式，能够直接被QTimer使用
    **/
    BOOL GetExecTime(__out CStdStringW& sTime, __out QTime& tmBegin, __out QTime& tmEnd)
    {
        if ( !GetLifePeriod(tmBegin, tmEnd) )
        {
            return FALSE;
        }

        // 检查相对时间
        StrArrayItr iR = FindX(L"-r");
        StrArrayItr iA = FindX(L"-a");
        if ((_End() != iR) && (_End() != iA))
        {
            m_arErrors.push_back(L"-r/-a    执行时间    只能指定一个执行时间");
            return FALSE;
        }

        if ( (_End() == iA) && (_End() == iR))
        {
            m_arErrors.push_back(L"-r/-a    未设定执行时间");
            return FALSE;
        }

        if (_End() != iA)
        { // 绝对时间
            // 下一个就是参数
            ++iA;
            if (_End() == iA)
            {
                m_arErrors.push_back(L"-a   执行时间（绝对时间）    未指定参数");
                return FALSE;
            }
            return ParseAbsTime(*iA, tmBegin, tmEnd,sTime);
        }
        
        if (_End() != iR)
        {
            // 下一个就是参数
            ++iR;
            if (_End() == iR)
            {
                m_arErrors.push_back(L"-r   执行时间（相对时间）    未指定参数");
                return FALSE;
            }
            return ParseRelateTime(*iR, sTime);
        }

        return FALSE;
    }

    /** 判断是否是命令，以英文'-'开头的是Command
     *	return:
     *      TRUE    是
     *	params:
     *		-[in]
     *          sText  
    **/
    inline BOOL IsCommand(__in const CStdString& sText)
    {
        ASSERT(sText.size() > 1);
        return (sText[0] == L'-');
    }
    /** 任务有效期
     *	return:
     *      TRUE    成功
    **/
    BOOL GetLifePeriod(__out QTime& tmBegin, __out QTime& tmEnd)
    {
        // -l
        StrArrayItr i = FindX(L"-l");
        if (_End() == i)
        {
            tmBegin = QTime::GetCurrentTime();
            // 执行有效期：10年！够长了吧。
            tmEnd = tmBegin + QTimeSpan(365 * 10, 1, 0 ,0);
            return TRUE;
        }

        // 下一个就是参数
        ++i;
        if (_End() == i)
        {
            m_arErrors.push_back(L"-l   任务有效期    未指定参数");
            return FALSE;
        }
        StrArray arP;
        if (SplitString(*i, arP) != 2)
        {
            m_arErrors.push_back(L"-l   任务有效期   参数错误：" + *i);
            return FALSE;
        }

        if (arP[0].IsEmpty())
        {
            tmBegin = QTime::GetCurrentTime();
        }
        else if (!tmBegin.ParseDateTime(arP[0]))
        {
            m_arErrors.push_back(L"-l   任务有效期   开始时间错误：" + *i);
            return FALSE;
        }

#ifdef _DEBUG
        std::wcout<<L"开始时间:"<<tmBegin.Format(L"%c\n");
#endif

        if (arP[1].IsEmpty())
        {
            tmBegin += QTimeSpan(365 * 10, 1, 1, 1);
        }
        else if (!tmEnd.ParseDateTime(arP[1]))
        {
            m_arErrors.push_back(L"-l   任务有效期   结束时间错误：" + *i);
            return FALSE;
        }

#ifdef _DEBUG
        std::wcout<<L"开始时间:"<<tmEnd.Format(L"%c\n");
#endif
        
        if (tmBegin >= tmEnd)
        {
            m_arErrors.push_back(L"-l   任务有效期   开始时间大于结束时间：" + *i);
            return FALSE;
        }
        return TRUE;
    }

    BOOL ParseAbsTime(__in const CStdStringW& src, __inout QTime& tmBegin, 
        __inout QTime& tmEnd, __out CStdStringW& sTime)
    { // abs time
        ASSERT(tmEnd > tmBegin);

        StrArray arP;
        if (SplitString(src, arP) != 2)
        {
            m_arErrors.push_back(L"-a   执行时间（绝对时间）    参数错误：" + src);
            return FALSE;
        }

        // 日期点；时间点
        StrArray aS;
        if (SplitString(arP[0], aS, L':') != 2)
        {
            m_arErrors.push_back(L"-a   执行时间（绝对时间）    参数错误: " + arP[0]);
            return FALSE;
        }

        ENUM_AUTOTASK_EXECFLAG eFlag;
        // aS[0] 只能有3种可能：p,s,t
        if (aS[0].CompareNoCase(L"d") == 0)
        {   // littlet 启动后
            eFlag = AUTOTASK_EXEC_ATDATE;
        }
        else if (aS[0].CompareNoCase(L"s") == 0)
        {
            eFlag = AUTOTASK_EXEC_ATDAILY;
        }
        else if (aS[0].CompareNoCase(L"w") == 0)
        {
            eFlag = AUTOTASK_EXEC_ATWEEKDAY;
        }
        else if (aS[0].CompareNoCase(L"m") == 0)
        {
            eFlag = AUTOTASK_EXEC_ATMONTHDAY;
        }
        else
        {
            m_arErrors.push_back(L"-a   执行时间（绝对时间）    无效参数错误：" + src);
            return FALSE;
        }
        StringArray arDatePots;
        SplitString(aS[1], arDatePots, L',');

        StringArray arTimePots;
        if (SplitString(arP[1], arTimePots, L',') <= 0)
        {
            m_arErrors.push_back(L"-a   执行时间（绝对时间）    执行时间点无效：" + src);
            return FALSE;
        }
        CStdString sError;
        if ( !QHelper::MakeAbsExp(eFlag, tmBegin, tmEnd, arDatePots, arTimePots, sTime, sError ))
        {
            m_arErrors.push_back(sError);
            return FALSE;
        }
        return TRUE;
    }

    BOOL ParseRelateTime(__in const CStdStringW& src, __out CStdStringW& sTime)
    {
        StrArray arP;
        if (SplitString(src, arP) != 3)
        {
            m_arErrors.push_back(L"-r   执行时间（相对时间）    参数错误：" + src);
            return FALSE;
        }

        // 相对于；间隔；执行次数
        StrArray aS;
        if (SplitString(arP[0], aS, L':') != 2)
        {
            m_arErrors.push_back(L"-r   执行时间（相对时间）    参数错误: " + src);
            return FALSE;
        }
        ENUM_AUTOTASK_EXECFLAG eFlag;
        // aS[0] 只能有3种可能：p,s,t
        if (aS[0].CompareNoCase(L"p") == 0)
        {   // littlet 启动后
            eFlag = AUTOTASK_EXEC_AFTERMINDERSTART;
        }
        else if (aS[0].CompareNoCase(L"s") == 0)
        {
            eFlag = AUTOTASK_EXEC_AFTERSYSBOOT;
        }
        else if (aS[0].CompareNoCase(L"t") == 0)
        {
            eFlag = AUTOTASK_EXEC_AFTERTASKSTART;
        }
        else
        {
            m_arErrors.push_back(L"-r   执行时间（相对时间）    参数错误：" + src);
            return FALSE;
        }
        int nSpan = 0;
        wchar_t cUnit = L's';
        if (!QHelper::ParseUnitTime(aS[1], nSpan, cUnit))
        {
            m_arErrors.push_back(L"-r   执行时间（相对时间）    参数错误：" + src);
            return FALSE;
        }

        // 间隔和执行次数
        int nSpan1 = 0, nExecCount = 0;
        wchar_t cUnit1 = L's';
        if (!arP[1].IsEmpty())
        {
            if (!QHelper::ParseUnitTime(arP[1], nSpan1, cUnit1))
            {
                m_arErrors.push_back(L"-r   执行时间（相对时间）    参数错误：" + src);
                return FALSE;
            }
            if (!arP[2].IsEmpty())
            {
                if (!QHelper::ParseInt(arP[2], nExecCount))
                {
                    m_arErrors.push_back(L"-r   执行时间（相对时间）    参数错误" + src);
                    return FALSE;
                }
            }
        }
        sTime = QHelper::MakeReleateExp(eFlag, nSpan, cUnit, nSpan1, cUnit1, nExecCount);
        return TRUE;
    }

    inline StrArrayItr _End()
    {
        return m_args.end();
    }

    StrArrayItr FindX(const CStdStringW& sP )
    {
        StrArrayItr iEnd = _End();
        for (StrArrayItr i = m_args.begin(); i != iEnd; ++i)
        {
            if (StrCmpIW((*i).c_str(), sP.c_str()) == 0)
            {
                return i;
            }
        }
        return iEnd;
    }

    /** 分割字符串到数组中
     *	return:
     *      分割的字符串个数，分割的个数由chSep的个数决定
     *      分割的个数为 COUNT(chSep) + 1
     *	params:
     *		-[in]
     *          src     源字符串
     *          chSep   分割符号
     *		-[out]
     *          result  结果
    **/
    int SplitString(__in const CStdStringW& src, __out StrArray& result,
                    __in const wchar_t chSep = L';')
    {
        result.clear();

        CStdStringW tmp = src;
        if (tmp.Trim().IsEmpty())
        {
            return 0;
        }
        // 最后一个是 分隔符的话，后面的算作空的字符串
        BOOL bPushEmpty = (tmp.back() == chSep);
        do 
        {
            int i = tmp.Find(chSep);
            if (-1 == i)
            {
                result.push_back(tmp);
                break;
            }
            result.push_back(tmp.Left(i));
            tmp = tmp.Mid(i + 1);
        } while ( !tmp.IsEmpty() );

        if (bPushEmpty)
        {
            result.push_back(CStdStringW());
        }
        return result.size();
    }

private:
    StrArray    m_args;
    StrArray    m_arErrors;
};

