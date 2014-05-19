#include "LFormPlan.h"
#include "../common/LittleTUIcmn.h"
#include "LFormStage.h"
#include "ui/QUIGlobal.h"

QUI_BEGIN_EVENT_MAP(LFormPlan,QForm)
    MENUITEM_CLICK_ID(L"mi-plan-newplan", &LFormPlan::OnMenuItemNewPlan)
    MENUITEM_CLICK_ID(L"mi-plan-edit", &LFormPlan::OnMenuItemEdit)
    MENUITEM_CLICK_ID(L"mi-plan-delete", &LFormPlan::OnMenuItemDelete)
//    MENUITEM_CLICK_ID(L"mi-plan-newstage", &LFormPlan::OnMenuItemNewStage)
    BN_STATECHANGED_NAME(L"plan-item", &LFormPlan::OnPlanItemSelectChanged)
    BN_CLICKED_NAME(L"plan-item", &LFormPlan::OnPlanItemClicked)
QUI_END_EVENT_MAP()

LFormPlan::LFormPlan(void)
{
}

LFormPlan::~LFormPlan(void)
{
}

void LFormPlan::OnMenuItemNewPlan(HELEMENT,EMenuItem mi)
{
    LPlanDlg PDlg(NULL);
    if (PDlg.DoModal() == IDOK)
    {
        NewPlanAdded(PDlg.m_pPlanInout);
    }
}

void LFormPlan::OnMenuItemEdit( HELEMENT,EMenuItem mi )
{
    QPlan* pPlan = GetCurrentPlan();
    if (NULL == pPlan)
        return;
    
    LPlanDlg PDlg(pPlan);
    if (PDlg.DoModal() == IDOK)
    {
        ETable tbl = _CurrentPlanCtl();
        RefreshPlanItem(tbl);
        OnPlanItemSelectChanged(tbl);
    }
}

void LFormPlan::OnMenuItemDelete( HELEMENT,EMenuItem mi )
{
    QPlan* pPlan = GetCurrentPlan();
    if (NULL == pPlan)
        return;

    if (XMsgBox::YesNoMsgBox(L"确定要删除这个计划吗？<br> <b .red>删除后不可以恢复</b>") == IDYES)
    {
        if (QPlanMan::GetInstance()->DeletePlan(pPlan->ID()))
        {
            ETable tblPlan = (HELEMENT)_PlanBox().FindFirstWithData(pPlan);
            if (tblPlan.is_valid())
            {
                tblPlan.destroy();
                _PlanTitle().RemoveData();
            }
            SelectPlanItem();
        }
    }
}

void LFormPlan::OnMenuItemNewStage( HELEMENT,EMenuItem mi )
{
    QPlan* pPlan = GetCurrentPlan();
    if (NULL == pPlan)
    {
        XMsgBox::OkMsgBox(L"请新建一个计划");
        return ;
    }

    LStageDlg SDlg(pPlan);
    if (SDlg.DoModal() == IDOK)
    {
        QUISendCodeTo(GetSafeHwnd(),VIEWPLAN_NOTIFY_STAGEADDED,
            (LPARAM)(SDlg.m_pStageInout));
    }
}

void LFormPlan::OnPlanItemSelectChanged(HELEMENT he)
{
    ETable tblPlan(he);
    ECtrl ctlTitle = _PlanTitle();
    QPlan *pPlan = _PlanOfTable(tblPlan);
    ctlTitle.SetData(pPlan);
    ctlTitle.SetText(pPlan->Plan());

    // 隐藏自己
    ::HTMLayoutHidePopup(GetCtrl("#list-plan>popup"));

    // 反射事件，让父窗口也处理此事件
    QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_PLANSELCHANGED, (LPARAM)pPlan);
//    ReflectThisEvent();
}

void LFormPlan::RefreshPlans()
{
    _PlanBox().DeleteAllChild();
    VecPlan* pPlans = QPlanMan::GetInstance()->GetAllPlan();
    int nPlan = pPlans->size();
    for (int i = 0; i < nPlan; ++i)
    {
        AddPlan(pPlans->at(i));
    }
    SelectPlanItem();
}

void LFormPlan::RefreshPlanItem(ETable &tblPlan)
{
    if (!tblPlan.is_valid())
    {
        ASSERT(FALSE);
        return;
    }
    QPlan *pPlan = reinterpret_cast<QPlan*>(tblPlan.GetData());
    if (NULL == pPlan)
    {
        ASSERT(FALSE);
        return;
    }

    CStdString sHtml;
    sHtml.Format(
        L"<tr> <td .plan-title>%s</td> </tr>"   // 计划名
        L"<tr> <td .plan-des>%s</td> </tr>" // 计划描述
        L"<tr> <td .plan-time>[%s] - [%s] </td> </tr>",  // 周期 
        pPlan->Plan(),
        pPlan->Des(),
        pPlan->BeginTime().Format(L"%x"),
        pPlan->EndTime().Format(L"%x"));
    tblPlan.SetHtml(sHtml);
}

ETable LFormPlan::AddPlan(QPlan *pPlan)
{
    if (NULL == pPlan)
    {
        ASSERT(FALSE);
        return NULL;
    }

    ETable ctlPlan = ETable::create("table");
    _PlanBox().append(ctlPlan);
    ctlPlan.SetData(pPlan);
    ctlPlan.set_attribute("name",L"plan-item");
    RefreshPlanItem(ctlPlan);
    return ctlPlan;
}

BOOL LFormPlan::SelectPlanItem( __in ETable tblPlan /*= NULL*/ )
{
    if ( !tblPlan.is_valid() )
    {   // 无效的话，选中第一个项目
        ECtrl ctlPlanBox = _PlanBox();
        if ( ctlPlanBox.children_count() )
        {
            tblPlan = ctlPlanBox.child(0);
        }
    }
#ifdef _DEBUG
    else
    {
        ASSERT(aux::wcseqi(L"plan-item", tblPlan.get_attribute("name")));
    }
#endif

    QPlan *pSel = nullptr;
    if (tblPlan.is_valid())
    {
        tblPlan.SetCheck(TRUE,TRUE);
        tblPlan.post_event(BUTTON_STATE_CHANGED, 0 , tblPlan);
        pSel = _PlanOfTable(tblPlan);
    }
    else
    {
        pSel = nullptr;
        _PlanTitle().SetText(L"{还没有计划}");
    }

    QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_PLANSELCHANGED, (LPARAM)pSel);

    return (nullptr != pSel);
}

void LFormPlan::OnPlanItemClicked( HELEMENT he )
{
    // 隐藏自己
    ::HTMLayoutHidePopup(GetCtrl("#list-plan>popup"));
}

void LFormPlan::NewPlanAdded( QPlan *pPlan )
{
    ETable tbl = AddPlan(pPlan);
    // 转到新建的计划项目
    SelectPlanItem(tbl);

//     if (XMsgBox::YesNoMsgBox(L"计划已经添加，是否规划一下？") == IDYES)
//     {
//         OnMenuItemNewStage(NULL,NULL);
//     }
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
LPlanDlg::LPlanDlg( QPlan *pPlan /*= NULL*/ )
    :QDialog(L"qabs:plandlgs/PlanDlg.htm")
{
    m_pPlanInout = pPlan;
    m_bEditMode = (pPlan != NULL);
}

BOOL LPlanDlg::CheckDate(__out QTime &tmBegin,__out QTime &tmEnd)
{
    tmBegin = EDate(GetCtrl("#date_begin")).GetDate();
    tmEnd = EDate(GetCtrl("#date_end")).GetDate();

//     QTime tmNow = QTime::GetCurrentTime();
//     if ((tmBegin.CompareDate(tmNow) < 0) || (tmBegin >= tmEnd))
//     {
//         XMsgBox::OkMsgBox(L"请设定一个正确的任务周期");
//         return FALSE;
//     }
    return TRUE;
}

BOOL LPlanDlg::OnDefaultButton( INT_PTR nBtn )
{
    if (IDOK != nBtn)
        return TRUE;

    if (!CheckMemberParameters())
        return TRUE;

    QTime tmBegin, tmEnd;
    if ( !CheckDate(tmBegin, tmEnd) )
        return FALSE;

    CStdString sPlan = EEdit(GetCtrl("#txt-plan")).GetText().Trim();
    if (sPlan.IsEmpty())
    {
        XMsgBox::OkMsgBox(L"计划不能为空");
        return FALSE;
    }

    // 需要测试
    BOOL bOK = FALSE;
    if (m_bEditMode)
    {
        bOK = m_pPlanInout->Edit(sPlan,
            EEdit(GetCtrl("#txt-plandes")).GetText(),
            tmBegin, tmEnd);
    }
    else
    {
        QPlan      newPlan;
        newPlan.m_sPlan = sPlan;
        newPlan.m_sDes = EEdit(GetCtrl("#txt-plandes")).GetText();
        newPlan.m_tmBegin = tmBegin;
        newPlan.m_tmEnd = tmEnd;
        QPlanMan *pMan = QPlanMan::GetInstance();
        m_pPlanInout = pMan->AddPlan(newPlan);
        bOK = (m_pPlanInout != NULL);
    }
    if (!bOK)
    {
        XMsgBox::ErrorMsgBox(L"操作失败！请重试.");
        return FALSE;
    }
    return TRUE;
}

BOOL LPlanDlg::CheckMemberParameters()
{
    BOOL bOK = !m_bEditMode;
    if (m_bEditMode)
    {
        bOK = (m_pPlanInout != NULL);
    }
    if ( !bOK )
    {
        XMsgBox::ErrorMsgBox(L"数据错误，操作失败!");
    }
    return bOK;
}

LRESULT LPlanDlg::OnDocumentComplete()
{
    if (m_bEditMode && (NULL != m_pPlanInout))
    {
        EEdit(GetCtrl("#txt-plan")).SetText(m_pPlanInout->Plan());
        EEdit(GetCtrl("#txt-plandes")).SetText(m_pPlanInout->Des());
        EDate(GetCtrl("#date_begin")).SetDate(m_pPlanInout->BeginTime());
        EDate(GetCtrl("#date_end")).SetDate(m_pPlanInout->EndTime());
    }
    return 0;
}

