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

    void EditIt(__in TTodoTask* p);
    void Show(BOOL bNew, BOOL bUseDate, QTime tmDate = QTime::GetCurrentTime() );
    void ShowAndEdit( TTodoTask* p );

protected:
    void NewIt();
    void OnclkNewNote(HELEMENT he);
    void OnClkNoteItem(HELEMENT he);
    void OnClkFind(HELEMENT he);
    void OnClkShowAll(HELEMENT he);
    void OnClkDelItem(HELEMENT he);

    void OnSelColorSchemeChanged(HELEMENT he, HELEMENT );

    void OnClkClose(HELEMENT he);
    void OnClkPinTop(HELEMENT he);

    void ShowEditPane(BOOL bShow , TTodoTask* p = nullptr);

    void OnClose();
    void OnKillFocus(HWND);
    void OnSetFocus(HWND);

    // 位置
    void SaveWindowPos();
    BOOL RestoreSetting();

    void SetTopMost(bool top);

protected:
    TTodoTask* _ItemData(__in ETable& t)
    {
        return reinterpret_cast<TTodoTask*>(t.GetData());
    }

    virtual LRESULT OnDocumentComplete();

    inline ECtrl _Text()
    {
        return GetCtrl("#editor_msg>richtext");
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

    LStickyNoteWnd* Find(int taskid);

private:
    LstStickyWnd        lst_;
};

