#pragma once

#include <list>
#include "ui/QFrame.h"

class LStickyNoteWnd;
struct TTodoTask;

namespace littlet
{
    LStickyNoteWnd* NewStickyNote(__out TTodoTask&);
}

class LStickyNoteWnd : public QFrame
{
    typedef QFrame _Base;

    friend class StickyNoteMan;

    QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(LStickyNoteWnd)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MSG_WM_SETFOCUS(OnSetFocus)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_KEYDOWN(OnKeyDown)  // 这个为啥会造成字体设置按钮不可用？
        CHAIN_MSG_MAP(_Base)
    END_MSG_MAP()
public:
    inline int TaskID()const
    {
        return taskid_;
    }

    TTodoTask Task()const;

private:
    LStickyNoteWnd(const TTodoTask& task);

    bool SearchText(const CStdString& txt)const;

protected:
    void OnclkNewNote(HELEMENT he);
    void OnClkNoteItem(HELEMENT he);
    void OnClkFind(HELEMENT he);
    void OnClkFontEditor(HELEMENT he);
    void OnStrikeText(HELEMENT he);

    void OnSelColorSchemeChanged(HELEMENT he, HELEMENT );

    void OnClkClose(HELEMENT he);
    void OnClkPinTop(HELEMENT he);

    void OnClkSearchCancel(HELEMENT he);
    void OnClkSearchOK(HELEMENT he);

    void OnClose();
    void OnKillFocus(HWND);
    void OnSetFocus(HWND);
    void OnKeyDown(TCHAR ch, UINT n, UINT r);

    // 位置
    void SaveWindowPos();
    BOOL RestoreSetting();

    void SetTopMost(bool top);
    bool IsTopMost()const;

    void ShowSearchBar();

protected:
    TTodoTask* _ItemData(__in ETable& t)
    {
        return reinterpret_cast<TTodoTask*>(t.GetData());
    }

    virtual LRESULT OnDocumentComplete();

    inline EEdit _Text()const
    {
        return GetCtrl("#editor_msg>richtext");
    }

    inline EPopup _SearchBar()const
    {
        return GetCtrl("#id_popup_todoitem");
    }

    inline ECombobox _Font()
    {
        return GetCtrl("#editor_msg #richtext-fontfamily");
    }

    // bAtFirst 在最前插入
    ETable AddItem(__in TTodoTask* p, __in BOOL bAtFirst = TRUE);
    ETable NewItem(__in TTodoTask* p);
    BOOL UpdateItem(__in ETable& tbl);

    EPageCtrl _PageCtl()
    {
        return GetCtrl("#page_ctl");
    }

private:
    int     taskid_ = -1;
};

/*
 *	便签管理器
 */
class StickyNoteMan
{
    SINGLETON_ON_DESTRUCTOR(StickyNoteMan)
    {
        Shutdown();
    }

public:
    typedef std::list<LStickyNoteWnd*> LstStickyWnd;

    LStickyNoteWnd* Create(const TTodoTask& t);
    void Remove(int taskid);

    void Startup();
    void Shutdown();
    void ShowAll();

    LStickyNoteWnd* Find(int taskid);

    LStickyNoteWnd* PrevSibling(LStickyNoteWnd* p);
    LStickyNoteWnd* NextSibling(LStickyNoteWnd* p);

    /*
     *	搜索
     */
    LStickyNoteWnd* SearchNext(LStickyNoteWnd* cur_win);
public:
    CStdString      s_search_;

private:
    LstStickyWnd        lst_;
};

