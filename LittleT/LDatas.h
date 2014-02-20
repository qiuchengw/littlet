#pragma once

#include "BaseType.h"

enum ENUM_APP_MODE
{
    APP_MODE_INVALID,  //无效 
    APP_MODE_PLAN,  //计划
    APP_MODE_EVENTS,  //自动任务 
    APP_MODE_TODO,  //todo 
    APP_MODE_DING,  //灵感 ding~~ 亮了
};

namespace littlet
{
    void EnterAppMode(ENUM_APP_MODE eMode);
    ENUM_APP_MODE GetAppMode();

    /** ctrl键按下，有可能是本地快捷键，调用此函数可以完成相应操作
     *	return
     *      SetMsgHandled(OnCtrlKeyPressDown(...));
     *	param
     *		-[in]
     *          nChar       vk_xx
    **/
    BOOL OnCtrlKeyPressDown(UINT nChar,UINT nRepCnt,UINT nFlags);
};

class LViewEvent;
class LViewTodo;
class LViewPlan;

class LDatas
{
    friend void littlet::EnterAppMode(ENUM_APP_MODE eMode);
     
    SINGLETON_ON_DESTRUCTOR(LDatas)
    {

    }
public:
    void SetViewPlanPtr(LViewPlan *p)
    {
        m_pViewPlan = p;
    }

    LViewPlan* GetViewPlanPtr()
    {
        return m_pViewPlan;
    }

    void SetViewEventPtr(LViewEvent* p )
    {
        m_pViewEvent = p;
    }

    LViewEvent* GetViewEventPtr()
    {
        return m_pViewEvent;
    }

    void SetViewTodoPtr(LViewTodo *p)
    {
        m_pViewTodo = p;
    }

    LViewTodo* GetViewTodoPtr()
    {
        return m_pViewTodo;
    }

    ENUM_APP_MODE GetAppMode()const
    {
        return m_eMode;
    }

protected:
    void SetAppMode(ENUM_APP_MODE eMode)
    {
        m_eMode = eMode;
    }

private:
    ENUM_APP_MODE       m_eMode;
    LViewEvent*         m_pViewEvent;
    LViewTodo*          m_pViewTodo;
    LViewPlan*          m_pViewPlan;
};

