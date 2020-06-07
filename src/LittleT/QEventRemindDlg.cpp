#include "QEventRemindDlg.h"
#include "../common/QAutoTask.h"
#include "../common/LittleTUIcmn.h"
#include "../common/ConstValues.h"
#include "../common/QTimer.h"
#include <Mmsystem.h>
#include "LDatas.h"
#include "LViewEvent.h"

#pragma comment(lib,"Winmm.lib")

QUI_BEGIN_EVENT_MAP(LSingleRmdDlg,QDialog)
    BN_CLICKED_ID(L"btn_skip",&LSingleRmdDlg::OnClkDontExecThis)
QUI_END_EVENT_MAP()

LSingleRmdDlg::LSingleRmdDlg( LPTASK_REMINDER_PARAM pRP )
    :QDialog(L"qabs:dlgs/rmddlg.htm")
{
    m_pRP = pRP;

    m_nCountDown = 300;

}

LSingleRmdDlg::~LSingleRmdDlg( void )
{

    if (IsWindow())
    {
        SendMessage(WM_CLOSE);
        while (IsWindow())
        {
            Sleep(1);
        }
    }
}

void LSingleRmdDlg::OnClkDontRemindAgain( HELEMENT hBtn )
{

}

void LSingleRmdDlg::OnClkDontExecThis( HELEMENT hBtn )
{
    QAutoTaskMan *pMan = QAutoTaskMan::GetInstance();
    if (pMan->JumpoverTaskThisExec(m_pRP->nTaskID))
    {
        // 转到LEvent上的
        QUISendCodeTo(LDatas::GetInstance()->GetViewEventPtr()->GetSafeHwnd(),
            EVENTWND_NOTIFY_USERJUMPEVENTEXEC,
            (LPARAM)m_pRP->nTaskID);
        
        // 关闭掉自己
        PostMessage(WM_CLOSE);
    }
    else
    {
        ECtrl(hBtn).ShowTooltip(L"无法跳过此次任务！");
    }
}

void LSingleRmdDlg::OnDestroy()
{
    LReminderBox::GetInstance()->RmdDlgDestroying(this);
}

void LSingleRmdDlg::SetRmdParam( LPTASK_REMINDER_PARAM pRP )
{
    m_pRP = pRP;

    if ( IsWindow() )
    {
        // 如果只是执行一次的任务，则提示不自动关闭
        StartCountDown();

        RefreshRemindMessage();
    }
}

void LSingleRmdDlg::OnTimer( UINT nTimerID )
{
    if (AUTOTASK_REMINDER_COUNTDOWN_TIMERID == nTimerID)
    {
        if ( --m_nCountDown <= 0 )
        {   // 任务执行了，可以关闭对话框了
            KillTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID);
            PostMessage(WM_CLOSE);
        }
        else
        {
            CStdString str;
            str.Format(L" / %s",QTimeSpan((DWORD)m_nCountDown).Format(L"%H:%M:%S"));
            m_ctlCountdown.SetText(str);
        }
    }
    SetMsgHandled(FALSE);
}

LRESULT LSingleRmdDlg::OnDocumentComplete()
{
    if (NULL != m_pRP)
    {
        m_ctlCountdown = GetCtrl("#td_countdown");

        StartCountDown();

        RefreshRemindMessage(); 
    }

    CenterWindow();

    return 0;
}

void LSingleRmdDlg::StartCountDown()
{
    if (NULL == m_pRP)
    {
        ASSERT(FALSE);
        return;
    }

    if (0 < m_pRP->nSeconds)
    {
        // 也许是重用对话框，重新设置的数据，那么先停止之前的倒数定时器
        KillTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID);

        // 重新启动倒数定时器
        m_nCountDown = m_pRP->nSeconds;

        SetTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID, 1000, NULL);
    }
}

void LSingleRmdDlg::RefreshRemindMessage()
{
    if (NULL == m_pRP)
    {
        ASSERT(FALSE);
        return;
    }

    QAutoTask *pTask = QAutoTaskMan::GetInstance()->GetTask(m_pRP->nTaskID);
    if (NULL == pTask)
    {
        ASSERT(FALSE);
        return;
    }

    CStdString sImg;
    switch (pTask->GetDoWhat())
    {
    case AUTOTASK_DO_SYSSHUTDOWN:
        {
            sImg = L"qrel:images/shutdown_128.png";
            break;
        }
    case AUTOTASK_DO_EXECPROG:
        {
            sImg = L"qrel:images/cmd_128.png";
            break;
        }
    case AUTOTASK_DO_BREAKAMOMENT:
        {
            sImg = L"qrel:images/sleep_128.png";
            break;
        }
    default:
        {
            sImg = L"qrel:images/clock_128.png";
            break;
        }
    }

    GetCtrl("#img_type").set_attribute("src",sImg);
    GetCtrl("#td_time").SetText(m_pRP->tmExec.Format(L"%H:%M:%S")); // :%S
    GetCtrl("#td_tip").SetHtml(m_pRP->sMsg.IsEmpty() ? pTask->GetDoWhatString() : m_pRP->sMsg);
    GetCtrl("#td_dowhat").SetHtml(pTask->GetDoWhatString());

    if (!(m_pRP->sSound.IsEmpty()))
    { // play sound
        ::PlaySound(m_pRP->sSound,NULL,SND_FILENAME|SND_ASYNC);
    }
}

//////////////////////////
BOOL LReminderBox::ShowReminderDlg( LPTASK_REMINDER_PARAM pRP )
{
    CheckRmdList();

    ST_RMDDLG *p = NULL;
    // 每个Event在同时只能显示一个提示对话框
    // 查找现在是否正在显示中
    BOOL bExist = FALSE;
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        p = *i;
        if ( p->GetRmdParam()->nTaskID == pRP->nTaskID )
        {   
            // 如果对话框还在显示中
            // 重用这个数据
            p->SetClose(FALSE);
            bExist = TRUE;
            break;
         }
    }

    if ( !bExist )
    {
        p = new ST_RMDDLG();
        m_lstRmd.push_back(p);
    }

    // 设置新的参数
    p->SetRmdParam(pRP);

    // 对话框是否启动了
    LSingleRmdDlg *pDlg = p->GetRmdDlg();
    if (   !pDlg->IsWindow()    // 如果没有，那么启动它 
        && !pDlg->Create(NULL,WS_POPUP|WS_VISIBLE,WS_EX_TOPMOST|WS_EX_TOOLWINDOW))
    {
        return FALSE;
    }
    // 对话框已经创建，显示之
    pDlg->ShowWindow(SW_SHOW);

    return TRUE;
}

void LReminderBox::RemoveAll()
{
    for (RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); )
    {
        delete *i;
        i = m_lstRmd.erase(i);
    }
    ASSERT(m_lstRmd.size() == 0);
}

void LReminderBox::RmdDlgDestroying( LSingleRmdDlg* pDlg )
{
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        if ((*i)->GetRmdDlg() == pDlg)
        {
            (*i)->SetClose(TRUE);

            break;
        }
    }
}

void LReminderBox::CheckRmdList()
{
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); )
    {
        if ((*i)->IsClosed() && !((*i)->GetRmdDlg()->IsWindow()))
        {
            delete *i;
            i = m_lstRmd.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void LReminderBox::OnEventTimerChanged( int nEventID )
{
    ST_RMDDLG *p;
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        p = *i;
        if ( p->GetTaskID() == nEventID )
        {   
            // 如果对话框还在显示中，就关闭它
            LSingleRmdDlg *pDlg = p->GetRmdDlg();
            if (pDlg->IsWindow())
            {
                pDlg->PostMessage(WM_CLOSE);
            }
            break;
        }
    }
}

void LReminderBox::RemoveReminderDlg( int nTaskID )
{
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        if ((*i)->GetTaskID() == nTaskID)
        {
            delete *i;
            m_lstRmd.erase(i);
            break;
        }
    }
}

