#include "LFormGoalItem.h"
#include "../common/LittleTUIcmn.h"

QUI_BEGIN_EVENT_MAP(LFormGoalItem,QForm)
    BN_CLICKED_NAME(L"btn_goalitem_delete", &LFormGoalItem::OnClkDeleteItem)
    BN_CLICKED_NAME(L"chk_checkitem", &LFormGoalItem::OnClkChkCheckItem)
    BN_CLICKED_NAME(L"btn_goalitem_title", &LFormGoalItem::OnClkGoalitemTitle)
    BN_CLICKED_ID(L"btn_new_goalitem", &LFormGoalItem::OnClkNewGoalitem)
    CHAIN_HLEVENT_TO_FORM(&m_widgetGoalitemOP, "#id_popup_goalitem")
QUI_END_EVENT_MAP()

QUI_BEGIN_REFLECT_EVENT_MAP(LFormGoalItem, QForm)
    BN_CLICKED_ID(L"id_bar_ok", &LFormGoalItem::OnClkNewOrEdit)
QUI_END_REFLECT_EVENT_MAP()

LFormGoalItem::LFormGoalItem(void)
{
    m_pGoal = NULL;
    m_bShowFinished = FALSE;
    m_bShowUnfinished = TRUE;
}

LFormGoalItem::~LFormGoalItem(void)
{
    ClearMem();
}

void LFormGoalItem::RefreshGoalBox()
{
    _GoalItemBox().DeleteAllChild();

//     BOOL bShowWaiting = ECheckBox(GetCtrl("#CHK_SHOW_WAITING")).IsChecked();
//     BOOL bShowDone = ECheckBox(GetCtrl("#CHK_SHOW_FINISHED")).IsChecked();

    VecGoalItemItr iEnd = m_goalitems.end();
//     if (bShowDone && bShowWaiting)
//     {
//         while ( i != iEnd)
//         {
//             AddGoalItem(*i);
//             ++i;
//         }
//     }
    if (m_bShowUnfinished)
    {
        for (VecGoalItemItr i = m_goalitems.begin(); i != iEnd; ++i)
        {
            if ( !(*i)->IsDone() )
            {
                AddGoalItem(*i);
            }
        }
    }

    if (m_bShowFinished)
    {
        for (VecGoalItemItr i = m_goalitems.begin(); i != iEnd; ++i)
        {
            if ( (*i)->IsDone() )
            {
                AddGoalItem(*i);
            }
        }
    }
    
}

ETable LFormGoalItem::AddGoalItem( QGoalItem* pGoalItem )
{
    ETable tblGoalItem = ETable::create("table");
    _GoalItemBox().append(tblGoalItem);
    tblGoalItem.set_attribute("name",L"goalitem-item");
    tblGoalItem.set_attribute("class", L"goalitem");
    tblGoalItem.SetData(pGoalItem);
    RefreshGoalItem(tblGoalItem);
    return tblGoalItem;
}

void LFormGoalItem::RefreshGoalItem( ETable& tblGoalItem )
{
    if ( !tblGoalItem.is_valid() )
    {
        ASSERT(FALSE);
        return ;
    }
    QGoalItem* pGoalItem = reinterpret_cast<QGoalItem*>(tblGoalItem.GetData());
    if (NULL == pGoalItem)
    {
        ASSERT(FALSE);
        return;
    }

    QString sNoteAndClock;
    if (pGoalItem->IsHasTimer())
    {
        sNoteAndClock = L"<td .qbtn name=\"btn_clock\" />";
    }
    // <td .qbtn name="btn_note" />
    QString sHtml;
    sHtml.Format(
        L"<tr>"
        L"  <td .item-exec><widget type=\"checkbox\" name=\"chk_checkitem\" %s/></td>" // %s checked
        L"  <td .item-todo name=\"btn_goalitem_title\">%s</td>"    // %s, goalitem 
        L"  %s"    // note and timer
        L"  <td .close-btn name=\"btn_goalitem_delete\">r</td>"
        L"</tr>",
        pGoalItem->IsDone() ? L"checked" : L"",
        pGoalItem->Item(),
        sNoteAndClock);
    tblGoalItem.SetHtml(sHtml);
}

void LFormGoalItem::ClearMem()
{
    std::for_each(m_goalitems.begin(), m_goalitems.end(),
        [](QGoalItem * p) { delete p; } );
    m_goalitems.clear();
}

void LFormGoalItem::NewGoalItem( HELEMENT heShowAt )
{
    if (NULL == m_pGoal)
        return ;

//     LGoalItemDlg GIDlg(m_pGoal);
//     if (GIDlg.DoModal() == IDOK)
//     {
//         QGoalItem *pNew = GIDlg.m_pGoalItemInout;
//         m_goalitems.push_back(pNew);
//         ETable tbl = AddGoalItem(pNew);
//         tbl.scroll_to_view(false,true);
//         return pNew;
//     }
//     return NULL;
    
    m_widgetGoalitemOP.SetAndShow(m_pGoal, heShowAt);
}

void LFormGoalItem::OnClkDeleteItem( HELEMENT he )
{
    ETable tbl = ECtrl(he).select_parent("table",4);
    QGoalItem *pGoalItem = _GoalItemOfCtl(tbl);
    ASSERT(NULL != pGoalItem);
    VecGoalItemItr iFind = std::find(
        m_goalitems.begin(), m_goalitems.end(), pGoalItem);
    ASSERT(iFind != m_goalitems.end());
    if (XMsgBox::YesNoMsgBox(L"确定删除吗？") == IDYES)
    {
        if (m_pGoal->DeleteItem(pGoalItem))
        {
            QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_GOALSUBITEMDELETED,
                (LPARAM) m_pGoal);

            tbl.destroy();
            delete pGoalItem;
            m_goalitems.erase(iFind);
        }
    }
}

void LFormGoalItem::OnClkChkCheckItem( HELEMENT he )
{
    ECheck chk(he);
    ETable tbl = chk.select_parent("table",4);
    QGoalItem *pGoalItem = _GoalItemOfCtl(tbl);
    ASSERT(NULL != pGoalItem);
    if (!pGoalItem->CheckIt(chk.IsChecked()))
    {
        chk.SetCheck( !chk.IsChecked() );
    }
    else
    {
        QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_GOALSUBITEMSTATUSCHANGED,
            (LPARAM) pGoalItem);
    }
}

void LFormGoalItem::OnClkNewOrEdit( HELEMENT )
{
    QGoalItem *pItem = m_widgetGoalitemOP.m_pGoalItemInout;
    if (m_widgetGoalitemOP.IsEditMode())
    {
        ETable tbl = _GoalItemBox().FindFirstWithData(pItem);
        RefreshGoalItem(tbl);
    }
    else
    {
        m_goalitems.push_back(pItem);
        AddGoalItem(pItem).scroll_to_view(false, true);
        QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_GOALSUBITEMADDED,
             (LPARAM) pItem);
    }
}

void LFormGoalItem::OnClkGoalitemTitle( HELEMENT he )
{
    ETable tbl = ECtrl(he).select_parent("table",4);
//     ECtrl pp = GetCtrl("#id_popup_goalitem");
//     ::HTMLayoutShowPopup(pp , tbl, 2);
//     return;
    QGoalItem *pGoalItem = _GoalItemOfCtl(tbl);
    ASSERT(NULL != pGoalItem);
    m_widgetGoalitemOP.SetAndShow(pGoalItem, tbl);
}

void LFormGoalItem::OnClkNewGoalitem( HELEMENT he )
{
    NewGoalItem(ECtrl(he).parent());
}

void LFormGoalItem::Destroy()
{
    ClearMem();

    RemoveFormEntry(&m_widgetGoalitemOP);
    m_widgetGoalitemOP.Detach();

    ECtrl cRoot = GetRoot();
    if (cRoot.is_valid())
    {
        cRoot.destroy();
    }

    Detach();
}

void LFormGoalItem::CreateForm( ECtrl& cParent, int iPostion ,QGoal *pGoal)
{
    Destroy();

    QString sHtml = 
        L"<div id=\"id-goalitem-box-wrapper\">"
        L"  <popup #id_popup_goalitem .xpopup>"
        L"      <input type=\"text\" id=\"id_bar_goalitem\" style=\"width:*;\" maxlength=\"50\" novalue=\"小目标：\" />"
        L"      <button id=\"id_bar_ok\" accesskey=\"!RETURN\" style=\"visibility:collapse;\" />"
        L"      <button id=\"id_bar_cancel\" accesskey=\"!ESCAPE\" style=\"visibility:collapse;\" />"
        L"  </popup>"
        L"  <div id=\"id-goalitem-box\" />"
        L"</div>";
    ECtrl cThis = ECtrl::create("div");
    cParent.insert(cThis, iPostion);
    cThis.SetHtml(sHtml);

    Attach(cThis);

    m_pGoal = pGoal;
    _GoalItemBox().DeleteAllChild();
    if (NULL != m_pGoal)
    {
        int nGoalItem = m_pGoal->GetAllItems(m_goalitems);
    }
    RefreshGoalBox();
}

void LFormGoalItem::SetShowFlag(BOOL bShowFinished, BOOL bShowUnFinished)
{
    m_bShowFinished = bShowFinished;
    m_bShowUnfinished = bShowUnFinished;

    RefreshGoalBox();
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LGoalitemWidget,QForm)
    BN_CLICKED_ID(L"id_bar_ok", &LGoalitemWidget::OnClkIdbarOK)
    BN_CLICKED_ID(L"id_bar_cancel", &LGoalitemWidget::OnClkIdbarCancel)
QUI_END_EVENT_MAP()

BOOL LGoalitemWidget::CheckDate(__out QTime &tmBegin,__out QTime &tmEnd)
{
    tmBegin = tmEnd = QTime::GetCurrentTime();
    return TRUE;
//     tmBegin = EDate(GetCtrl("#date_begin")).GetDate();
//     tmEnd = EDate(GetCtrl("#date_end")).GetDate();
// 
//     QTime tmNow = QTime::GetCurrentTime();
//     if ((tmBegin.CompareDate(tmNow) < 0) || (tmBegin >= tmEnd))
//     {
//         XMsgBox::OkMsgBox(L"请设定一个正确的任务周期");
//         return FALSE;
//     }
//     return TRUE;
}

BOOL LGoalitemWidget::CheckMemberParameters()
{
    BOOL bOK = FALSE;
    if (m_bEditMode)
    {
        bOK = (m_pGoalItemInout != NULL);
    }
    else
    {
        bOK = (m_pGoal != NULL);
    }
    if ( !bOK )
    {
        GetRoot().ShowTooltip(L"数据错误，操作失败!");
    }
    return bOK;
}

void LGoalitemWidget::OnClkIdbarOK( HELEMENT )
{
    if (!CheckMemberParameters())
        return;

    QTime tmBegin, tmEnd;
    if ( !CheckDate(tmBegin, tmEnd) )
        return ;

    EEdit ctlInput = GetCtrl("#id_bar_goalitem");
    QString sGoalItem = ctlInput.GetText().Trim();
    if (sGoalItem.IsEmpty())
    {
        ctlInput.ShowTooltip(L"不能为空");
        return;
    }

    // 需要测试
    BOOL bOK = FALSE;
    if (m_bEditMode)
    {
        bOK = m_pGoalItemInout->Edit(sGoalItem, tmBegin, tmEnd);
    }
    else
    {
        QGoalItem      newGoalItem;
        newGoalItem.m_nGoalID = m_pGoal->ID();
        newGoalItem.m_sItem = sGoalItem;
        newGoalItem.m_tmBegin = tmBegin;
        newGoalItem.m_tmEnd = tmEnd;
        m_pGoalItemInout = m_pGoal->AddGoalItem(newGoalItem);
        bOK = (NULL != m_pGoalItemInout);
    }
    if (!bOK)
    {
        GetRoot().ShowTooltip(L"操作失败！请重试.");
        return ;
    }
    HTMLayoutHidePopup(GetRoot());

    ReflectThisEvent();
}

void LGoalitemWidget::OnClkIdbarCancel( HELEMENT )
{
    HTMLayoutHidePopup(GetRoot());
}

void LGoalitemWidget::SetAndShow( QGoal *pGoal, HELEMENT heShowAt )
{
    ASSERT(NULL != pGoal);
    m_pGoal = pGoal;
    m_pGoalItemInout = NULL;
    m_bEditMode = FALSE;

    Show(heShowAt);
}

void LGoalitemWidget::SetAndShow( QGoalItem* pGoalItem , HELEMENT heShowAt)
{
    ASSERT(NULL != pGoalItem);
    m_pGoal = NULL;
    m_pGoalItemInout = pGoalItem;
    m_bEditMode = TRUE;

    Show(heShowAt);
}

void LGoalitemWidget::Show(HELEMENT he)
{
    ASSERT(NULL != he);
    ECtrl r = GetRoot();
    ECtrl ctl = r.find_first("#id_bar_goalitem");
    EEdit ctlInput = GetCtrl("#id_bar_goalitem");
    // 模式设置
    if (m_bEditMode)
    {
        ctlInput.SetText(m_pGoalItemInout->Item());
        ctlInput.SelectText();
    }
    else
    {
        ctlInput.SetText(L"");
    }
    r.SetFocus();
    ctlInput.SetFocus();

    HTMLayoutShowPopup(GetRoot(), he, 2);
}

//创建普通快捷方式
// BOOL CModieDlg::fnCreateLink ( )
// {
//     LPCSTR pszPath,pszLink;
//     pszPath="C:\123.exe";//目标程序名
//     pszLink="c:\\123.url";//快捷方式名
//     HRESULT hres=NULL;
//     IShellLink * psl ;
//     IPersistFile* ppf ;
//     WCHAR   wsz[MAX_PATH]; //   buffer   for   Unicode   string 
//     hres = CoCreateInstance( CLSID_ShellLink, NULL,CLSCTX_INPROC_SERVER, IID_IShellLink,(void **)&psl) ;
// 
//     if(NULL ==hres)        return FALSE ;
// 
//     psl->SetPath(pszPath) ;//设置路径
//     psl->SetHotkey(MAKEWORD('A',HOTKEYF_CONTROL)) ;  //设置快捷键、热键
//     hres = psl -> QueryInterface( IID_IPersistFile,    (void**)&ppf);//获取IPersistFile接口
//     if(NULL ==hres)        return FALSE ;
//     MultiByteToWideChar(CP_ACP,0,pszLink,-1,wsz,MAX_PATH);
//     hres = ppf -> Save( pszLink, STGM_READWRITE) ;//保存快捷方式
//     ppf->Release();//释放IPersistFile和IShellLink接口
//     psl->Release();
//     // 通知SHELL变化
//     SHChangeNotify( ....) ;//从略，具体可以参考该函数定义，一般创建后通知本目录和父目录即可。
//     return TRUE;
// }