#pragma once

#include "ui/QWindow.h"
#include "../common/QLongplan.h"

/**
 *  添加、修改QGoalItem
**/
class LGoalitemWidget : public QForm
{
    QUI_DECLARE_EVENT_MAP;

public:
    void SetAndShow( QGoalItem* pGoalItem , HELEMENT heShowAt); // 添加模式
    void SetAndShow( QGoal *pGoal, HELEMENT heShowAt ); // 编辑模式

    inline BOOL IsEditMode()const
    {
        return m_bEditMode;
    }

protected:
    BOOL CheckDate(__out QTime &tmBegin,__out QTime &tmEnd);
    // 检查参数的合法性
    BOOL CheckMemberParameters();
    // heShowAt 现在何处
    void Show(HELEMENT heShowAt);
    void OnClkIdbarOK( HELEMENT );
    void OnClkIdbarCancel( HELEMENT );

private:
    QGoal*      m_pGoal;
    BOOL        m_bEditMode;

public:
    QGoalItem*     m_pGoalItemInout;
};

class LFormGoalItem : public QForm
{
    QUI_DECLARE_EVENT_MAP;
    QUI_DECLARE_REFLECT_EVENT_MAP;

    void ClearMem();

public:
    LFormGoalItem(void);
    ~LFormGoalItem(void);

    void SetShowFlag(BOOL bShowFinished, BOOL bShowUnFinished);

    void CreateForm( ECtrl& cParent, int iPostion ,QGoal *pGoal);
    void Destroy();

    int GetIndex()
    {
        if (!GetRoot().is_valid())
            return -1;
        return GetRoot().index();
    }
    // 新创建一个GoalItem
    // 显示的创建对话框显示在heShowAt下面
    void NewGoalItem( HELEMENT heShowAt );

protected:
    ETable AddGoalItem(QGoalItem* pGoalItem);
    
    void RefreshGoalItem(ETable& tblGoalItem);

    void OnClkDeleteItem(HELEMENT he);

    void OnClkChkCheckItem(HELEMENT he);

    void OnClkNewOrEdit(HELEMENT);

    void OnClkGoalitemTitle(HELEMENT he);
    // new
    void OnClkNewGoalitem(HELEMENT he);

    void RefreshGoalBox();

protected:
    ECtrl _GoalItemBox()
    {
        return GetCtrl("#id-goalitem-box");
        //return GetRoot();
    }

    QGoalItem* _GoalItemOfCtl(ETable tbl)
    {
        if (tbl.is_valid())
        {
            ASSERT(aux::wcseqi(L"goalitem-item", tbl.get_attribute("name")));
            return reinterpret_cast<QGoalItem*>(tbl.GetData());
        }
        return NULL;
    }

    ETable _CurrentGoalItemCtl()
    {
        return _GoalItemBox().find_first("table:checked");
    }

    QGoalItem* _CurrentGoalItem()
    {
        return _GoalItemOfCtl(_CurrentGoalItemCtl());
    }

private:
    QGoal*      m_pGoal;
    VecGoalItem m_goalitems;
    LGoalitemWidget m_widgetGoalitemOP; // 用于操作QGoalItem
    BOOL        m_bShowFinished;
    BOOL        m_bShowUnfinished;
};

// class LGoalItemDlg : public QDialog
// {
// public:
//     LGoalItemDlg(QGoal *pGoal);    // 添加模式
//     LGoalItemDlg(QGoalItem* pGoalItem);  // 编辑模式
// 
// protected:
//     virtual LRESULT OnDocumentComplete();
//     virtual BOOL OnDefaultButton(INT_PTR nBtn);
//     BOOL CheckDate(__out QTime &tmBegin,__out QTime &tmEnd);
//     // 检查参数的合法性
//     BOOL CheckMemberParameters();
// 
// private:
//     QGoal*      m_pGoal;
//     BOOL        m_bEditMode;
// public:
//     QGoalItem*     m_pGoalItemInout;
// };
