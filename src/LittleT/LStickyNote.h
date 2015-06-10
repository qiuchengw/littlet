#pragma once

#include <list>
#include "ui/QFrame.h"

struct TTodoTask;
class LStickyNoteWnd : public QFrame
{
    typedef QFrame _Base;

    friend class StickyNoteMan;

    QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(LStickyNoteWnd)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MSG_WM_CLOSE(OnClose)
        CHAIN_MSG_MAP(_Base)
    END_MSG_MAP()

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

    void OnClkPrev(HELEMENT he);
    void OnClkNext(HELEMENT he);

    void ShowEditPane(BOOL bShow , TTodoTask* p = nullptr);

    void OnClose();
    void OnKillFocus(HWND);

protected:
    TTodoTask* _ItemData(__in ETable& t)
    {
        return reinterpret_cast<TTodoTask*>(t.GetData());
    }

    virtual LRESULT OnDocumentComplete();

    ECtrl _Box()
    {
        return GetCtrl("#box_item");
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

    void Startup();
    void Shutdown();

private:
    LstStickyWnd        lst_;
};

