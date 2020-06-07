#pragma once

#include "file/db/QDBMgr.h"
#include "../common/QLongplan.h"
#include "../common/QNote.h"

class QDBPlan : public DBMan
{
    friend class QPlanMan;

	SINGLETON_ON_DESTRUCTOR(QDBPlan){}

public:
	BOOL Startup();

protected:
    friend class QPlan;
	// 如果成功，则ID写入到p::m_nID
	BOOL AddPlan( __inout QPlan *p );
    BOOL DeletePlan(__in int nID);
	BOOL EditPlan(int nID,const CStdString& sPlan, const CStdString& sDes, QTime tmBegin, QTime tmEnd);
	BOOL GetPlan(int nPlanID,__out QPlan& p);
	int GetAllPlan(__out VecPlan &vp);
	BOOL _Plan(SqlQuery &q, __out QPlan& p);

protected: // stage
    friend class QStage;
	BOOL AddStage(int nPlanID,QStage *s);
    BOOL DeleteStage(int nStageID);
	BOOL GetStage(int nStageID,QStage& s);
	BOOL _Stage(SqlQuery &q,__out QStage &s);
	int GetPlanStages(int nPlanID,__out VecStage &vs);
    int GetGoalNumOfStage(int nStageID);
    int GetFinishedGoalNumOfStage(int nStageID);
    int GetWorkingGoalNumOfStage(int nStageID);
    BOOL EditStage(__in int nStageID, const CStdString& sStage, 
        const CStdString& sDes, QTime tmBegin, QTime tmEnd);
    BOOL SetStageIcon(int nStageID, int nIconID);

protected: // goal
    friend class QGoal;
	BOOL AddGoal( int nStageID,QGoal *g );
	BOOL GetGoal(int nGoalID,__out QGoal& g);
	BOOL _Goal(SqlQuery &q,__out QGoal& g);
	int GetStageGoals(int nStageID,__out VecGoal& vg);
    BOOL EditGoal( int nGoalID, const CStdString& sGoal, const CStdString& sDes);
    BOOL DeleteGoal(int nGoalID);

protected: // goalitem
    friend class QGoalItem;
	BOOL AddGoalItem(int nGoalID,__inout QGoalItem *gi);
	BOOL DeleteGoalItem(int nGoalItemID);
	BOOL GetGoalItem(int nItemID,__out QGoalItem& gi);
	BOOL _GoalItem(SqlQuery &q,__out QGoalItem &gi);
    BOOL EditGoalItem( int nGoalItemID, const CStdString& sItem, QTime tmBegin, QTime tmEnd );
    BOOL SetGoalItemNotes(int nGoalItemID,const CStdString &sNotes);
    BOOL SetGoalItemTimer(int nGoalItemID,int nTimerID);
    BOOL SetGoalItemStagus(int nGoalItemID, ENUM_GOALITEM_STATUS eStatus);
	int GetItemNumOfGoal(int nGoalID);
	int GetItemsOfGoal(int nGoalID,__out VecGoalItem &vgi,ENUM_GOALITEM_STATUS eStatus);
	int GetUnfinishedItemNumOfGoal( int nGoalID );

protected: // note
    friend class QNote;
	BOOL _Note(SqlQuery&q , __out QNote&n);
	int _Notes(SqlQuery&q,__out VecNote&vn);
	BOOL AddNote(QNote *n);
	BOOL UpdateNote(QNote *n);
	BOOL DeleteNote(int nID);
	BOOL GetNote(int nID,__out QNote& n);
	int GetNote_WithParent(int nParentID,__out VecNote& vn);
	int GetNote_WithType(ENUM_NOTE_TYPE eType,__out VecNote& vn);
	int GetNote_WithTags(const CStdString& sTag,__out VecNote& vn);
	int GetAllNote(__out VecNote&vn);
};

