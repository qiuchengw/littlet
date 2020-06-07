#include "QDBPlan.h"
#include "QLongplan.h"

BOOL QDBPlan::Startup()
{
	return Open(L"plan.db");
}

BOOL QDBPlan::AddPlan( QPlan *p )
{
	CStdString sQ;
	sQ.Format(L"INSERT INTO tbl_plan(Plan,Des,Flag,IconID,BTime,ETime,Status,crtime,udtime,Deleted)"
		L" VALUES('%s','%s',%d,%d,%lf,%lf,%d,%lf,%lf,0)",
		SFHSQ(p->m_sPlan),SFHSQ(p->m_sDes),p->m_nFlag,p->IconID(),p->m_tmBegin.SQLDateTime(),
		p->m_tmEnd.SQLDateTime(),p->m_eStatus,QTime::SQLDateTimeCurrent(),
		QTime::SQLDateTimeCurrent());
	p->m_nID = _ExecSQL_RetLastID(sQ);
	return p->m_nID != INVALID_ID;
}

BOOL QDBPlan::DeletePlan( __in int nID )
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_plan SET Deleted=1 WHERE ID=%d", nID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::EditPlan( int nID,const CStdString& sPlan,
    const CStdString& sDes, QTime tmBegin, QTime tmEnd )
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_plan SET Plan='%s',Des='%s',"
        L"BTime=%lf,ETime=%lf,udtime=%lf "
        L" WHERE (ID=%d)",
        SFHSQ(sPlan),SFHSQ(sDes),tmBegin.SQLDateTime(),
        tmEnd.SQLDateTime(),QTime::SQLDateTimeCurrent(),nID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::GetPlan( int nPlanID,__out QPlan& p )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_plan WHERE (ID=%d)",nPlanID);
	return _Plan(ExecQuery(sQ),p);
}

BOOL QDBPlan::_Plan( SqlQuery &q, __out QPlan& p )
{
	QDB_BEGIN_TRY
		if (!q.eof())
		{
			p.m_nID = q.IntValue(L"ID");
			p.m_nIconID = q.IntValue(L"IconID");
			p.m_sPlan = q.StrValue(L"Plan");
			p.m_sDes = q.StrValue(L"Des");
			p.m_eStatus = (ENUM_PLAN_STATUS)q.IntValue(L"Status");
			p.m_nFlag = q.IntValue(L"Flag");
			p.m_tmBegin = q.DateTimeValue(L"BTime");
			p.m_tmEnd = q.DateTimeValue(L"ETime");
			p.m_tmCreate = q.DateTimeValue(L"crtime");
			p.m_tmFinish = q.DateTimeValue(L"FTime");
			p.m_tmUpdate = q.DateTimeValue(L"udtime");

			return TRUE;
		}
	QDB_END_TRY
		return FALSE;
}

int QDBPlan::GetAllPlan( __out VecPlan &vp )
{
	QDB_BEGIN_TRY
        QPlan *p;
		SqlQuery q = ExecQuery(L"SELECT * FROM tbl_plan WHERE (Deleted=0)");
		for (q.nextRow(); !q.eof(); q.nextRow())
		{
            p = new QPlan;
			if (_Plan(q, *p))
			{
				vp.push_back(p);
			}
		}
	QDB_END_TRY
		return vp.size();
}   

//////////////////////////////////////////////////////////////////////////
// stage
BOOL QDBPlan::AddStage( int nPlanID,QStage *s )
{
	CStdString sQ;
    s->m_eStatus = STAGE_STATUS_WORKING;
    s->m_tmUpdate = s->m_tmCreate = QTime::GetCurrentTime();
	sQ.Format(L"INSERT INTO tbl_stage(PlanID,Stage,Des,IconID,Flag,BTime,ETime,Status,crtime,udtime)"
			L" VALUES(%d,'%s','%s',%d,%d,%lf,%lf,%d,%lf,%lf)",
			nPlanID,SFHSQ(s->m_sStage),SFHSQ(s->m_sDes),s->IconID(),
            s->m_nFlag,s->m_tmBegin.SQLDateTime(),
			s->m_tmEnd.SQLDateTime(),s->m_eStatus,QTime::SQLDateTimeCurrent(),
			QTime::SQLDateTimeCurrent());
	s->m_nID = _ExecSQL_RetLastID(sQ);
	return s->m_nID != INVALID_ID;
}

BOOL QDBPlan::DeleteStage( int nStageID )
{
    // 删除其下的goal的逻辑应该由上层来做。
    // 如果下面还有goal，那么删除失败
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_goal WHERE (StageID=%d)", nStageID);
    if (_ExecSQL_RetInt(sQ) <= 0)
    {
        sQ.Format(L"DELETE FROM tbl_stage WHERE (ID=%d)",nStageID);
        return _ExecSQL_RetBOOL(sQ);
    }
    ASSERT(FALSE);
    return FALSE;
}

BOOL QDBPlan::GetStage( int nStageID,QStage& s )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_stage WHERE (ID=%d)",nStageID);
	return _Stage(ExecQuery(sQ),s);
}

BOOL QDBPlan::_Stage( SqlQuery &q,__out QStage &s )
{
	QDB_BEGIN_TRY
		if (!q.eof())
		{
			s.m_nID = q.IntValue(L"ID");
            s.m_nIconID = q.IntValue(L"IconID");
			s.m_nFlag = q.IntValue(L"Flag");
			s.m_eStatus = (ENUM_STAGE_STATUS)q.IntValue(L"Status");
			s.m_nPlanID = q.IntValue(L"PlanID");
			s.m_sDes = q.StrValue(L"Des");
			s.m_sStage = q.StrValue(L"Stage");
			s.m_tmBegin = q.DateTimeValue(L"BTime");
			s.m_tmEnd = q.DateTimeValue(L"ETime");
			s.m_tmCreate = q.DateTimeValue(L"crtime");
			s.m_tmUpdate = q.DateTimeValue(L"udtime");
			return TRUE;
		}
	QDB_END_TRY
		return FALSE;
}

int QDBPlan::GetPlanStages( int nPlanID,__out VecStage &vs )
{
	QDB_BEGIN_TRY
		CStdString sQ;
        QStage *s = NULL;
		sQ.Format(L"SELECT * FROM tbl_stage WHERE (PlanID=%d)",nPlanID);
		SqlQuery q = ExecQuery(sQ);
		for (q.nextRow(); !q.eof(); q.nextRow())
		{
            s = new QStage;
			if (_Stage(q,*s))
			{
				vs.push_back(s);
			}
		}
	QDB_END_TRY
		return vs.size();
}

int QDBPlan::GetGoalNumOfStage( int nStageID )
{
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_goal WHERE (StageID=%d)",nStageID);
    return _ExecSQL_RetInt(sQ);
}

int QDBPlan::GetFinishedGoalNumOfStage(int nStageID)
{
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_goal WHERE (StageID=%d) AND (Status=%d)",
            nStageID, GOAL_STATUS_CHECKED);
    return _ExecSQL_RetInt(sQ);
}

int QDBPlan::GetWorkingGoalNumOfStage(int nStageID)
{
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_goal WHERE (StageID=%d) AND (Status=%d)",
        nStageID, GOAL_STATUS_WORKING);
    return _ExecSQL_RetInt(sQ);
}

BOOL QDBPlan::EditStage( __in int nStageID, const CStdString& sStage, 
    const CStdString& sDes, QTime tmBegin, QTime tmEnd )
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_stage SET Stage='%s',Des='%s',BTime=%lf,ETime=%lf,udtime=%lf"
        L" WHERE (ID=%d)",
        SFHSQ(sStage),SFHSQ(sDes),tmBegin.SQLDateTime(),
        tmEnd.SQLDateTime(),QTime::SQLDateTimeCurrent(), nStageID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::SetStageIcon( int nStageID, int nIconID )
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_stage SET IconID=%d WHERE (ID=%d)",
                nIconID, nStageID);
    return _ExecSQL_RetBOOL(sQ);
}

//////////////////////////////////////////////////////////////////////////
// goal
BOOL QDBPlan::AddGoal( int nStageID,QGoal *g )
{
	CStdString sQ;
	sQ.Format(L"INSERT INTO tbl_goal(StageID,Goal,Des,Flag,Status,crtime,udtime)"
		L" VALUES(%d,'%s','%s',%d,%d,%lf,%lf)",
		nStageID,SFHSQ(g->Goal()),SFHSQ(g->Des()),g->Flag(),g->Status(),
		QTime::SQLDateTimeCurrent(),QTime::SQLDateTimeCurrent());
	g->m_nID = _ExecSQL_RetLastID(sQ);
	return g->m_nID != INVALID_ID;
}

BOOL QDBPlan::GetGoal( int nGoalID,__out QGoal& g )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_goal WHERE (ID=%d)",nGoalID);
	return _Goal(ExecQuery(sQ),g);
}

BOOL QDBPlan::_Goal( SqlQuery &q,__out QGoal& g )
{
	QDB_BEGIN_TRY
		if (!q.eof())
		{
			g.m_nID = q.IntValue(L"ID");
			g.m_nFlag = q.IntValue(L"Flag");
			g.m_eStatus = (ENUM_GOAL_STATUS)q.IntValue(L"Status");
			g.m_sDes = q.StrValue(L"Des");
			g.m_sGoal = q.StrValue(L"Goal");
			g.m_nStageID = q.IntValue(L"StageID");
			g.m_tmCreate = q.DateTimeValue(L"crtime");
			g.m_tmUpdate = q.DateTimeValue(L"udtime");

			return TRUE;
		}
	QDB_END_TRY
		return FALSE;
}

int QDBPlan::GetStageGoals( int nStageID,__out VecGoal& vg )
{
	QDB_BEGIN_TRY
		CStdString sQ;
		sQ.Format(L"SELECT * FROM tbl_goal WHERE (StageID=%d)",nStageID);
		SqlQuery q = ExecQuery(sQ);
        QGoal *g;
		for (q.nextRow(); !q.eof(); q.nextRow())
		{
            g = new QGoal;
			if (_Goal(q, *g))
			{
				vg.push_back(g);
			}
		}
	QDB_END_TRY
		return vg.size();
}

BOOL QDBPlan::EditGoal( int nGoalID, const CStdString& sGoal, const CStdString& sDes)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_goal SET Goal='%s',Des='%s',udtime=%lf"
        L" WHERE (ID=%d)",
        SFHSQ(sGoal),SFHSQ(sDes),QTime::SQLDateTimeCurrent(), nGoalID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::DeleteGoal( int nGoalID )
{
    // 删除其下的goalitem的逻辑应该由上层来做。
    // 如果下面还有goalitem，那么删除失败
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_goalitem WHERE (GoalID=%d)", nGoalID);
    if (_ExecSQL_RetInt(sQ) <= 0)
    {
        sQ.Format(L"DELETE FROM tbl_goal WHERE (ID=%d)",nGoalID);
        return _ExecSQL_RetBOOL(sQ);
    }
    ASSERT(FALSE);
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
BOOL QDBPlan::AddGoalItem( int nGoalID,__inout QGoalItem *gi )
{
	CStdString sQ;
	sQ.Format(L"INSERT INTO tbl_goalitem(GoalID,Item,Status,TimerID,BTime,ETime,Attachs,crtime,udtime) "
				L"VALUES(%d,'%s',%d,%d,%lf,%lf,'%s',%lf,%lf)",
				nGoalID,gi->Item(),gi->Status(),gi->TimerID(),gi->BeginTime().SQLDateTime(),
				gi->EndTime().SQLDateTime(),gi->Attachs(),
				QTime::SQLDateTimeCurrent(),QTime::SQLDateTimeCurrent());
	gi->m_nID = _ExecSQL_RetLastID(sQ);
	return INVALID_ID != gi->ID();
}

BOOL QDBPlan::GetGoalItem( int nItemID,__out QGoalItem& gi )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_goalitem WHERE (ID=%d)",nItemID);
	SqlQuery q = ExecQuery(sQ);
	return _GoalItem(q,gi);
}

BOOL QDBPlan::_GoalItem( SqlQuery &q,__out QGoalItem &gi )
{
	QDB_BEGIN_TRY
		if (!q.eof())
		{
			gi.m_nID = q.IntValue(L"ID");
			gi.m_nGoalID = q.IntValue(L"GoalID");
			gi.m_sItem = q.StrValue(L"Item");
            gi.m_nTimerID = q.IntValue(L"TimerID");
			gi.m_tmBegin = q.DateTimeValue(L"BTime");
			gi.m_tmEnd = q.DateTimeValue(L"ETime");
			gi.m_tmCreate = q.DateTimeValue(L"crtime");
			gi.m_tmUpdate = q.DateTimeValue(L"udtime");
			gi.m_eStatus = (ENUM_GOALITEM_STATUS)q.IntValue(L"Status");
			gi.m_sAttachs = q.StrValue(L"Attachs");
			//gi.m_sNotes = q.StrValue(L"Notes");

			return TRUE;
		}
	QDB_END_TRY
		return FALSE;
}

int QDBPlan::GetItemsOfGoal( int nGoalID,__out VecGoalItem &vgi,ENUM_GOALITEM_STATUS eStatus )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_goalitem WHERE (GoalID=%d)",nGoalID);
	if (GOALITEM_STATUS_NOTSET != eStatus)
	{
		CStdString sPart;
		sPart.Format(L" AND (Status=%d)",eStatus);
		sQ += sPart;
	}
	QDB_BEGIN_TRY
		SqlQuery q = ExecQuery(sQ);
        QGoalItem *gi;
        for (q.nextRow(); !q.eof(); q.nextRow())
		{
            gi = new QGoalItem;
			if (_GoalItem(q, *gi))
			{
				vgi.push_back(gi);
			}
		}
	QDB_END_TRY
		return vgi.size();
}

int QDBPlan::GetItemNumOfGoal( int nGoalID )
{
	CStdString sQ;
	sQ.Format(L"SELECT COUNT(ID) FROM tbl_goalitem WHERE (GoalID=%d)",nGoalID);
	return _ExecSQL_RetInt(sQ);
}

int QDBPlan::GetUnfinishedItemNumOfGoal( int nGoalID )
{
	CStdString sQ;
	sQ.Format(L"SELECT COUNT(ID) FROM tbl_goalitem WHERE (GoalID=%d) AND (Status=%d)",
		nGoalID,GOALITEM_STATUS_WAITING);
	return _ExecSQL_RetInt(sQ);
}

BOOL QDBPlan::EditGoalItem( int nGoalItemID, const CStdString& sItem, QTime tmBegin, QTime tmEnd )
{
	CStdString sQ;
	sQ.Format(L"UPDATE tbl_goalitem SET Item='%s',BTime=%lf,"
		L"ETime=%lf,udtime=%lf WHERE(ID=%d)",
		sItem, tmBegin.SQLDateTime(), tmEnd.SQLDateTime(), 
        QTime::SQLDateTimeCurrent(), nGoalItemID);
	return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::DeleteGoalItem( int nGoalItemID )
{
	CStdString sQ;
	sQ.Format(L"DELETE FROM tbl_goalitem WHERE (ID=%d)",nGoalItemID);
	return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::SetGoalItemNotes(int nGoalItemID,const CStdString &sNotes)
{
    ASSERT(FALSE);
    return FALSE;

	CStdString sQ;
	sQ.Format(L"UPDATE tbl_goalitem SET Notes='%s' WHERE (ID=%d)",
		SFHSQ(sNotes),nGoalItemID);
	return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::SetGoalItemTimer(int nGoalItemID,int nTimerID)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_goalitem SET TimerID=%d WHERE (ID=%d)",
        nTimerID,nGoalItemID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::SetGoalItemStagus(int nGoalItemID, ENUM_GOALITEM_STATUS eStatus)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_goalitem SET Status=%d WHERE (ID=%d)",
        eStatus,nGoalItemID);
    return _ExecSQL_RetBOOL(sQ);
}

//////////////////////////////////////////////////////////////////////////
// Note
BOOL QDBPlan::_Note( SqlQuery&q , __out QNote&n )
{
	QDB_BEGIN_TRY
		if (!q.eof())
		{
			n.m_nID = q.IntValue(L"ID");
			n.m_eType = (ENUM_NOTE_TYPE)q.IntValue(L"Typex");
			n.m_lExtraFlag = q.IntValue(L"ExtraFlag");
			n.m_sExtra = q.StrValue(L"Extra");
			n.m_lFlag = q.IntValue(L"Flag");
			n.m_sAttachs = q.StrValue(L"Attachs");
			n.m_sTags = q.StrValue(L"Tags");
			QBuffer bufContent;
			if (q.BlobValue(L"Content",bufContent))
			{
				n.m_sContent = (LPCWSTR)bufContent.GetBuffer(0);
			}
			n.m_sOutline = q.StrValue(L"Outline");
			n.m_sTitle = q.StrValue(L"Title");
			n.m_tmCreate = q.DateTimeValue(L"crtime");
			n.m_tmUpdate = q.DateTimeValue(L"udtime");
			return TRUE;
		}
	QDB_END_TRY
		return FALSE;
}

int QDBPlan::_Notes( SqlQuery&q,__out VecNote&vn )
{
	for (q.nextRow(); !q.eof(); q.nextRow())
	{
		QNote n;
		if (_Note(q,n))
		{
			vn.push_back(n);
		}
	}
	return vn.size();
}

BOOL QDBPlan::AddNote( QNote *n )
{
	CStdString sQ;
	sQ.Format(L"INSERT INTO tbl_note(Typex,ParentID,Title,Outline,Content,Attachs,Tags,crtime,udtime) "
		L" VALUES(%d,%d,'%s','%s',?,'%s','%s',%lf,%lf)",
		n->Type(),n->ParentID(),SFHSQ(n->Title()),SFHSQ(n->Outline()),
		SFHSQ(n->Attachs()),SFHSQ(n->Tags()),
		QTime::SQLDateTimeCurrent(),QTime::SQLDateTimeCurrent());
	QBuffer bufContent;
	bufContent.Write(n->Content());
	n->m_nID = _ExecSQL_RetLastID(sQ,bufContent);
	return n->m_nID != INVALID_ID;
}

BOOL QDBPlan::DeleteNote( int nID )
{
	CStdString sQ;
	sQ.Format(L"DELETE FROM tbl_note WHERE (ID=%d)",nID);
	return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBPlan::GetNote( int nID,__out QNote& n )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_note WHERE (ID=%d)",nID);
	return _Note(ExecQuery(sQ),n);
}

BOOL QDBPlan::UpdateNote( QNote *n )
{
	CStdString sQ;
	sQ.Format(L"UPDATE tbl_note SET Typex=%d,ParentID=%d,Title='%s',"
			L"Outline='%s',Content=?,Attachs='%s',Tags='%s',udtime=%lf "
			L" WHERE (ID=%d)",
		n->Type(),n->ParentID(),SFHSQ(n->Title()),SFHSQ(n->Outline()),
		SFHSQ(n->Attachs()),SFHSQ(n->Tags()),QTime::SQLDateTimeCurrent(),n->ID());
	QBuffer bufContent;
	bufContent.Write(n->Content());
	_ExecSQL_RetLastID(sQ,bufContent);
	return TRUE;
}

int QDBPlan::GetNote_WithParent( int nParentID/*=INVALID_ID*/,__out VecNote& vn )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_note WHERE (ParentID=%d)",nParentID);
	return _Notes(ExecQuery(sQ),vn);
}

int QDBPlan::GetNote_WithType( ENUM_NOTE_TYPE eType,__out VecNote& vn )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_note WHERE (Typex=%d)",eType);
	return _Notes(ExecQuery(sQ),vn);
}

int QDBPlan::GetNote_WithTags( const CStdString& sTag,__out VecNote& vn )
{
	CStdString sQ;
	sQ.Format(L"SELECT * FROM tbl_note WHERE Tags LIKE '%%%s%%",sTag);
	return _Notes(ExecQuery(sQ),vn);
}

int QDBPlan::GetAllNote( __out VecNote&vn )
{
	return _Notes(ExecQuery(L"SELECT * FROM tbl_note"),vn);
}

