#include "LDatas.h"
#include "ui/QUIGlobal.h"
#include "../common/ConstValues.h"
#include "LViewEvent.h"
#include "LViewTodo.h"
#include "LViewPlan.h"

namespace littlet
{
    // 进入到某模式
    void EnterAppMode(ENUM_APP_MODE eMode)
    {
        QUIPostCodeToMainWnd(MWND_CMD_ENTERAPPMODE,(LPARAM)eMode);

        LDatas::GetInstance()->SetAppMode(eMode);
    }

    ENUM_APP_MODE GetAppMode()
    {
        return LDatas::GetInstance()->GetAppMode();
    }

    BOOL OnCtrlKeyPressDown( UINT nChar,UINT nRepCnt,UINT nFlags )
    {
        if ( VK_TAB == nChar)
        {
            switch (littlet::GetAppMode())
            {
            case APP_MODE_PLAN:
                {
                    littlet::EnterAppMode(APP_MODE_EVENTS);
                    break;
                }
            case APP_MODE_EVENTS:
                {
                    littlet::EnterAppMode(APP_MODE_TODO);
                    break;
                }
            case APP_MODE_TODO:
                {
                    littlet::EnterAppMode(APP_MODE_PLAN);
                    break;
                }
            }
        }
        else if (0x4E == nChar) // N
        {
            switch (littlet::GetAppMode())
            {
            case APP_MODE_PLAN:
                {
                    // 新计划
                    LDatas::GetInstance()->GetViewPlanPtr()->OnClkNewPlan(NULL);
                    break; 
                }
            case APP_MODE_EVENTS:
                {
                    // 新自动任务
                    LDatas::GetInstance()->GetViewEventPtr()->OnClkNewEvent(NULL);
                    break;
                }
            case APP_MODE_TODO:
                {
                    // 新todo任务
                    LDatas::GetInstance()->GetViewTodoPtr()->OnClkNewTask(NULL);
                    break;
                }
            case APP_MODE_DING:
                {
                    break;
                }
            }
        }
        return FALSE;
    }

};
