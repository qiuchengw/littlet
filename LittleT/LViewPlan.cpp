#include "LViewPlan.h"
#include "LDatas.h"
#include "LFormPlan.h"

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LViewPlan,LittleTView)
//    BN_CLICKED_ID(L"tbl-curgoal",&LViewPlan::OnClkCurrentGoal)
    BN_CLICKED_ID(L"id_btn_newplan",&LViewPlan::OnClkNewPlan)
    BN_CLICKED_NAME(L"goal-item",&LViewPlan::OnClkGoalCtl)
    BN_CLICKED_ID(L"CHK_SHOW_WAITING",&LViewPlan::OnClkChkGoalitemShow)
    BN_CLICKED_ID(L"CHK_SHOW_FINISHED", &LViewPlan::OnClkChkGoalitemShow)
    CHAIN_HLEVENT_TO_FORM(&m_formPlan,"#id-plan-wrapper")
    CHAIN_HLEVENT_TO_FORM(&m_formGoal,"#id-goal-box")
    CHAIN_HLEVENT_TO_FORM(&m_formStage,"#id-stage-box-wrapper")
QUI_END_EVENT_MAP()

QUI_BEGIN_REFLECT_EVENT_MAP(LViewPlan, LittleTView)
//    BN_STATECHANGED_NAME(L"plan-item", &LViewPlan::OnPlanSelectChanged)
//     BN_CLICKED_ID(L"btn-next-stage", &LViewPlan::OnStageItemChanged)
//     BN_CLICKED_ID(L"btn-prev-stage", &LViewPlan::OnStageItemChanged)
QUI_END_REFLECT_EVENT_MAP()

LViewPlan::LViewPlan(void)
{
    LDatas::GetInstance()->SetViewPlanPtr(this);
}

LViewPlan::~LViewPlan(void)
{
}

void LViewPlan::OnKeyDown( UINT nChar,UINT nRepCnt,UINT nFlags )
{
    if (GetKeyState(VK_CONTROL) < 0)
    {
        SetMsgHandled(littlet::OnCtrlKeyPressDown(nChar,nRepCnt,nFlags));
        return;
    }
    SetMsgHandled(FALSE);
}

void LViewPlan::OnClkNewPlan( HELEMENT hBtn )
{
//     LPlanDlg PDlg(NULL);
//     if (PDlg.DoModal() == IDOK)
//     {
//         m_formPlan.NewPlanAdded(PDlg.m_pPlanInout);
//     }
    m_formPlan.OnMenuItemNewPlan(NULL,NULL);
}

LRESULT LViewPlan::OnDocumentComplete()
{
    m_formPlan.RefreshPlans();

    NotifyPlanNumChanged();

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// 各种选择变化
void LViewPlan::OnPlanSelectChanged( HELEMENT he )
{
//     QPlan *pPlan = m_formPlan.GetCurrentPlan();
//     m_formStage.SetPlan(pPlan);
// 
//     // 在这儿刷新plan的数目(不好，但是方便)
//     NotifyPlanNumChanged();

    OnPlanSelect((LPARAM)m_formPlan.GetCurrentPlan());
}

void LViewPlan::OnPlanSelect( LPARAM lParam )
{
//    QPlan *pPlan = m_formPlan.GetCurrentPlan();
    QPlan *pPlan = reinterpret_cast<QPlan*>(lParam);

    m_formStage.SetPlan(pPlan);

    // 在这儿刷新plan的数目(不好，但是方便)
    NotifyPlanNumChanged();
}

//////////////////////////////////////////////////////////////////////////
void LViewPlan::RefreshCurrentGoal()
{
//     QGoal* pGoal = m_formGoal.GetCurrentGoal();
//     ETable ctlGoal = _CurrentGoalCtl();
//     ctlGoal.SetData(pGoal);
// 
//     if (NULL != pGoal)
//     {
//         ECtrl ctl = ctlGoal.find_first("#id-goal");
//         QString sText;
//         sText.Format(L"%s  <b .red>[%d]</b>", pGoal->Goal(), 
//             pGoal->GetGoalItemUnfinishedNum()/*, pGoal->GetGoalItemNum()*/);
//         ctl.SetHtml(sText);
// //        ctl = ctlGoal.find_first("#id-goalitem-checked");
// //         QString sHtml;
// //         sHtml.Format(L"<div .number-text>%d / %d</div>", 
// //             pGoal->GetGoalItemUnfinishedNum(), pGoal->GetGoalItemNum());
// //         ctl.SetHtml(sHtml);
//         ctl = GetCtrl("#id-goal-des");
//         ctl.SetText(pGoal->Des());
//     }
//     else
//     {
//         ECtrl ctl = ctlGoal.find_first("#id-goal");
//         ctl.SetText(L"【没有目标不是上进青年】");
// //         ctl = ctlGoal.find_first("#id-goalitem-checked");
// //         ctl.SetText(L"-");
//         ctl = GetCtrl("#id-goal-des");
//         ctl.SetText(L"快来添加一个目标吧^_^");
//     }
}

// 选中一个goal时，显示其下的goal-item
void LViewPlan::OnClkGoalCtl( HELEMENT he )
{
}

//////////////////////////////////////////////////////////////////////////
// stage 添加、删除、变化
void LViewPlan::OnStageAdded( LPARAM lParam )
{
}

void LViewPlan::OnStageDeleted( LPARAM lParam )
{
    //OnStageSelChanged((LPARAM)m_formStage.GetCurrentStage());
}

void LViewPlan::OnStageSelChanged( LPARAM lParam )
{
    QStage *pStage = reinterpret_cast<QStage*>(lParam);
    m_formGoal.SetStage(pStage);
}

//////////////////////////////////////////////////////////////////////////
// goal 添加和删除
void LViewPlan::OnGoalAdded( LPARAM lParam )
{
    // 显示goal
    //ShowGoalPane(TRUE);
    m_formStage.RefreshCurrentStageItem();
}

void LViewPlan::OnGoalDeleted( LPARAM lParam )
{
    // 显示goal
    //ShowGoalPane(TRUE);
    m_formStage.RefreshCurrentStageItem();
}

void LViewPlan::OnGoalSelChanged( LPARAM lParam )
{

}

//////////////////////////////////////////////////////////////////////////
// goalitem 添加、删除、状态改变
void LViewPlan::OnGoalSubitemAdded( LPARAM lParam )
{
    QGoalItem *pGoalItem = reinterpret_cast<QGoalItem*>(lParam);
    ASSERT(NULL != pGoalItem);
    m_formGoal.NewGoalSubItemAdded(pGoalItem);

    RefreshCurrentGoal();
}

void LViewPlan::OnGoalSubitemDeleted( LPARAM lParam )
{
    QGoal* pGoalParent = reinterpret_cast<QGoal*>(lParam);
    ASSERT(NULL != pGoalParent);
    m_formGoal.GoalSubItemDelete(pGoalParent);
}

void LViewPlan::OnGoalSubitemStatusChanged( LPARAM lParam )
{
    QGoalItem* pGoalItem = reinterpret_cast<QGoalItem*>(lParam);
    ASSERT(NULL != pGoalItem);
    m_formGoal.GoalSubItemStatusChanged(pGoalItem);

    // 刷新当前的goal显示
    RefreshCurrentGoal();
}

void LViewPlan::NotifyPlanNumChanged()
{
    QUIPostCodeToMainWnd(MWND_NOTIFY_PLANNUMCHANGED, 
        QPlanMan::GetInstance()->GetWorkingPlanNum());
}

void LViewPlan::OnClkChkGoalitemShow( HELEMENT he )
{
    BOOL bShowWaiting = ECheck(GetCtrl("#CHK_SHOW_WAITING")).IsChecked();
    BOOL bShowDone = ECheck(GetCtrl("#CHK_SHOW_FINISHED")).IsChecked();
    m_formGoal.SetGoalitemShow(bShowDone, bShowWaiting);
}
