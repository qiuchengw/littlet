#pragma once

#include "ui/QWindow.h"
#include "QWndGroup.h"
#include "JdrlCmn.h"
#include "clock/LAddTask.h"
#include "clock/DBSmileT.h"
#include "clock/DATaskMan.h"
#include "IPageDataMan.h"


struct TShortnote : public IDataItem
{
    TShortnote(const CStdString& cont, DWORD f = 0, int idClock = INVALID_ID)
        :cont_(cont), clock_id_(idClock), flag_(f)
    {
    }

    CStdString         cont_;
    int          clock_id_;// 闹钟的id
    DWORD        flag_;
// 
//     // 需要手动更新到数据库
//     void SetClock(QClock* p)
//     {
//         clock_id_ = (nullptr != p) ? p->ID(): INVALID_ID;
//     }
// 
//     QClock* GetClock()
//     {
//         if (IsHasClock())
//         {
//             return QClockMan::GetInstance()->GetItem(clock_id_);
//         }
//         return nullptr;
//     }

    inline BOOL IsHasClock() const
    {
        return INVALID_ID != clock_id_;
    }

    virtual void OnDelete();

};

class TShortNoteMan : public IDataMan<TShortnote>
{
    friend class TShortNoteManWrapper;

public:
    typedef IDataMan<TShortnote> _Base;

    //     SINGLETON_ON_DESTRUCTOR(TShortNoteMan)
    //     {
    //     }
    TShortNoteMan() : _Base(nullptr)
    {
        tbl_ = new _Table();
        _Base::_SetDBTable(tbl_);

        Startup();

        date_create_ = QTime::GetCurrentTime();
    }

private:
    class _Table : public IDBTable
    {
    public:
        QTime tm_create_;

    public:
        _Table(__in LPCWSTR szTbl = L"tbl_shortnote") 
            : IDBTable(jdrl::JdrlGetMainDBMan()->GetAdapter(), szTbl)
        {
        }

        // 增
        virtual BOOL InsertRow(TShortnote* t)
        {
            CStdString sQ;
            sQ.Format(L"INSERT INTO %s%s",m_sTableName, InsertionSql(t));
            t->ID() = m_pDB->_ExecSQL_RetLastID(sQ);

            t->CreationTime() = tm_create_;
            t->UpdationTime() = QTime::GetCurrentTime();

            return INVALID_ID != t->ID();
        }

        virtual CStdString InsertionSql(TShortnote* t)
        {
            CStdString sQ;
            sQ.Format(
                L" (clock_id,content,flag,crtime,udtime) "
                L" VALUES(%d,'%s',%d, %lf,%lf) ",
                t->clock_id_,t->cont_, t->flag_,
                tm_create_, 
                QTime::SQLDateTimeCurrent());
            return sQ;
        }

        virtual CStdString ModifySql(TShortnote* t)
        {
            CStdString sQ;
            sQ.Format(L"clock_id=%d,content='%s',flag=%d ",
                t->clock_id_,SFHSQ( t->cont_), t->flag_); 
            return sQ;
        }

        virtual BOOL OnSelect(__in SqlQuery& q, __out _Cont& cont)
        {
            QDB_BEGIN_TRY
                CStdString sError;
            for ( ; !q.eof() ; q.nextRow() )
            {
                TShortnote *p = new TShortnote(
                    q.StrValue(L"content"),
                    q.IntValue(L"flag"), 
                    q.IntValue(L"clock_id"));
                p->ID() = q.IntValue(L"id");
                p->UpdationTime() = q.DateTimeValue(L"udtime");
                p->CreationTime() = q.DateTimeValue(L"crtime");
                cont.push_back(p);
            }
            QDB_END_TRY
                return TRUE;
        }
    };

public:
    void SearchFor(__in const CStdString& s, __out _Cont& lst)
    {
        lst.clear();
        for (auto i = _Begin(); i != _End(); ++i)
        {
            if ((*i)->cont_.Find(s) != -1)
            {
                lst.push_back(*i);
            }
        }
    }

    BOOL Startup();

    void Sort();

    BOOL NewItem(TShortnote* p, QTime tmCreation = QTime::GetCurrentTime());

private:
    _Table*     tbl_;
    QTime       date_create_;   // 创建时间
};

typedef TShortNoteMan::_Cont LstShortNote;

class TShortNoteManWrapper : public IPagedData<LstShortNote>
{
    typedef IPagedData<LstShortNote> _Base;

    SINGLETON_ON_DESTRUCTOR(TShortNoteManWrapper)
    {

    }

public:
    TShortNoteManWrapper() : _Base()
    {
        UseAll();
    }

    inline TShortNoteMan* Man()
    {
        return &man_;
    }

    void UseAll()
    {
        lst_ = man_.GetAll();
    }

    BOOL GetPageOf(__in int iPage, __out LstShortNote& lst)
    {
        return _Base::GetPageOf(lst_, iPage, lst);
    }

    // 从后面数的第x页，数据是后面的插入到最前
    BOOL GetReversePageOf(__in int iPage, __out LstShortNote& lst)
    {
        return _Base::GetReversePageOf(lst_, iPage, lst);
    }

    inline int GetPageCount()
    {
        return _Base::GetPageCount(lst_);
    }

    void GetItemsBetween(__in QTime tb, __in QTime te, __out LstShortNote& lst)
    {
        _Base::GetItemsBetween(man_.GetAll(), tb, te, lst);
    }

    void GetItemOfCreation(__in QTime t, __out LstShortNote& lst)
    {
        _Base::GetItemOfCreation(man_.GetAll(), t, lst);
    }

private:
    LstShortNote     lstSel_;   // 当前保有的数据
    LstShortNote*    lst_;
    TShortNoteMan    man_;
};

class CJdrlShortNoteWnd : public QSingletonStickFrame<CJdrlShortNoteWnd>
{
    friend struct helpPopulateLst;
    typedef QSingletonStickFrame<CJdrlShortNoteWnd> _Base;

    SINGLETON_ON_DESTRUCTOR(CJdrlShortNoteWnd)
    {

    }

    QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(CJdrlShortNoteWnd)
        MSG_WM_CLOSE(OnClose)
        CHAIN_MSG_MAP(_Base)
        END_MSG_MAP()

public:
    CJdrlShortNoteWnd(void);

    void EditIt(__in TShortnote* p);
    void Show(BOOL bNew, BOOL bUseDate, QTime tmDate = QTime::GetCurrentTime() );
    void ShowAndEdit( TShortnote* p );

protected:
    void NewIt();
    void OnclkNewNote(HELEMENT he);
    void OnClkNoteItem(HELEMENT he);
    void OnClkSave(HELEMENT he);
    void OnClkCancel(HELEMENT he);
    void OnClkFind(HELEMENT he);
    void OnClkFind2(HELEMENT he);
    void OnClkShowAll(HELEMENT he);
    void OnClkDelItem(HELEMENT he);

    void OnClkPrev(HELEMENT he);
    void OnClkNext(HELEMENT he);

    void ShowEditPane(BOOL bShow , TShortnote* p = nullptr);

    void OnClose();

    void OnClkPageItem(HELEMENT he);

protected:
    TShortnote* _ItemData(__in ETable& t)
    {
        return reinterpret_cast<TShortnote*>(t.GetData());
    }

    virtual LRESULT OnDocumentComplete();

    ECtrl _Box()
    {
        return GetCtrl("#box_item");
    }

    // bAtFirst 在最前插入
    ETable AddItem(__in TShortnote* p, __in BOOL bAtFirst = TRUE);
    ETable NewItem(__in TShortnote* p);
    BOOL UpdateItem(__in ETable& tbl);

    EPageCtrl _PageCtl()
    {
        return GetCtrl("#page_ctl");
    }

    void RefreshPage();

private:
    int                 m_curPage;  // 当前页
    TShortnote*      m_pCur;
    ETable              m_tblCur;
    TShortNoteManWrapper*    m_pMan;
    BOOL                m_bUseDate;
    QTime               m_tmCreation;   // 创建事件
    LstShortNote             m_lstCur;   // 当前显示的那一页数据
};
