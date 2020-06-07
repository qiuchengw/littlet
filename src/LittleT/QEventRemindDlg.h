#pragma once

#include "ui/QDialog.h"
#include "../common/QTimer.h"

enum 
{
    AUTOTASK_REMINDER_COUNTDOWN_TIMERID = 101,
};

class LSingleRmdDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(LSingleRmdDlg)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(QDialog)
    END_MSG_MAP()

public:
    LSingleRmdDlg(LPTASK_REMINDER_PARAM pRp = NULL);
    ~LSingleRmdDlg(void);

    void SetRmdParam(LPTASK_REMINDER_PARAM pRP);

protected:
    virtual LRESULT OnDocumentComplete();
    void OnClkDontRemindAgain(HELEMENT hBtn);
    void OnClkDontExecThis(HELEMENT hBtn);
    void OnDestroy();
    void OnTimer(UINT nTimerID);

    void StartCountDown();
    // 刷新提示信息
    void RefreshRemindMessage();

protected:
    int         m_nCountDown;
    LPTASK_REMINDER_PARAM	m_pRP;
    ECtrl       m_ctlCountdown;
};

// 只能在主界面线程中使用
class LReminderBox
{
    SINGLETON_ON_DESTRUCTOR(LReminderBox)
    {
        RemoveAll();
    }

    class ST_RMDDLG 
    {
    public:
        ST_RMDDLG()
        {
            m_pRmdP = NULL;
            m_bClosed = FALSE;
        }
        ~ST_RMDDLG()
        {
        }

        void SetRmdParam(LPTASK_REMINDER_PARAM pRP)
        {
            m_pRmdP = pRP;

            m_Dlg.SetRmdParam(m_pRmdP);
        }

        BOOL IsClosed()const
        {
            return m_bClosed;
        }

        void SetClose( BOOL bClose )
        {
            m_bClosed = bClose;
        }

        LSingleRmdDlg* GetRmdDlg()
        {
            return &m_Dlg;
        }

        LPTASK_REMINDER_PARAM GetRmdParam()const
        {
            return m_pRmdP;
        }

        int GetTaskID()
        {
            if (NULL == m_pRmdP)
            {
                ASSERT(FALSE);
                return -1;
            }
            return m_pRmdP->nTaskID;
        }
    private:
        LSingleRmdDlg       m_Dlg;
        BOOL                m_bClosed;
        LPTASK_REMINDER_PARAM m_pRmdP;
    };

    typedef std::list<ST_RMDDLG*> RmdList;
    typedef RmdList::iterator RmdListItr;

public:
    BOOL ShowReminderDlg(LPTASK_REMINDER_PARAM pRP);
    void RemoveReminderDlg(int nTaskID);

    void RmdDlgDestroying(LSingleRmdDlg* pDlg);
    // 事件的定时器改变了，可以关闭当前的提示对话框
    void OnEventTimerChanged( int nEventID );
    // 事件删除了

protected:
    void RemoveAll();
    // 检查下那个事件的对话框已经关闭了
    // 如果标记为已关闭，并且窗口确实销毁了，就释放数据
    void CheckRmdList();
    
private:
    RmdList     m_lstRmd;
};
