#pragma once

#include "../common/QLongplan.h"
#include "../common/QDBPlan.h"

#include "ui/QWindow.h"
#include "LFormPlan.h"
#include "LFormStage.h"
#include "LFormGoal.h"

class LViewPlan : public QView
{
    QUI_DECLARE_EVENT_MAP;
    QUI_DECLARE_REFLECT_EVENT_MAP;

    BEGIN_MSG_MAP_EX(LViewTodo)
        MSG_WM_KEYDOWN(OnKeyDown)
        BEGIN_QUI_MSG
            // plan
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_PLANSELCHANGED, OnPlanSelect)
            // stage
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_STAGEADDED, OnStageAdded)
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_STAGEDELETED, OnStageDeleted)
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_STAGESELCHANGED, OnStageSelChanged)
            // goal
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_GOALADDED, OnGoalAdded)
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_GOALDELETED, OnGoalDeleted)
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_GOALSELCHANGED, OnGoalSelChanged)
            // goal sub item
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_GOALSUBITEMADDED, OnGoalSubitemAdded)
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_GOALSUBITEMDELETED, OnGoalSubitemDeleted)
            MSG_QUI_CODE(VIEWPLAN_NOTIFY_GOALSUBITEMSTATUSCHANGED, OnGoalSubitemStatusChanged)
        END_QUI_MSG
        CHAIN_MSG_MAP(QView)
    END_MSG_MAP()

public:
    LViewPlan(void);
    ~LViewPlan(void);

    void OnClkNewPlan( HELEMENT hBtn );

protected:
    void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);
    virtual LRESULT OnDocumentComplete();
    void OnPlanSelectChanged(HELEMENT he);
    // void OnStageItemChanged(HELEMENT he);
    // 
    void OnClkGoalCtl(HELEMENT he);

protected:
    void OnPlanSelect(LPARAM lParam);

    // stage
    void OnStageAdded(LPARAM lParam);
    void OnStageDeleted(LPARAM lParam);
    void OnStageSelChanged(LPARAM lParam);

    // goal
    void OnGoalAdded(LPARAM lParam);
    void OnGoalDeleted(LPARAM lParam);
    void OnGoalSelChanged(LPARAM lParam);

    // goal item
    void OnGoalSubitemAdded(LPARAM lParam);
    void OnGoalSubitemDeleted(LPARAM lParam);
    void OnGoalSubitemStatusChanged(LPARAM lParam);

    void OnClkChkGoalitemShow( HELEMENT he );

protected:
    ETable _CurrentGoalCtl()
    {
        return GetCtrl("#id-goal-box>table[name=\"goal-item\"]:checked");
    }

    void SetCurrentGoal();

    void RefreshCurrentGoal();

    // 通知上级计划数目变化
    void NotifyPlanNumChanged();
private:
    LFormPlan   m_formPlan;
    LFormGoal   m_formGoal;
    LFormStage  m_formStage;
    QPlan*      m_pPlan;
};

