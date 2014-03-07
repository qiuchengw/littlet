#pragma once

#include "../common/ConstValues.h"

#include "ui/QWindow.h"
#include "ui/SystemTraySDK.h"
#include "LViewEvent.h"
#include "LViewTodo.h"
#include "LViewPlan.h"
#include "LDatas.h"
#include "feedback/QUIConnectCenter.h"
#include "ui/WndHelper.h"

DECLARE_USER_MESSAGE(QSOFT_LITTLET_SYSTRAYMSG);

class LittleTFrame : public QFrame
{
    BEGIN_MSG_MAP_EX(LittleTFrame)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_HOTKEY(OnHotKey)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_POWERBROADCAST(OnComputerPowerChanged)
        MSG_WM_COMMAND(OnCommand)
        MSG_WM_COPYDATA(OnCopyData)
        MSG_WM_SIZE(OnSize)
        MESSAGE_HANDLER(QSOFT_MSG_UPDATEAPP,OnAppVersionCheck)
        MESSAGE_HANDLER(QSOFT_LITTLET_SYSTRAYMSG,OnSysTrayMessage)
//         MESSAGE_HANDLER(WM_ENTERSIZEMOVE, HandleAutohideMessage)
//         MESSAGE_HANDLER(WM_EXITSIZEMOVE, HandleAutohideMessage)
//         MESSAGE_HANDLER(WM_MOUSEMOVE, HandleAutohideMessage)
        BEGIN_QUI_MSG
            // todo task
            MSG_QUI_CODE(MWND_NOTIFY_TODOTASKCHANGED,OnTodoTaskStatusChanged)
            // auto task(event)
            MSG_QUI_CODE(MWND_NOTIFY_EVENTNUMCHANGED,OnEventStatusChanged)
            MSG_QUI_CODE(MWND_NOTIFY_AUTOTASKTOGGLEREMINDER,OnEventToggleReminder)
            MSG_QUI_CODE(MWND_NOTIFY_AUTOTASKOVERDUE,OnEventOverdue)
            MSG_QUI_CODE(MWND_NOTIFY_AUTOTASKEDIT,OnEventEdit)
            MSG_QUI_CODE(MWND_NOTIFY_AUTOTASKSTART,OnEventStart)
            MSG_QUI_CODE(MWND_NOTIFY_AUTOTASKDELETE,OnEventDelete)
            MSG_QUI_CODE(MWND_NOTIFY_AUTOTASKPAUSE,OnEventPause)
            MSG_QUI_CODE(MWND_CMD_SHOWAUTOTASKREMIND,OnCmdShowEventReminder)
            MSG_QUI_CODE(MWND_NOTIFY_AUTOTASKFIRED,OnAutoTaskFired)
            // plan
            MSG_QUI_CODE(MWND_NOTIFY_PLANNUMCHANGED,OnPlanNumChanged)
            // app mode
            MSG_QUI_CODE(MWND_CMD_ENTERAPPMODE,OnEnterAppMode)
            //////////////////////////////////////////////////////////////////////////
            // 任务
//             MSG_QUI_CODE(MWND_CMD_SHOWMESSAGE,OnDoWork)
//             MSG_QUI_CODE(MWND_CMD_SHUTDOWN,OnDoWork)
//             MSG_QUI_CODE(MWND_CMD_REBOOT,OnDoWork)
//             MSG_QUI_CODE(MWND_CMD_SHOWSCREENSAVER,OnDoWork)
        END_QUI_MSG
        CHAIN_MSG_MAP_MEMBER(wnd_autohide_)
        CHAIN_MSG_MAP(QFrame)
    END_MSG_MAP()

    QUI_DECLARE_EVENT_MAP;

public:
    LittleTFrame(void);
    ~LittleTFrame(void);

protected:
    void OnClose();
    void OnSize(UINT nType, WTL::CSize sz);
    void OnHotKey(int nSystemHotKey,UINT nModifier,UINT nVk);
    void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);
    LRESULT OnComputerPowerChanged(DWORD , DWORD );

    void OnClkAbout(HELEMENT);
    void OnClkSetting(HELEMENT);
    void OnClkMyWeibo(HELEMENT);
    void OnClkFeedback(HELEMENT);

    virtual QView* CustomControlCreate(HELEMENT he);

    virtual LRESULT OnDocumentComplete();

    void OnTodoTaskStatusChanged(LPARAM lParam);

    void OnCmdShowEventReminder(LPARAM lp);

    void OnAutoTaskFired(LPARAM lp);

    // event（qautotask）要求执行响应的任务
//    void OnDoWork(LPARAM lp);

    void OnEventStatusChanged(LPARAM lParam);
    void OnEventToggleReminder(LPARAM lParam);
    void OnEventOverdue(LPARAM lParam);
    void OnEventEdit(LPARAM lParam);
    void OnEventStart(LPARAM lParam);
    void OnEventPause(LPARAM lParam);
    void OnEventDelete(LPARAM lParam);
    void OnEnterAppMode(LPARAM lParam);

    //////////////////////////////////////////////////////////////////////////
    // plan
    void OnPlanNumChanged(LPARAM lParam);

    //////////////////////////////////////////////////////////////////////////
    // 程序更新
   /**          wParam      1，有更新， 0，当前无更新
    *          lParam      当wParam==0，此域也为0 
    *                        wParam==1，此域为LPQUI_USERMSGPARAM，
    *  -----------------------------------------
    *          LPQUI_USERMSGPARAM 域的值为：
    *                      wParam  1：更新成功
    *                              0：更新失败
    *                      sParam  已下载的更新文件路径
    *                      bFreeIt TRUE    接收者应该删除这个数据
    *		-[out]
    **/
    LRESULT OnAppVersionCheck(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    // system tray 消息
    LRESULT OnSysTrayMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void OnCommand(UINT nType, int nID ,HWND hWnd);
    LRESULT OnCopyData(HWND hWnd, PCOPYDATASTRUCT pps);
    // 窗口自动隐藏消息处理
//    LRESULT HandleAutohideMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void OnTabSwitched(HELEMENT he );
    void RefreshHeader();
    void RefreshEventNum();
    BOOL ConfigHotKey(BOOL bConfig);

    ETabCtrl _Tabs() { return GetCtrl("[id=\"TABS-CONT\"]"); }

    void StartUpdater();

private:
    LViewTodo   m_viewTodo;
    LViewEvent  m_viewEvent;
    LViewPlan   m_viewPlan;
    BOOL        m_bStartup;
    ATOM        m_idAtom;
    CSystemTray m_trayicon;
    CAutoHideWnd    wnd_autohide_;

private:
    BOOL        m_bIsCloseForUpdate;
    QString     m_sUpdateFile;
};

