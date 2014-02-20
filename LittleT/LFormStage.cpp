#include "LFormStage.h"
#include "../common/LittleTUIcmn.h"
#include "LFormGoal.h"
#include "QResIconsDlg.h"

QUI_BEGIN_EVENT_MAP(LFormStage,QForm)
//     BN_CLICKED_ID(L"btn-prev-stage", &LFormStage::OnClkBtnPrevStage)
//     BN_CLICKED_ID(L"btn-next-stage", &LFormStage::OnClkBtnNextStage)
//    BN_STATECHANGED_NAME(L"stage-item", &LFormStage::OnStageItemSelectChanged)
    MENUITEM_CLICK_ID(L"mi-stage-newstage", &LFormStage::OnMenuItemClkNewStage)
//    MENUITEM_CLICK_ID(L"mi-stage-newgoal", &LFormStage::OnMenuItemClkNewGoal)
    MENUITEM_CLICK_ID(L"mi-stage-edit", &LFormStage::OnMenuItemClkEdit)
    MENUITEM_CLICK_ID(L"mi-stage-delete", &LFormStage::OnMenuItemClkDelete)
    UNKNOWN_SELECTION_CHANGED_ID(L"id_stage_nav", &LFormStage::OnPageNaveSelectChanged)
QUI_END_EVENT_MAP()

LFormStage::LFormStage(void)
{
    m_pPlan = NULL;
}

LFormStage::~LFormStage(void)
{
    ClearMem();
}

// void LFormStage::OnClkBtnPrevStage( HELEMENT )
// {
//     ETable tblCurrent = _CurrentStageItem();
//     if (tblCurrent.is_valid())
//     {
//         ETable tblPrev = tblCurrent.prev_sibling();
//         if ( !tblPrev.is_valid())
//         {
//             tblPrev = tblCurrent.last_sibling();
//         }
// 
//         if (tblPrev.is_valid())
//         {
//             tblPrev.SetCheck(TRUE,TRUE);
//             RefreshStageBar();
// 
//             QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_STAGESELCHANGED, 
//                 (LPARAM)GetCurrentStage());
//         }
//     }
// }
// 
// void LFormStage::OnClkBtnNextStage( HELEMENT )
// {
//     ETable tblCurrent = _CurrentStageItem();
//     if (tblCurrent.is_valid())
//     {
//         ETable tblNext = tblCurrent.next_sibling();
//         if (!tblNext.is_valid())
//         {
//             tblNext = tblCurrent.first_sibling();
//         }
// 
//         if (tblNext.is_valid())
//         {
//             tblNext.SetCheck(TRUE,TRUE);
//             RefreshStageBar();
//             QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_STAGESELCHANGED, 
//                 (LPARAM)GetCurrentStage());
//             //ReflectThisEvent();
//         }
//     }
// }

void LFormStage::SetPlan( QPlan* pPlan )
{
    if ((NULL != pPlan) && (pPlan == m_pPlan))
        return;

    m_pPlan = pPlan;
    _StageBox().DeleteAllChild();
    ClearMem();

    int nStage = 0;
    if (NULL != pPlan)
    {
        nStage = pPlan->GetAllStage(m_stages);
        for (int i = 0; i < nStage; ++i)
        {
            AddStage(m_stages[i]);
        }
    }

    SelectStageItem();
}

ETable LFormStage::AddStage( QStage* pStage )
{
    ETable tblStage = ETable::create("table");
    _StageBox().append(tblStage);
    tblStage.set_attribute("name",L"stage-item");
    tblStage.SetData(pStage);
    RefreshStageItem(tblStage);
    return tblStage;
}

void LFormStage::RefreshStageItem( ETable &tblStage )
{
    if ( !tblStage.is_valid() )
    {
        ASSERT(FALSE);
        return;
    }
    QStage *pStage = reinterpret_cast<QStage*>(tblStage.GetData());
    if (NULL == pStage)
    {
        ASSERT(FALSE);
        return;
    }

    QString sTimeTitle;
    sTimeTitle.Format(L"%d%% (%s)", 
        pStage->GetTimeProcess(), 
        pStage->GetTimeFlee());
    QString sProgressTitle;
    sProgressTitle.Format(L"%d%% (%d / %d)",
        pStage->GetStageProcess(),
        pStage->GetFinishedGoalNum(),
        pStage->GetGoalNum());
    QString sHtml;
    sHtml.Format(
        L"<tr style=\"height:*;\">"
        L"  <td rowspan=3 .icon><picture src=\"%s\"/></td>"   // %s, icon path
        L"  <td id=\"stage-title\">%s <b .red>[%d]</b></td>"     // %s stage, %d goal num
//        L"  <td rowspan=3 id=\"stage-goalnum\"><div .number-text>%d</div></td>"  // %d, goalnum
        L"</tr>"
        L"<tr>"
        L"  <td .des-text .stage-des>%s</td>"  // %s, stage des
        L"</tr>"
        L"<tr>"
        L"  <td .time-text .stage-period>%s ~ %s</td>"    // %s %s, begin/end time
        L"</tr>",
        pStage->IconPath(),
        pStage->Stage(), pStage->GetWorkingGoalNum(),
//        pStage->GetGoalNum(),
        pStage->Des(),
        pStage->BeginTime().Format(L"%x"), pStage->EndTime().Format(L"%x"));
//     sHtml.Format(
//         L"<tr>"
//         L"  <td id=\"stage-title\"  colspan=2>%s</td>"       // %s, stage title
//         L"  <td id=\"stage-goalnum\" .number-text>%d</td>"    // %s 未完成的goal
//         L"</tr>"
//         L"<tr>"
//         L"  <td .prgs-time><progress red id=\"prgs-stage-time\" value=\"%d\" /></td>"  // %d: progress;
//         L"  <td .time-text>%s</td>" // %s %s %s,用时， begin / end time
//         L"</tr>"
//         L"<tr>"
//         L"  <td .prgs-prgs><progress id=\"prgs-stage-time\" value=\"%d\" /></td>"
//         L"  <td .time-text>%s</td>" // %s 进行度
//         L"</tr>"
// //        L"  <td>"
// //         L"      <widget type=\"button-menu\" name=\"stage-menu\" with-icon>"
// //         L"          <menu>"
// //         L"              <li name=\"mi-stage-newstage\">添加</li>"
// //         L"              <li name=\"mi-stage-edit\">修改</li>"
// //         L"              <li name=\"mi-stage-delete\">删除</li>"
// //         L"          </menu>"
// //         L"      </widget>"
// //        L"  </td>"
//         L"</tr>",
//         pStage->Stage(),
//         pStage->GetGoalNum(),
//         pStage->GetTimeProcess(), 
//         sTimeTitle, //pStage->BeginTime().Format(L"%x"), pStage->EndTime().Format(L"%x"),
//         pStage->GetWorkingGoalNum(), sProgressTitle);
    tblStage.SetHtml(sHtml);
}

BOOL LFormStage::SelectStageItem(__in ETable tblStage)
{
    if ( !tblStage.is_valid() )
    {   
        // 无效的话，选中第一个项目
        tblStage = _CurrentStageItem();
        if (!tblStage.is_valid())
        {
            ECtrl tblStageBox = _StageBox();
            if (tblStageBox.children_count())
            {
                // 选中第一个
                tblStage = tblStageBox.child(0);
                tblStage.SetCheck(TRUE,TRUE);
            }
        }
    }
#ifdef _DEBUG
    else
    {
        ASSERT(aux::wcseqi(L"stage-item", tblStage.get_attribute("name")));
    }
#endif
    QStage* pStage = NULL;
    if (tblStage.is_valid())
    {
        //tblStage.SetCheck(TRUE,TRUE);
        struct _forall : public htmlayout::dom::callback
        {
            HELEMENT h_;
            _forall (HELEMENT h) : h_(h)
            {

            }

            virtual bool on_element(HELEMENT h)
            {
                ECtrl ctl(h);
                ctl.ShowCtrl( (h == h_) ? SHOW_MODE_SHOW : SHOW_MODE_COLLAPSE);
                return false;
            }
        };

        _StageBox().select_elements(&_forall(tblStage), L"table[name=\"stage-item\"]");

        tblStage.SetCheck(TRUE, TRUE);

        pStage = _StageOfCtl(tblStage);
    }

    RefreshPageNum();

    QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_STAGESELCHANGED, 
        (LPARAM)pStage);

//    RefreshStageBar();
    return TRUE;
}

// void LFormStage::RefreshStageBar()
// {
//     int nNO = 0;
//     ETable tblCurrentStage = _CurrentStageItem();
//     QString sPeriod = L"---";
//     QStage *pStage = _CurrentStage();
//     if (NULL != pStage)
//     {
//         sPeriod.Format(L"%s - %s", 
//             pStage->BeginTime().Format(L"%x"),
//             pStage->EndTime().Format(L"%x"));
//     }
//     _StagePeriod().SetText(sPeriod);
// 
//     if (tblCurrentStage.is_valid())
//     {
//         nNO = tblCurrentStage.index() + 1;
//     }
//     _StageNO().SetText(L"%d / %d", nNO, _StageBox().children_count());
// }

void LFormStage::NewStageAdded( QStage* pStage )
{
    ASSERT(NULL != pStage);
    m_stages.push_back(pStage);
    ETable tbl = AddStage(pStage);

    RefreshPageNum();

    SelectStageItem(tbl);
}

void LFormStage::OnMenuItemClkNewGoal( HELEMENT, EMenuItem mi )
{
//     QStage * pStage = _CurrentStage();
//     if (NULL != pStage)
//     {
//         LGoalDlg GDlg(pStage);
//         if (GDlg.DoModal() == IDOK)
//         {
//             QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_GOALADDED,
//                 (LPARAM)&(GDlg.m_NewGoal));
// 
//             // 刷新当前显示的stage
//             RefreshStageItem(_CurrentStageItem());
//         }
//     }
}

void LFormStage::OnMenuItemClkEdit( HELEMENT, EMenuItem mi )
{
    QStage *pStage = _CurrentStage();
    if (NULL != pStage)
    {
        LStageDlg sDlg(pStage);
        if (sDlg.DoModal() == IDOK)
        {
            RefreshStageItem(_CurrentStageItem());
        }
    }
}

void LFormStage::OnMenuItemClkDelete( HELEMENT, EMenuItem mi )
{
    QStage *pStage = _CurrentStage();
    if (NULL != pStage)
    {
        QString sTip;
        sTip.Format(
            L"确定要删除阶段【%s】吗？<br />其下有【%d】个目标。<br />"
            L"<b .red>删除后不可以恢复</b>",
            pStage->Stage(), pStage->GetGoalNum());
        if (XMsgBox::YesNoMsgBox(sTip) == IDYES)
        {
            int nStageID = pStage->ID();

            if (DeleteStage(pStage))
            {
                // 告诉上级
                QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_STAGEDELETED, 
                    (LPARAM)nStageID);
            }
        }
    }
}

BOOL LFormStage::DeleteStage( QStage* pStage )
{
    for (VecStageItr i = m_stages.begin(); i < m_stages.end(); i++)
    {
        if (pStage == (*i))
        {
            if ( m_pPlan->DeleteStage(pStage))
            {
                ETable tblItem = _StageBox().FindFirstWithData(pStage);
                ASSERT(tblItem.is_valid());
                tblItem.destroy();
                delete pStage;
                m_stages.erase(i);

                // 如果确实删掉了，那么重新选择当前stage
                SelectStageItem();

                return TRUE;
            }
            break;
        }   
    }
    return FALSE;
}

void LFormStage::OnMenuItemClkNewStage( HELEMENT, EMenuItem mi )
{
    if (NULL == m_pPlan)
    {
        XMsgBox::OkMsgBox(L"请新建一个计划");
        return ;
    }

    LStageDlg SDlg(m_pPlan);
    if (SDlg.DoModal() == IDOK)
    {
        QUISendCodeTo(GetSafeHwnd(),VIEWPLAN_NOTIFY_STAGEADDED,
            (LPARAM)(SDlg.m_pStageInout));

        NewStageAdded(SDlg.m_pStageInout);
    }
}

void LFormStage::ClearMem()
{
    std::for_each(m_stages.begin(), m_stages.end(),
        [](QStage* p) { delete p; } );
    m_stages.clear();
}

void LFormStage::OnStageItemSelectChanged( HELEMENT he )
{
    SelectStageItem(he);
//     QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_STAGESELCHANGED, 
//         (LPARAM)GetCurrentStage());
}

void LFormStage::RefreshCurrentStageItem()
{
    RefreshStageItem(_CurrentStageItem());
}

void LFormStage::OnPageNaveSelectChanged( HELEMENT he, HELEMENT h2 )
{
    int idx = _PageNav().GetCurPage();
    if (-1 != idx)
    {
        ETable tblStage = _StageBox().child(idx - 1);
        if (tblStage.is_valid())
        {
            SelectStageItem(tblStage);
        }
    }
}

void LFormStage::RefreshPageNum()
{
    ECtrl tBox = _StageBox();
    EPageCtrl ctlPage = _PageNav();
    ctlPage.SetPageNum(tBox.children_count());
    if (ctlPage.GetPageNum() > 0)
    {
        ETable tbl = _CurrentStageItem();
        if (tbl.is_valid())
        {
            ctlPage.SetCurPage(tbl.index() + 1);
        }
        else
        {
            ctlPage.SetCurPage(0);
        }

//         tBox.ShowCtrl(SHOW_MODE_HIDE);
//         tBox.ShowCtrl(SHOW_MODE_SHOW);
    }
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LStageDlg, QDialog)
    BN_CLICKED_ID(L"stage-icon", &LStageDlg::OnClkStageIcon)
QUI_END_EVENT_MAP()

LStageDlg::LStageDlg( QPlan *pPlan )
    :QDialog(L"qabs:plandlgs/StageDlg.htm")
{
    ASSERT(NULL != pPlan);
    m_pPlan = pPlan;
    m_pStageInout = NULL;
    m_bEditMode = FALSE;
    m_nIconID = INVALID_ID;
}

LStageDlg::LStageDlg( QStage* pStage )
    :QDialog(L"qabs:plandlgs/StageDlg.htm")
{
    ASSERT(NULL != pStage);
    m_pPlan = NULL;
    m_pStageInout = pStage;
    m_bEditMode = TRUE;
}

BOOL LStageDlg::CheckDate(__out QTime &tmBegin,__out QTime &tmEnd)
{
    tmBegin = EDate(GetCtrl("#date_begin")).GetDate();
    tmEnd = EDate(GetCtrl("#date_end")).GetDate();

//     QTime tmNow = QTime::GetCurrentTime();
//     if (m_bEditMode)
//     {
//         if ( tmBegin.CompareDate(m_pStageInout->BeginTime()) < 0 )
//         {
//             XMsgBox::OkMsgBox(L"起始时间不能比原来的时间还小啊！");
//             return FALSE;
//         }
// 
//         if (tmBegin > tmEnd)
//         {
//             XMsgBox::OkMsgBox(L"请设定一个正确的任务周期");
//             return FALSE;
//         }
//     }
//     else
//     {
//         if ((tmBegin.CompareDate(tmNow) < 0) || (tmBegin >= tmEnd))
//         {
//             XMsgBox::OkMsgBox(L"请设定一个正确的任务周期");
//             return FALSE;
//         }
//     }
    return TRUE;
}

BOOL LStageDlg::OnDefaultButton( INT_PTR nBtn )
{
    if (IDOK != nBtn)
        return TRUE;

    if (!CheckMemberParameters())
        return TRUE;

    QTime tmBegin, tmEnd;
    if ( !CheckDate(tmBegin, tmEnd) )
        return FALSE;

    QString sStage = EEdit(GetCtrl("#txt-stage")).GetText();
    sStage = sStage.Trim();
    if (sStage.IsEmpty())
    {
        XMsgBox::OkMsgBox(L"阶段名不能为空");
        return FALSE;
    }
    // 需要测试
    BOOL bOK = FALSE;
    if (m_bEditMode)
    {
        bOK = m_pStageInout->Edit(
                sStage,
                EEdit(GetCtrl("#txt-stagedes")).GetText(),
                tmBegin, tmEnd);
    }
    else
    {
        QStage newStage;
        newStage.m_nPlanID = m_pPlan->ID();
        newStage.m_sStage = sStage;
        newStage.m_sDes = EEdit(GetCtrl("#txt-stagedes")).GetText();
        newStage.m_tmBegin = tmBegin;
        newStage.m_tmEnd = tmEnd;
        m_pStageInout = m_pPlan->AddStage(newStage);
        bOK = (NULL != m_pStageInout);
    }

    if (!bOK)
    {
        XMsgBox::ErrorMsgBox(L"操作失败！请重试.");
        return FALSE;
    }
    else
    {
        m_pStageInout->SetIcon(m_nIconID);
    }
    return TRUE;
}

BOOL LStageDlg::CheckMemberParameters()
{
    BOOL bOK = FALSE;
    if (m_bEditMode)
    {
        bOK = (m_pStageInout != NULL);
    }
    else
    {
        bOK = (m_pPlan != NULL);
    }
    if ( !bOK )
    {
        XMsgBox::ErrorMsgBox(L"数据错误，操作失败!");
    }
    return bOK;
}

LRESULT LStageDlg::OnDocumentComplete()
{
    if (m_bEditMode && (NULL != m_pStageInout))
    {
        ECtrl eIcon = _StageIcon();
        eIcon.set_attribute("src", m_pStageInout->IconPath());
        m_nIconID = m_pStageInout->IconID();

        EEdit(GetCtrl("#txt-stage")).SetText(m_pStageInout->Stage());
        EEdit(GetCtrl("#txt-stagedes")).SetText(m_pStageInout->Des());
        EDate(GetCtrl("#date_begin")).SetDate(m_pStageInout->BeginTime());
        EDate(GetCtrl("#date_end")).SetDate(m_pStageInout->EndTime());
    }

    if ( !m_bEditMode && (NULL != m_pPlan))
    {   // stage周期默认设置为plan的周期
        EDate(GetCtrl("#date_begin")).SetDate(m_pPlan->BeginTime());
        EDate(GetCtrl("#date_end")).SetDate(m_pPlan->EndTime());
        m_nIconID = INVALID_ID;
    }
    return 0;
}

void LStageDlg::OnClkStageIcon( HELEMENT he )
{
    QResIconsDlg RIDlg;
    if (RIDlg.DoModal() == IDOK)
    {
        _StageIcon().set_attribute("src", RIDlg.m_sIconPath);
        m_nIconID = RIDlg.m_nIconID;
    }
}

