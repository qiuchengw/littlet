#include "stdafx.h"

#include "LStickyNote.h"
#include "common/QDBHelper.h"
#include "ui/QConfig.h"
#include "ui/WndHelper.h"


namespace littlet
{
    LStickyNoteWnd* NewStickyNote(__out TTodoTask& t)
    {
        t.sTask = L"新便签";
        t.nFlag = TODO_FLAG_STICKYNOTE;
        t.nID = QDBEvents::GetInstance()->TodoTask_Add(&t);
        t.nCateID = INVALID_ID;
        t.nPriority = 1;
        t.eStatus = TODO_STATUS_PROCESSING;

        // 创建便签
        return StickyNoteMan::GetInstance()->Create(t);
    }
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LStickyNoteWnd, _Base)
    BN_CLICKED_NAME(L"item_note", &LStickyNoteWnd::OnClkNoteItem)
    BN_CLICKED_NAME(L"del_item", &LStickyNoteWnd::OnClkDelItem)
    BN_CLICKED_ID(L"btn_find", &LStickyNoteWnd::OnClkFind)
    BN_CLICKED_ID(L"btn_editor", &LStickyNoteWnd::OnClkFontEditor)
    BN_CLICKED_ID(L"btn_new", &LStickyNoteWnd::OnclkNewNote)
    BN_STATECHANGED_ID(L"chk_topmost", &LStickyNoteWnd::OnClkPinTop);
    BN_CLICKED_ID(L"btn_close", &LStickyNoteWnd::OnClkClose)
    UNKNOWN_SELECTION_CHANGED_ID(L"color_scheme", &LStickyNoteWnd::OnSelColorSchemeChanged)
QUI_END_EVENT_MAP()

LStickyNoteWnd::LStickyNoteWnd(const TTodoTask& task)
    :taskid_(task.nID)
{
    m_sHtmlPath = L"qabs:main/todo/note.htm";
}

void LStickyNoteWnd::OnclkNewNote(HELEMENT he) 
{
    TTodoTask t;
    if (LStickyNoteWnd* w = littlet::NewStickyNote(t))
    {
        // 显示在自己旁边
        CRect rc;
        GetWindowRect(&rc);

        CRect rc_dest;
        w->GetWindowRect(&rc_dest);
        CPoint pt_dest = rc.TopLeft();
        pt_dest.Offset(40, 40);
        rc_dest.MoveToXY(pt_dest);

        w->MoveWindow(&rc_dest, FALSE);
    }
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
    return ETable();
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
    SaveWindowPos();

    // 如果不是退出就不关闭
    SetMsgHandled(FALSE);
}

void LStickyNoteWnd::OnClkFontEditor(HELEMENT he) 
{
    ECtrl ctl = GetCtrl("#editor_msg");
    if (ctl.IsHasAttribute("focus"))
        ctl.remove_attribute("focus");
    else
        ctl.set_attribute("focus", L"1");
}

void LStickyNoteWnd::OnClkFind(HELEMENT he) 
{
    
}

LRESULT LStickyNoteWnd::OnDocumentComplete() 
{
    if (taskid_ == INVALID_ID)
    {
        ASSERT(FALSE);
        return 0;
    }

    RestoreSetting();

    TTodoTask task;
    if (QDBEvents::GetInstance()->TodoTask_Get(taskid_, task))
    {
        _Text().SetValue(task.sTask);
    }

    return 0;
}

void LStickyNoteWnd::NewIt() 
{
    OnclkNewNote(NULL);
}

void LStickyNoteWnd::EditIt(__in TTodoTask* p) 
{

}

void LStickyNoteWnd::OnClkClose(HELEMENT he) 
{
    TTodoTask task = Task();
    // 设置为已完成
    task.eStatus = TODO_STATUS_FINISH;
    // 关闭窗口认为去掉stick标志
    _RemoveFlag(task.nFlag, TODO_FLAG_STICKYNOTE);

    // 保存
    QDBEvents::GetInstance()->TodoTask_Edit(&task);

    // 关闭窗口
    SendMessage(WM_CLOSE);
}

void LStickyNoteWnd::SetTopMost(bool top)
{
    ModifyStyleEx(0, WS_EX_TOPMOST);

    GetCtrl("#chk_topmost").SetCheck(top);
    WTL::CRect rc(0, 0, 0, 0);
    SetWindowPos(top ? HWND_TOPMOST : HWND_NOTOPMOST, &rc, SWP_SHOWWINDOW | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOMOVE);
    QUIGetConfig()->SetValue(L"StickyNoteTop", CStdString::number(taskid_), top ? L"1" : L"0");
}

void LStickyNoteWnd::OnClkPinTop(HELEMENT he) 
{
    SetTopMost(ECheck(he).IsChecked());
}

void LStickyNoteWnd::Show(BOOL bNew, BOOL bUseDate, QTime tmDate /*= QTime::GetCurrentTime() */) 
{
}

void LStickyNoteWnd::ShowAndEdit(TTodoTask* p) 
{
}

void LStickyNoteWnd::OnKillFocus(HWND)
{
    TTodoTask task = Task();
    task.sTask = _Text().get_value().to_string();
    QDBEvents::GetInstance()->TodoTask_Edit(&task);
}

void LStickyNoteWnd::OnSetFocus(HWND)
{
}

void LStickyNoteWnd::OnSelColorSchemeChanged(HELEMENT he, HELEMENT)
{
    CStdString cr = EColorPicker(he).GetColor();
    GetBody().SetBkgndColor(cr);
    // 保存背景色
    QUIGetConfig()->SetValue(L"StickyNoteColor", CStdString::number(taskid_), cr);
}

TTodoTask LStickyNoteWnd::Task() const
{
    TTodoTask task;
    QDBEvents::GetInstance()->TodoTask_Get(TaskID(), task);
    return task;
}

void LStickyNoteWnd::SaveWindowPos()
{
    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);

    // get window position and iconized/maximized status
    GetWindowPlacement(&wp);

    wp.flags = 0;
    wp.showCmd = SW_SHOWNORMAL;

    TCHAR tmp[200];
    WTL::SecureHelper::sprintf_x(tmp, 200, _T("%u,%u,%d,%d,%d,%d"),
        wp.flags,
        wp.showCmd,
        wp.rcNormalPosition.left,
        wp.rcNormalPosition.top,
        wp.rcNormalPosition.right,
        wp.rcNormalPosition.bottom);

    // write position to INI file
    QUIGetConfig()->SetValue(L"StickyNote", CStdString::number(taskid_), tmp);
}

BOOL LStickyNoteWnd::RestoreSetting()
{
    // read window position from INI file
    // MainWindow format =0,1,395,198,1012,642
    auto* cfg = QUIGetConfig();

    // 背景色
    CStdString scolor = cfg->GetValue(L"StickyNoteColor", CStdString::number(taskid_));
    if (!scolor.IsEmpty())
    {
        GetBody().SetBkgndColor(scolor);
    }

    // topmost
    if (cfg->GetValue(L"StickyNoteTop", CStdString::number(taskid_)) == L"1")
    {
        SetTopMost(true);
    }

    // 位置
    CStdString sPos = cfg->GetValue(L"StickyNote", CStdString::number(taskid_));
    if (sPos.IsEmpty())
    {
        CenterWindow(::GetDesktopWindow());
        return FALSE;
    }

    TCHAR tmp[256];
    _tcsncpy_s(tmp, sPos, _countof(tmp) - 2);

    // get WINDOWPLACEMENT info
    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);
    wp.ptMaxPosition = CPoint(-::GetSystemMetrics(SM_CXBORDER), -::GetSystemMetrics(SM_CYBORDER));
    TCHAR *cp = 0;
    wp.rcNormalPosition = CRect(200, 100, 850, 550);
    if ((cp = _tcstok(tmp, _T(",\r\n"))) != NULL)
        wp.flags = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.showCmd = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.left = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.top = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.right = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.bottom = _ttoi(cp);

    // sets window's position and iconized/maximized status
    SetWindowPlacement(&wp);

    // 最后给它一个wm_move 消息
    SendMessage(WM_EXITSIZEMOVE, 0, 0);

    return TRUE;
}

void LStickyNoteWnd::OnKeyDown(TCHAR ch, UINT n, UINT r)
{
    if (GetKeyState(VK_TAB) & 0x8000)
    {
        if (GetKeyState(VK_CONTROL) & 0x8000)
        {
            LStickyNoteWnd* p = nullptr;
            if (GetKeyState(VK_SHIFT) & 0x8000)
            {
                // 向前
                p = StickyNoteMan::GetInstance()->PrevSibling(this);
            }
            else
            {
                // 向后
                p = StickyNoteMan::GetInstance()->NextSibling(this);
            }
            if ((nullptr != p) && (p != this))
            {
                quibase::SetForegroundWindowInternal(p->GetSafeHwnd());
                p->SetFocus();
            }
            return;
        }
    }

    SetMsgHandled(FALSE);
}

//////////////////////////////////////////////////////////////////////////
LStickyNoteWnd* StickyNoteMan::Create(const TTodoTask& t)
{
    auto*db = QDBEvents::GetInstance();
    db->TodoTask_SetFlag(t.nID, t.nFlag | TODO_FLAG_STICKYNOTE);
    db->TodoTask_SetStatus(t.nID, TODO_STATUS_PROCESSING);
    
    LStickyNoteWnd* wnd = Find(t.nID);
    if (nullptr == wnd)
    {
        // 显示桌面便签
        wnd = new LStickyNoteWnd(t);
        wnd->Create(NULL, WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW,
            WS_QEX_WNDSHADOW | WS_QEX_THICKFRAME);
        
        lst_.push_back(wnd);
    }
    return wnd;
}

void StickyNoteMan::Shutdown()
{
    for (LStickyNoteWnd* p : lst_)
    {
        if (p->IsWindow())
        {
            p->SendMessage(WM_CLOSE);
        }
        delete p;
    }
    lst_.clear();
}

void StickyNoteMan::Startup()
{
    if (!lst_.empty())
    {
        ASSERT(FALSE);
        return;
    }

    TodoTaskList lst;
    if (QDBEvents::GetInstance()->TodoTask_GetAllStickyNote(lst))
    {
        for (auto itm : lst )
        {
            Create(itm);
        }
    }
}

LStickyNoteWnd* StickyNoteMan::Find(int taskid)
{
    for (auto i = lst_.begin(); i != lst_.end();)
    {
        if (!(*i)->IsWindow())
        {
            delete *i;
            i = lst_.erase(i);
        }
        else
        {
            if ((*i)->TaskID() == taskid)
            {
                return *i;
            }
            ++i;
        }
    }
    return nullptr;
}

void StickyNoteMan::Remove(int taskid)
{
    if (auto* p = Find(taskid))
    {
        p->SendMessage(WM_CLOSE);
    }
}

LStickyNoteWnd* StickyNoteMan::PrevSibling(LStickyNoteWnd* p)
{
    auto i = std::find(lst_.begin(), lst_.end(), p);
    if (i == lst_.end())
    {
        return nullptr;
    }

    if (i == lst_.begin())
    {
        return lst_.back();
    }
    return *--i;
}

LStickyNoteWnd* StickyNoteMan::NextSibling(LStickyNoteWnd* p)
{
    auto i = std::find(lst_.begin(), lst_.end(), p);
    if (i == lst_.end())
    {
        return nullptr;
    }

    if (p == lst_.back())
    {
        return lst_.front();
    }
    return *++i;
}
