#pragma once

#include "LittleTView.h"
#include "../common/ConstValues.h"

class QAutoTask;
class LittleTFrame;
class LViewEvent : public LittleTView
{
    friend class LittleTFrame;

    QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(LViewEvent)
        MSG_WM_KEYDOWN(OnKeyDown)
        BEGIN_QUI_MSG
            MSG_QUI_CODE(EVENTWND_CMD_SELECTAUTOTASK,OnCmdSelectEventItem)
            MSG_QUI_CODE(EVENTWND_NOTIFY_USERJUMPEVENTEXEC,OnNotifyJumpOverExec)
        END_QUI_MSG
        CHAIN_MSG_MAP(LittleTView)
    END_MSG_MAP()

public:
    LViewEvent(void);
    ~LViewEvent(void);

    void OnClkNewEvent(HELEMENT );
    void NewEventAdd(QAutoTask *pTask);

protected:

    void OnAutoTaskFired(QAutoTask* pTask);

    void AddEvent(QAutoTask* pTask);
    // 提前提示状态变化了。
    void FreshEventReminderFlag( QAutoTask* pTask );

    void RefreshEventItem(QAutoTask *pTask);

    void SelectEventItem(QAutoTask *pTask);

    void OnCmdSelectEventItem(LPARAM lp);
    // 跳过任务
    void OnNotifyJumpOverExec(LPARAM lp);

    void OnMouseLeaveEventList(HELEMENT );

    ECtrl _TaskList() { return GetCtrl("#EVENT_LIST"); }
    ETable _CurSelTask() { return (HELEMENT)GetCtrl("#EVENT_LIST>table:checked"); }

protected:
    virtual LRESULT OnDocumentComplete();

    //---------------------------------------------------
    // Auto Task
    void OnClkDeleteEvent(HELEMENT hBtn);
    void OnClkEditEvent(HELEMENT hBtn);
    void OnCalcTestEvent(HELEMENT hBtn);
    void OnClkPlayOrPauseEvent(HELEMENT hBtn);
    void OnClkEnableEventReminder(HELEMENT hBtn);
    void OnDragDropDelete(HELEMENT contbox, HELEMENT src,HELEMENT target);
    void OnClkEventItem(HELEMENT he);

    void AddEvent(ECtrl&eATgroup,QAutoTask* pTask);
    void RefreshEventList();
    void RefreshEventItem(ECtrl &eTable);
    void RefreshEventNum();

    HELEMENT _FindEventItem(QAutoTask* pTask);
    ECtrl _InfoPopup() { return GetCtrl("#popup_eventitem"); }
    // 自动选择一个项目
    void AutoSelect();

    void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);

};

