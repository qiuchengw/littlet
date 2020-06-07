#pragma once

#include "BaseType.h"

#include <vector>
#include <list>
#include <map>
#include "time/QTime.h"

// 命名约定：
// struct T - data struct
// class D - data class
// class C - UI class
// class I - interface class

typedef DWORD IDExtResType;

#ifndef INVALID_ID
#define INVALID_ID (-1)
#endif

enum
{
    INVALID_ID_VAL = -1,
};

// 所有的数据库表对应的数据结构都必须继承自此结构
struct IDataItem
{
public:
    IDataItem()
    {
        m_nID = INVALID_ID;
    }

    // 数据项ID
    int      m_nID;
    // 数据项创建时间
    QTime       m_tmCreation;

    // 删除
    virtual void OnDelete() 
    {
   
    }

    // 移除
    virtual void OnRemove()
    {

    }

    int& ID()
    {
        return m_nID;
    }

    QTime& CreationTime()
    {
        return m_tmCreation;
    }

    // bWithTime    是否有time
    CStdString CreationTimeDes(BOOL bWithTime = FALSE)
    {
        if (bWithTime)
            return CreationTime().Format(L"%Y/%m/%d %H:%M:%S");
        else
            return CreationTime().Format(L"%Y/%m/%d");
    }

    // 自定义格式
    CStdString CreationTimeDes(LPCWSTR szFormat)
    {
        return CreationTime().Format(szFormat);
    }
};
