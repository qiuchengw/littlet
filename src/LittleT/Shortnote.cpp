#include "stdafx.h"

#include "JdrlShortnote.h"
#include "JdrlMsgBox.h"

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(CJdrlShortNoteWnd, _Base)
    BN_CLICKED_NAME(L"item_note", &CJdrlShortNoteWnd::OnClkNoteItem)
    BN_CLICKED_NAME(L"del_item", &CJdrlShortNoteWnd::OnClkDelItem)
    BN_CLICKED_ID(L"btn_find", &CJdrlShortNoteWnd::OnClkFind)
    BN_CLICKED_ID(L"btn_find2", &CJdrlShortNoteWnd::OnClkFind2)
    BN_CLICKED_ID(L"btn_showall", &CJdrlShortNoteWnd::OnClkShowAll)
    BN_CLICKED_ID(L"btn_new", &CJdrlShortNoteWnd::OnclkNewNote)
    BN_CLICKED_ID(L"btn_save", &CJdrlShortNoteWnd::OnClkSave)
    BN_CLICKED_ID(L"btn_cancel", &CJdrlShortNoteWnd::OnClkCancel)
    BN_STATECHANGED_ID(L"page_ctl", &CJdrlShortNoteWnd::OnClkPageItem)

    BN_CLICKED_ID(L"btn_next", &CJdrlShortNoteWnd::OnClkNext)
    BN_CLICKED_ID(L"btn_prev", &CJdrlShortNoteWnd::OnClkPrev)

    CHAIN_HLEVENT_TO_FORM(&m_formClock, "#div_timer_set")
QUI_END_EVENT_MAP()

CJdrlShortNoteWnd::CJdrlShortNoteWnd(void)
{
    m_sHtmlPath = L"qabs:dlgs/note.htm";
    m_pMan = TShortNoteManWrapper::GetInstance();
}

void CJdrlShortNoteWnd::OnclkNewNote(HELEMENT he) 
{
    ShowEditPane(TRUE);
}

void CJdrlShortNoteWnd::OnClkDelItem(HELEMENT he) 
{
    ETable tbl = ECtrl(he).select_parent(L"table", 3);
    TShortnote *p = _ItemData(tbl);
    if (nullptr != p)
    {
        m_pMan->Man()->DeleteItem(p);
        tbl.destroy();

        int n = m_pMan->GetPageCount();
        if (m_curPage >= n)
        {
            m_curPage = n - 1;
        }

        _PageCtl().SetPageNum(n);
        _PageCtl().SetCurPage(m_curPage + 1);

        RefreshPage();

        return;
    }

    tbl.ShowTooltip(L"删除失败");
}

void CJdrlShortNoteWnd::OnClkNoteItem(HELEMENT he) 
{
    // 编辑
    ETable t(he);
    TShortnote *p = _ItemData(t);
    ASSERT(nullptr != p);

    m_tblCur = t;
    ShowEditPane(TRUE, p);
}

void CJdrlShortNoteWnd::ShowEditPane(BOOL bShow, TShortnote* p) 
{
    ECtrl cList = GetCtrl("#div_list");
    ECtrl cEdit = GetCtrl("#div_edit");

    if (bShow )
    {
        cList.ShowCtrl(SHOW_MODE_COLLAPSE);
        cEdit.ShowCtrl(SHOW_MODE_SHOW);

        EEdit txt = GetCtrl("#txt_event");
        txt.SetText((nullptr != p) ? p->cont_ : L"");

        m_pCur = p;

        QClock* pClock = (nullptr == p) ? nullptr : p->GetClock();
        VERIFY(m_formClock.SetClock(pClock));
    }
    else
    {
        cList.ShowCtrl(SHOW_MODE_SHOW);
        cEdit.ShowCtrl(SHOW_MODE_COLLAPSE);

        m_pCur = nullptr;
        m_tblCur = NULL;

        VERIFY(m_formClock.SetClock(NULL));
    }
}

void CJdrlShortNoteWnd::OnClkSave(HELEMENT he) 
{
    QString sCont = EEdit(GetCtrl("#txt_event")).GetText();
    if (sCont.Trim().IsEmpty())
    {
        CJdrlMsgBox::OkMsgBox(GetSafeHwnd(), L"请输入记事内容");

        return;
    }

    QString sError;
    QClock *pClock = nullptr;
    if (!m_formClock.MakeClock(sCont.Left(50), sError))
    {
        return;
    }

    pClock = m_formClock.m_pClock;

    if ((nullptr != m_pCur) && (_ItemData(m_tblCur) == m_pCur))
    {
        // 编辑模式
        m_pCur->SetClock(pClock);
        m_pCur->cont_ = sCont;

        // 更新
        UpdateItem(m_tblCur);
    }
    else
    {
        // 新加
        m_pCur = new TShortnote(sCont);
        m_pCur->SetClock(pClock);
        if (m_bUseDate)
            VERIFY(m_pMan->Man()->NewItem(m_pCur, m_tmCreation));
        else
            VERIFY(m_pMan->Man()->NewItem(m_pCur));

        // 界面
        NewItem(m_pCur);
    }
    m_pMan->Man()->EditItem(m_pCur);
    ShowEditPane(FALSE);
}

ETable CJdrlShortNoteWnd::AddItem(__in TShortnote* p, __in BOOL bAtFirst) 
{
    ETable tbl = ETable::create("table");
    if (bAtFirst)
        _Box().insert(tbl, 0);
    else
        _Box().append(tbl);

    tbl.SetData(p);
    tbl.SetName(L"item_note");

    UpdateItem(tbl);

    return tbl;
}

ETable CJdrlShortNoteWnd::NewItem(__in TShortnote* p) 
{
    ETable tbl = AddItem(p);

    _PageCtl().SetPageNum(m_pMan->GetPageCount());
    _PageCtl().SetCurPage(m_curPage + 1);

    RefreshPage();

    return tbl;
}

BOOL CJdrlShortNoteWnd::UpdateItem(__in ETable& tbl) 
{
    TShortnote *p = _ItemData(tbl);
    if (nullptr != p)
    {
        QString s, sTime;
        if (p->CreationTime() > QTime::GetCurrentTime())
        {
            sTime = p->CreationTime().Format(L"%Y-%m-%d");
        }
        else
        {
            sTime = p->CreationTime().Format(L"%Y-%m-%d %H:%M");
        }
        s.Format(
            L"<tr><td .title>%s</td><td name=\"del_item\" .qbtn /></tr>"
            L"<tr><td .date>%s</td><td %s/></tr>",
            (LPCWSTR)p->cont_.Left(40), (LPCWSTR)sTime,
            p->IsHasClock() ? L".clock .qbtn" : L"");
        tbl.SetHtml(s);
        return TRUE;
    }
    return FALSE;
}

void CJdrlShortNoteWnd::OnClose() 
{
    // 如果不是退出就不关闭
    SetMsgHandled((CJdrlDatas::GetAppStatus() != APP_STATUS_QUITING));
    if (CJdrlDatas::GetAppStatus() != APP_STATUS_QUITING)
    {
        ShowWindow(SW_HIDE);
    }
    m_formClock.StopSound();
}

void CJdrlShortNoteWnd::OnClkCancel(HELEMENT he) 
{
    ShowEditPane(FALSE);
}

void CJdrlShortNoteWnd::OnClkShowAll(HELEMENT he) 
{
    m_curPage = 0;

    RefreshPage();
}

void CJdrlShortNoteWnd::OnClkFind(HELEMENT he) 
{
    QTime tb = EDate(GetCtrl("#date_begin")).GetDate();
    QTime te = EDate(GetCtrl("#date_end")).GetDate();

    _PageCtl().SetPageNum(0);
    m_curPage = 0;

    _Box().DeleteAllChild();
    m_pMan->GetItemsBetween(tb, te, m_lstCur);
    for (auto i = m_lstCur.rbegin(); i != m_lstCur.rend(); ++i) 
    {
        AddItem(*i);
    }
}

void CJdrlShortNoteWnd::OnClkFind2(HELEMENT he) 
{
    _PageCtl().SetPageNum(0);
    m_curPage = 0;

    QString sFind = EEdit(GetCtrl("#txt_cont")).GetText();
    if (!sFind.Trim().IsEmpty())
    {
        _Box().DeleteAllChild();

        m_pMan->Man()->SearchFor(sFind, m_lstCur);

        for (auto i = m_lstCur.rbegin(); i != m_lstCur.rend(); ++i) 
        {
            AddItem(*i);
        }
    }
}

LRESULT CJdrlShortNoteWnd::OnDocumentComplete() 
{
    m_pMan->Man()->Startup();
    m_pMan->SetItemCountPerPage(9);

    // 默认显示第一页，最新的在最后
    m_curPage = 0;

    _PageCtl().SetPageNum(m_pMan->GetPageCount());
    _PageCtl().SetCurPage(m_curPage + 1);

    RefreshPage();

    return 0;
}

void CJdrlShortNoteWnd::NewIt() 
{
    OnclkNewNote(NULL);
}

void CJdrlShortNoteWnd::EditIt(__in TShortnote* p) 
{

}

void CJdrlShortNoteWnd::OnClkPageItem(HELEMENT he) 
{
    m_curPage = _PageCtl().GetCurPage() - 1;
    if (m_curPage < 0)
        m_curPage = 0;

    RefreshPage();
}

void CJdrlShortNoteWnd::RefreshPage() 
{
    // 清空所有的项目
    _Box().DeleteAllChild();

    _PageCtl().SetPageNum(m_pMan->GetPageCount());
    _PageCtl().SetCurPage(m_curPage + 1);

    m_pMan->GetReversePageOf(m_curPage, m_lstCur);
    for (auto i = m_lstCur.rbegin(); i != m_lstCur.rend(); ++i) 
    {
        AddItem(*i);
    }
}

void CJdrlShortNoteWnd::OnClkPrev(HELEMENT he) 
{
    if (GetCtrl("#div_1").visible())
    {
        GetCtrl("#div_1").ShowCtrl(SHOW_MODE_COLLAPSE);
        GetCtrl("#div_2").ShowCtrl(SHOW_MODE_SHOW);
    }
    else
    {
        GetCtrl("#div_1").ShowCtrl(SHOW_MODE_SHOW);
        GetCtrl("#div_2").ShowCtrl(SHOW_MODE_COLLAPSE );
    }
}

void CJdrlShortNoteWnd::OnClkNext(HELEMENT he) 
{
    OnClkPrev(he);
}

void CJdrlShortNoteWnd::Show(BOOL bNew, BOOL bUseDate, QTime tmDate /*= QTime::GetCurrentTime() */) 
{
    _Base::Show();

    m_tmCreation = tmDate;
    m_bUseDate = bUseDate;

    if ( bNew )
    {
        NewIt();
    }

    if (m_bUseDate)
    {
        EDate d1 = GetCtrl("#date_begin");
        d1.SetDate(tmDate);
        EDate d2 = GetCtrl("#date_end");
        d2.SetDate(tmDate);

        // 清空所有的项目
        _Box().DeleteAllChild();

        _PageCtl().SetPageNum(0);
        m_pMan->GetItemOfCreation(tmDate, m_lstCur);
        for (auto i = m_lstCur.rbegin(); i != m_lstCur.rend(); ++i) 
        {
            AddItem(*i);
        }
    }
    else
    {
        EDate d1 = GetCtrl("#date_begin");
        d1.SetDate(QTime::GetCurrentTime());
        EDate d2 = GetCtrl("#date_end");
        d2.SetDate(QTime::GetCurrentTime());
    }
}

void CJdrlShortNoteWnd::ShowAndEdit(TShortnote* p) 
{
    _Base::Show();

    EditIt(p);
}

//////////////////////////////////////////////////////////////////////////

void TShortnote::OnDelete() 
{
    if (IsHasClock())
    {
        QClockMan::GetInstance()->DeleteItem(GetClock());
    }
}

BOOL TShortNoteMan::Startup() 
{
    if (_Base::Startup())
    {
        Sort();

        return TRUE;
    }
    return FALSE;
}

void TShortNoteMan::Sort() 
{
    struct _Comp
    {
        bool operator()(const TShortnote* p1, const TShortnote* p2)const
        {
            return p1->m_tmCreation < p2->m_tmCreation;
        }
    };
    std::stable_sort(_Begin(), _End(), _Comp());
}

BOOL TShortNoteMan::NewItem(TShortnote* p, QTime tmCreation) 
{
    tbl_->tm_create_ = tmCreation;

    if ( _Base::NewItem(p) )
    {
        if (tmCreation != QTime::GetCurrentTime())
        {
            Sort();
        }
        return TRUE;
    }
    return FALSE;
}
