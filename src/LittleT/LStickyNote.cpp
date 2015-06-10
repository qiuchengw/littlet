#include "stdafx.h"

#include "LStickyNote.h"
#include "common/QDBHelper.h"

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LStickyNoteWnd, _Base)
    BN_CLICKED_NAME(L"item_note", &LStickyNoteWnd::OnClkNoteItem)
    BN_CLICKED_NAME(L"del_item", &LStickyNoteWnd::OnClkDelItem)
    BN_CLICKED_ID(L"btn_find", &LStickyNoteWnd::OnClkFind)
    BN_CLICKED_ID(L"btn_showall", &LStickyNoteWnd::OnClkShowAll)
    BN_CLICKED_ID(L"btn_new", &LStickyNoteWnd::OnclkNewNote)
    BN_CLICKED_ID(L"btn_next", &LStickyNoteWnd::OnClkNext)
    BN_CLICKED_ID(L"btn_prev", &LStickyNoteWnd::OnClkPrev)
    UNKNOWN_SELECTION_CHANGED_ID(L"color_scheme", &LStickyNoteWnd::OnSelColorSchemeChanged)
QUI_END_EVENT_MAP()

LStickyNoteWnd::LStickyNoteWnd(const TTodoTask& task)
    :taskid_(task.nID)
{
    m_sHtmlPath = L"qabs:main/todo/note.htm";
}

void LStickyNoteWnd::OnclkNewNote(HELEMENT he) 
{
    ShowEditPane(TRUE);
}

void LStickyNoteWnd::OnClkDelItem(HELEMENT he) 
{
}

void LStickyNoteWnd::OnClkNoteItem(HELEMENT he) 
{
    // 编辑
}

void LStickyNoteWnd::ShowEditPane(BOOL bShow, TTodoTask* p) 
{
}

ETable LStickyNoteWnd::AddItem(__in TTodoTask* p, __in BOOL bAtFirst) 
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

ETable LStickyNoteWnd::NewItem(__in TTodoTask* p) 
{
    return ETable();
}

BOOL LStickyNoteWnd::UpdateItem(__in ETable& tbl) 
{
    return FALSE;
}

void LStickyNoteWnd::OnClose() 
{
    // 如果不是退出就不关闭
    SetMsgHandled(FALSE);
}

void LStickyNoteWnd::OnClkShowAll(HELEMENT he) 
{
}

void LStickyNoteWnd::OnClkFind(HELEMENT he) 
{
    
}

LRESULT LStickyNoteWnd::OnDocumentComplete() 
{

    return 0;
}

void LStickyNoteWnd::NewIt() 
{
    OnclkNewNote(NULL);
}

void LStickyNoteWnd::EditIt(__in TTodoTask* p) 
{

}

void LStickyNoteWnd::OnClkPrev(HELEMENT he) 
{
}

void LStickyNoteWnd::OnClkNext(HELEMENT he) 
{
    OnClkPrev(he);
}

void LStickyNoteWnd::Show(BOOL bNew, BOOL bUseDate, QTime tmDate /*= QTime::GetCurrentTime() */) 
{
}

void LStickyNoteWnd::ShowAndEdit(TTodoTask* p) 
{
}

void LStickyNoteWnd::OnKillFocus(HWND)
{
    
}

void LStickyNoteWnd::OnSelColorSchemeChanged(HELEMENT he, HELEMENT)
{
    GetRoot().SetBkgndColor(EColorPicker(he).GetColor());
}

//////////////////////////////////////////////////////////////////////////
LStickyNoteWnd* StickyNoteMan::Create(const TTodoTask& t)
{
    auto*db = QDBEvents::GetInstance();
    db->TodoTask_SetFlag(t.nID, t.nFlag | TODO_FLAG_STICKYNOTE);

    // 显示桌面便签
    LStickyNoteWnd* wnd = new LStickyNoteWnd(t);
    wnd->Create(NULL, WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW,
        WS_QEX_WNDSHADOW | WS_QEX_THICKFRAME);
    wnd->CenterWindow(::GetDesktopWindow());
    lst_.push_back(wnd);

    return wnd;
}

void StickyNoteMan::Shutdown()
{
    for (LStickyNoteWnd* p : lst_)
    {
        if (p->IsWindow())
        {
            p->SendMessage(WM_CLOSE);

            delete p;
        }
    }
    lst_.clear();
}

void StickyNoteMan::Startup()
{

}
