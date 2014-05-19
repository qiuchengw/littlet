#include "QEventRemindDlg.h"
#include "../common/QAutoTask.h"
#include "../common/LittleTUIcmn.h"
#include "../common/ConstValues.h"
#include "../common/QTimer.h"
#include <Mmsystem.h>
#include "LDatas.h"
#include "LViewEvent.h"

#pragma comment(lib,"Winmm.lib")

// #define ID_TIMER_UPDATEREMIND  0x0fe0ef00
/*
QUI_BEGIN_EVENT_MAP(QEventRemindDlg,QDialog)
	BN_CLICKED_ID(L"btn_prev",&QEventRemindDlg::OnClkPrevRemind)
	BN_CLICKED_ID(L"btn_next",&QEventRemindDlg::OnClkNextRemind)
	BN_CLICKED_ID(L"btn_delete",&QEventRemindDlg::OnClkDeleteRemind)
	BN_CLICKED_ID(L"chk_DontRemind",&QEventRemindDlg::OnClkDontRemindAgain)
	BN_CLICKED_ID(L"btn_DontExec",&QEventRemindDlg::OnClkDontExecThis)
QUI_END_EVENT_MAP()

QEventRemindDlg::QEventRemindDlg(void)
    :QDialog(L"qabs:dlgs/")
{
	m_pCur = NULL;
}

QEventRemindDlg::~QEventRemindDlg(void)
{

}

void QEventRemindDlg::OnClkPrevRemind( HELEMENT hBtn )
{
	LPTASK_REMINDER_PARAM pPrev = PrevRemind();
	if ( m_pCur != pPrev)
	{
		ShowRemind(pPrev);
	}
}

void QEventRemindDlg::OnClkNextRemind( HELEMENT hBtn )
{
	LPTASK_REMINDER_PARAM pNext = NextRemind();
	if ( m_pCur != pNext)
	{
		ShowRemind(pNext);
	}
}

void QEventRemindDlg::OnClkDeleteRemind( HELEMENT hBtn )
{
	if (m_pCur != NULL)
	{
		DeleteRemind(m_pCur->nTaskID);
	}
}

BOOL QEventRemindDlg::OnDefaultButton( INT_PTR nID )
{
	// 隐藏，而不关闭
	ShowWindow(SW_SHOW);

	return FALSE;
}

void QEventRemindDlg::ShowRemind( LPTASK_REMINDER_PARAM pTRP )
{
	ASSERT(NULL != pTRP);
	CheckRmdList();
	SetRemind(pTRP);
	ShowWindow(SW_SHOW);
}

void QEventRemindDlg::AddRemind( LPTASK_REMINDER_PARAM pTRP )
{
	DeleteRemind(pTRP->nTaskID);
	m_lstRmd.push_back(pTRP);
	ShowRemind(pTRP);
}

QEventRemindDlg::RmdListItr QEventRemindDlg::_FindRemind( int nTaskID )
{
	RmdListItr itrEnd = _EndItr(m_lstRmd);
	for (RmdListItr itr = _BeginItr(m_lstRmd); itr != itrEnd; ++itr)
	{
		if ((*itr)->nTaskID == nTaskID)
		{
			return itr;
		}
	}
	return itrEnd;
}

void QEventRemindDlg::DeleteRemind( int nTaskID )
{
	RmdListItr itr = _FindRemind(nTaskID);
	if (itr != _EndItr(m_lstRmd))
	{
		if (m_pCur == *itr)
		{
			m_pCur = NextRemind();
			if (*itr != m_pCur)
			{
				ShowRemind(m_pCur);
			}
		}
        delete *itr;
		m_lstRmd.erase(itr);
	}
	if (m_lstRmd.size() <= 0)
	{
		ShowWindow(SW_HIDE);
	}
}

LPTASK_REMINDER_PARAM QEventRemindDlg::NextRemind(  )
{
	RmdListItr itr;
	if (NULL != m_pCur)
	{
		itr = _FindRemind(m_pCur->nTaskID);
		if (itr != _EndItr(m_lstRmd))
		{
			++itr;
			if (itr != _EndItr(m_lstRmd))
				return *itr;
		}
	}
	if (m_lstRmd.size())
	{
		return m_lstRmd.front();
	}
	return NULL;
}

LPTASK_REMINDER_PARAM QEventRemindDlg::PrevRemind()
{
	if (NULL != m_pCur)
	{
		RmdList::reverse_iterator itrEnd = m_lstRmd.rend();
		for(RmdList::reverse_iterator itr = m_lstRmd.rbegin();
			itr != itrEnd; ++itr)
		{
			if (m_pCur == (*itr))
			{
				++itr;
				if (itrEnd != itr)
				{
					return *itr;
				}
				break;
			}
		}
	}
	if (m_lstRmd.size())
	{
		return m_lstRmd.front();
	}
	return NULL;
}

void QEventRemindDlg::CheckRmdList()
{
	QTime tmNow = QTime::GetCurrentTime();
	// 接下来两秒内执行的任务的提示信息都被删掉
	// 因为反正用户也看不到的
	tmNow += QTimeSpan(0,0,0,2);
	RmdListItr itrEnd = _EndItr(m_lstRmd);
	for (RmdListItr itr = _BeginItr(m_lstRmd); itr != itrEnd; ++itr)
	{
		if ((*itr)->tmExec <= tmNow)
		{
            delete *itr;
			itr = m_lstRmd.erase(itr);
			if (itr == itrEnd)
				break;
		}
	}
}

void QEventRemindDlg::SetRemind( LPTASK_REMINDER_PARAM pTRP )
{
	m_pCur = pTRP;
	if (pTRP != NULL)
	{
		ECtrl eExec = GetCtrl("table#id_head td.time");
		eExec.SetText(pTRP->tmExec.Format(L"%Y/%m/%d %H:%M:%S"));
		ECtrl eWhat = GetCtrl("#td_message");
		if (pTRP->sMsg.IsEmpty())
		{
            QAutoTask* pTask = QAutoTaskMan::GetInstance()->GetTask(pTRP->nTaskID);
			ASSERT(pTask != NULL);
			if (NULL != pTask)
				eWhat.SetText(pTask->GetDoWhatString());
			else
				eWhat.SetText(L"");
		}
		else
		{
			eWhat.SetText(pTRP->sMsg);
		}
		if (!(pTRP->sSound.IsEmpty()))
		{ // play sound
			::PlaySound(pTRP->sSound,NULL,SND_FILENAME|SND_NOWAIT|SND_ASYNC);
		}
		ECheck(GetCtrl("#chk_DontRemind")).SetCheck(
			!(QAutoTaskMan::GetInstance()->IsTaskReminderEnabled(pTRP->nTaskID)));
	}
	else
	{
		ECtrl eExec = GetCtrl("table#id_head td.time");
		eExec.SetText(L"---");
		ECtrl eWhat = GetCtrl("#td_message");
		eWhat.SetText(L"---");
		ECheck(GetCtrl("#chk_DontRemind")).SetCheck(FALSE);
		ShowWindow(SW_HIDE);
	}
}

void QEventRemindDlg::OnClkDontRemindAgain( HELEMENT hBtn )
{
	if (NULL != m_pCur)	
	{
		BOOL bEnable = ECheck(hBtn).IsChecked();

		// notify mainwnd
        QUIPostCodeToMainWnd(MWND_CMD_AUTOTASKOPERATION,
			(LPARAM)QUIGetUserMsgParam(TASK_OP_TOGGLEREMIND,m_pCur->nTaskID));
	}
}

void QEventRemindDlg::OnClkDontExecThis( HELEMENT hBtn )
{
	if (NULL == m_pCur)
	{
		return;
	}

	if (XMsgBox::YesNoMsgBox(L"任务将不会执行<br/> 确定吗？") == IDYES)
	{
		QUIPostCodeToMainWnd(MWND_CMD_AUTOTASKOPERATION,
			(LPARAM)QUIGetUserMsgParam(TASK_OP_JUMPOVEREXEC,m_pCur->nTaskID));

		LPTASK_REMINDER_PARAM pNex = NextRemind();
		SetRemind( (pNex == m_pCur) ? NULL : pNex );
		DeleteRemind(m_pCur->nTaskID);
	}
}

*/
//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(QSingleRmdDlg,QDialog)
    BN_CLICKED_ID(L"btn_skip",&QSingleRmdDlg::OnClkDontExecThis)
QUI_END_EVENT_MAP()

QSingleRmdDlg::QSingleRmdDlg( LPTASK_REMINDER_PARAM pRP )
    :QDialog(L"qabs:dlgs/rmddlg.htm")
{
    m_pRP = pRP;
    m_nCountDown = 300;
}

QSingleRmdDlg::~QSingleRmdDlg( void )
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

void QSingleRmdDlg::OnClkDontRemindAgain( HELEMENT hBtn )
{

}

void QSingleRmdDlg::OnClkDontExecThis( HELEMENT hBtn )
{
    QAutoTaskMan *pMan = QAutoTaskMan::GetInstance();
    if (pMan->JumpoverTaskThisExec(m_pRP->nTaskID))
    {
        // 转到LEvent上的
        QUISendCodeTo(LDatas::GetInstance()->GetViewEventPtr()->GetSafeHwnd(),
            EVENTWND_NOTIFY_USERJUMPEVENTEXEC,(LPARAM)m_pRP->nTaskID);
        // 关闭掉自己
        PostMessage(WM_CLOSE);
    }
    else
    {
        ECtrl(hBtn).ShowTooltip(L"无法跳过此次任务执行");
    }
}

void QSingleRmdDlg::OnDestroy()
{
    LReminderBox::GetInstance()->RmdDlgDestroying(this);
}

void QSingleRmdDlg::SetRmdParam( LPTASK_REMINDER_PARAM pRP )
{
    m_pRP = pRP;

    if ( IsWindow() )
    {
        StartCountDown();

        RefreshRemindMessage();
    }
}

void QSingleRmdDlg::OnTimer( UINT nTimerID )
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

LRESULT QSingleRmdDlg::OnDocumentComplete()
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

void QSingleRmdDlg::StartCountDown()
{
    if (NULL == m_pRP)
    {
        ASSERT(FALSE);
        return;
    }
    // 也许是重用对话框，重新设置的数据，那么先停止之前的倒数定时器
    KillTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID);

    // 重新启动倒数定时器
    m_nCountDown = m_pRP->nSeconds;

    SetTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID, 1000, NULL);
}

void QSingleRmdDlg::RefreshRemindMessage()
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
    GetCtrl("#td_tip").SetHtml(m_pRP->sMsg);
    GetCtrl("#td_dowhat").SetHtml(pTask->GetDoWhatString());

    if (!(m_pRP->sSound.IsEmpty()))
    { // play sound
        ::PlaySound(m_pRP->sSound,NULL,SND_FILENAME|SND_ASYNC);
    }
}

//////////////////////////////////////////////////////////////////////////
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
    QSingleRmdDlg *pDlg = p->GetRmdDlg();
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

void LReminderBox::RmdDlgDestroying( QSingleRmdDlg* pDlg )
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
            QSingleRmdDlg *pDlg = p->GetRmdDlg();
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

