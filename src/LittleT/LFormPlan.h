#pragma once

#include "ui/QForm.h"
#include "ui/QDialog.h"
#include "../common/QLongplan.h"

DECLARE_USER_MESSAGE(QSOFT_LITTLET_PLANDELETE);

class LFormPlan : public QForm
{
    QUI_DECLARE_EVENT_MAP;

public:
    LFormPlan(void);
    ~LFormPlan(void);

    void RefreshPlans();

    QPlan* GetCurrentPlan()
    {
        return reinterpret_cast<QPlan*>(_PlanTitle().GetData());
    }

    void OnMenuItemNewPlan(HELEMENT,EMenuItem mi);

protected:
    void OnMenuItemEdit(HELEMENT,EMenuItem mi);
    void OnMenuItemDelete(HELEMENT,EMenuItem mi);
    void OnMenuItemNewStage(HELEMENT,EMenuItem mi);

    ECtrl _PlanBox()
    {
        return GetCtrl("#id-plan-box");
    }

    ECtrl _PlanTitle()
    {
        return GetCtrl("#id-plan-title");
    }

    ETable _CurrentPlanCtl()
    {
        return _PlanBox().find_first("table:checked");
    }

    QPlan * _PlanOfTable(ETable& ctlPlan)
    {
        ASSERT(ctlPlan.is_valid());
        return reinterpret_cast<QPlan*>(ctlPlan.GetData());
    }

    void RefreshPlanItem(ETable &tblPlan);
    ETable AddPlan(QPlan *pPlan);
    // 手动新创建了一个计划
    void NewPlanAdded(QPlan *pPlan);
    void OnPlanItemSelectChanged(HELEMENT he);
    void OnPlanItemClicked(HELEMENT he);
    /** 选中一个计划项目
     *	return:
     *      TRUE    成功
     *	params:
     *		-[in]
     *          tblPlan     需要被选中的项目
     *                      无效时选中列表中的第一个项目
    **/
    BOOL SelectPlanItem(__in ETable tblPlan = NULL);

    void OnClkChkGoalitemShow( HELEMENT he );
};

//////////////////////////////////////////////////////////////////////////
class LPlanDlg : public QDialog
{
public:
    LPlanDlg(QPlan *pPlan = NULL);    // NULL为添加模式

protected:
    virtual LRESULT OnDocumentComplete();
    virtual BOOL OnDefaultButton(INT_PTR nBtn);
    BOOL CheckDate(__out QTime &tmBegin,__out QTime &tmEnd);
    // 检查参数的合法性
    BOOL CheckMemberParameters();

private:
    BOOL        m_bEditMode;

public:
    QPlan*      m_pPlanInout;
};
