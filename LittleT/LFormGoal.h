#pragma once

#include "ui/QWindow.h"
#include "../common/QLongplan.h"
#include "LFormGoalItem.h"

class LFormGoal : public QForm
{
    QUI_DECLARE_EVENT_MAP;

public:
    LFormGoal(void);

    ~LFormGoal(void);

    void SetStage(QStage* pStage);

    QGoal* GetCurrentGoal()
    {
        return _CurrentGoal();
    }

    void NewGoalAdded(QGoal* pGoal);

    void NewGoalSubItemAdded(QGoalItem *pGoalItem);
    // goal sub item stagtus changed
    void GoalSubItemStatusChanged(QGoalItem *pGoalItem);
    
    // pParentGoal 下的一个subitem（qgoalitem）被删除
    void GoalSubItemDelete(QGoal *pParentGoal);

    void SetGoalitemShow(BOOL bShowFinished, BOOL bShowUnFinished);
protected:
    void ClearMem();

    ETable AddGoalCtl(QGoal* pGoal);
    
    void RefreshGoal(ETable &tblGoal);

    /** 选中一个计划项目
     *	return:
     *      TRUE    成功
     *	params:
     *		-[in]
     *          tblGoal     需要被选中的项目
     *                      无效时选中列表中的第一个项目
    **/
    BOOL SelectGoal(__in ETable tblGoal = NULL);
protected:
//    void OnGoalSelChanged(HELEMENT he);
    void OnMenuItemClkNewGoal(HELEMENT,EMenuItem);
    void OnMenuItemClkNewGoalSubItem(HELEMENT,EMenuItem);
    void OnMenuItemClkEdit(HELEMENT,EMenuItem);
    void OnMenuItemClkDelete(HELEMENT,EMenuItem);
    void OnClkDeleteGoal(HELEMENT);
    void OnGoalSelChanged(HELEMENT);

protected:
    ECtrl _GoalBox()
    {
        return GetRoot();
        //return GetCtrl("#id-goal-box");
    }
    
    QGoal* _GoalOfCtl(ETable tblGoal)
    {
        if (tblGoal.is_valid())
        {
#ifdef _DEBUG
            ASSERT(aux::wcseqi(L"goal-item", tblGoal.get_attribute("name")));
#endif
            return reinterpret_cast<QGoal*>(tblGoal.GetData());
        }
        return NULL;
    }

    ETable _CurrentGoalCtl()
    {
        return _GoalBox().find_first("table:checked");
    }
    
    QGoal* _CurrentGoal()
    {
        return _GoalOfCtl(_CurrentGoalCtl());
    }
    // 创建goal的子form
    void CreateGoalitemForm(ETable &tblGoal);
private:
    QStage*         m_pStage;
    VecGoal         m_goals;
    LFormGoalItem   m_formGoalItem;
};


class LGoalDlg : public QDialog
{
public:
    LGoalDlg(QGoal *pGoal);    // 编辑模式 
    LGoalDlg(QStage* pStage);  // 添加模式


protected:
    virtual LRESULT OnDocumentComplete();
    virtual BOOL OnDefaultButton(INT_PTR nBtn);
    // 检查参数的合法性
    BOOL CheckMemberParameters();

private:
    QStage*     m_pStage;
    BOOL        m_bEditMode;

public:
    QGoal*      m_pGoalInout;
};
