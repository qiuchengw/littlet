#include "QLongplan.h"
#include "../common/QDBPlan.h"
#include "AppHelper.h"

QString GetTimeFleeText(QTimeSpan &tmSpan)
{
	QString sRet,sPart;
	int nDays = tmSpan.GetDays();
	if (nDays > 365)
		sPart.Format(L"%d年",nDays / 365);
	sRet += sPart;
	nDays %= 365;
	sPart = L"";
	if (nDays > 30)
		sPart.Format(L"%d月",nDays / 30);
	sRet += sPart;
	nDays %= 30;
	sPart = L"";
	sPart.Format(L"%d天",nDays);
	sRet += sPart;

	return sRet;
}

/////////////////////////////////////////////////////////////////////////
BOOL ParseID( const QString&sID,__out IntArray &vi )
{
	if (sID.IsEmpty())
		return FALSE;

	QStrEx strEx(L';',sID);
	strEx.Unique(TRUE);
	strEx.GetAll(vi);

	return TRUE;
}

QString MakeIDS( __in IntArray &vi )
{
	QString sRet,sPart;
	for (int i = 0; i < vi.size(); i++)
	{
		sPart.Format(L"%d;",vi[i]);
		sRet += sPart;
	}
	return sRet;
}

//////////////////////////////////////////////////////////////////////////

int QGoalItem::GetAttachs( __out VecFileData &vfd, BOOL bIncludeData/*=FALSE*/ )
{
	IntArray vi;
	if (ParseID(Attachs(),vi))
	{
        return QResMan::GetInstance()->GetDataItems(vi,vfd,bIncludeData);
	}
	return 0;
}

int QGoalItem::AddNotes( int nNoteID )
{
	QString sID;
	sID.Format(L"%d;",nNoteID);
	m_sNotes += sID;

	IntArray vi;
	if (ParseID(Notes(),vi))
	{
		m_sNotes = MakeIDS(vi);
		QDBPlan::GetInstance()->SetGoalItemNotes(ID(),Notes());
		return vi.size();
	}
	return 0;
}

BOOL QGoalItem::Edit( const QString& sItem, QTime tmBegin, QTime tmEnd )
{
    if (QDBPlan::GetInstance()->EditGoalItem(ID(), sItem, tmBegin, tmEnd))
    {
        m_sItem = sItem;
        m_tmBegin = tmBegin;
        m_tmEnd = tmEnd;
        m_tmUpdate = QTime::GetCurrentTime();

        return TRUE;
    }
    return FALSE;
}

QGoalItem& QGoalItem::operator=( const QGoalItem& o )
{
    m_sItem = o.m_sItem;
    m_nGoalID = o.m_nGoalID;
    m_nTimerID = o.m_nTimerID;
    m_tmBegin = o.m_tmBegin;
    m_tmEnd = o.m_tmEnd;
    m_eStatus = o.m_eStatus;
    m_sAttachs = o.m_sAttachs;
    m_sNotes = o.m_sNotes;
    m_nID = o.m_nID;
    m_tmCreate = o.m_tmCreate;
    m_tmUpdate = o.m_tmUpdate;

    return *this;
}

QGoalItem::QGoalItem( const QGoalItem& g )
{
    this->operator =(g);
}

BOOL QGoalItem::CheckIt( BOOL bChecked /*= TRUE*/ )
{
    ENUM_GOALITEM_STATUS eStatus = (bChecked ? GOALITEM_STATUS_GAIN : GOALITEM_STATUS_WAITING);
    BOOL bOK = QDBPlan::GetInstance()->SetGoalItemStagus(ID(), eStatus);
    if ( bOK )
    {
        m_eStatus = eStatus;
    }
    return bOK;
}

BOOL QGoalItem::Delete()
{
    // 先删除下面所有的定时器
    // 暂时不实现
    return QDBPlan::GetInstance()->DeleteGoalItem(ID());
}

//////////////////////////////////////////////////////////////////////////
QGoal::QGoal()
{
	m_eStatus = GOAL_STATUS_WORKING;
	m_nID = INVALID_ID;
	m_nFlag = 0;
}

QGoal::QGoal( const QGoal& g )
{
    this->operator=(g);
}

BOOL QGoal::AddItem( __inout QGoalItem &gi )
{
	return QDBPlan::GetInstance()->AddGoalItem(ID(),&gi);
}

BOOL QGoal::GetAllItems( __out VecGoalItem &vgi,ENUM_GOALITEM_STATUS eStatus )
{
	return QDBPlan::GetInstance()->GetItemsOfGoal(ID(),vgi,eStatus);
}

int QGoal::GetGoalItemUnfinishedNum()
{
	return QDBPlan::GetInstance()->GetUnfinishedItemNumOfGoal(ID());
}

int QGoal::GetGoalItemNum()
{
	return QDBPlan::GetInstance()->GetItemNumOfGoal(ID());
}

QString QGoal::TimeFlee()
{
	return GetTimeFleeText(QTime::GetCurrentTime() - CreationTime());
}

BOOL QGoal::DeleteItem( QGoalItem *gi )
{
    return QDBPlan::GetInstance()->DeleteGoalItem(gi->ID());
}

BOOL QGoal::EditGoal( const QString& sGoal, const QString& sDes )
{
    if (QDBPlan::GetInstance()->EditGoal(ID(), sGoal, sDes))
    {
        m_sGoal = sGoal;
        m_sDes = sDes;
        m_tmUpdate = QTime::GetCurrentTime();
        return TRUE;
    }
    return FALSE;
}

QGoalItem* QGoal::AddGoalItem( __in const QGoalItem &gi )
{
    QGoalItem* pGI = new QGoalItem(gi);
    if ( QDBPlan::GetInstance()->AddGoalItem(ID(), pGI) )
    {
        return pGI;
    }
    delete pGI;
    return NULL;
}

QGoal& QGoal::operator=( const QGoal& o )
{
    m_sDes = o.m_sDes;
    m_sGoal = o.m_sGoal;
    m_nID = o.m_nID;
    m_nStageID = o.m_nStageID;
    m_nFlag = o.m_nFlag;
    m_eStatus = o.m_eStatus;
    m_tmUpdate = o.m_tmUpdate;
    m_tmCreate = o.m_tmCreate;

    return *this;
}

BOOL QGoal::Delete()
{
    // 先删除下面所有的goalitem
    VecGoalItem vi;
    GetAllItems(vi, GOALITEM_STATUS_NOTSET);
    QGoalItem *pGI;
    while (!vi.empty())
    {
        pGI = vi.back();
        VERIFY(pGI->Delete());
        delete pGI;
        vi.pop_back();
    }
    return QDBPlan::GetInstance()->DeleteGoal(ID());
}

BOOL QGoal::DeleteGoalitem( QGoalItem* pGoalitem )
{
    if (NULL == pGoalitem)
    {
        ASSERT(FALSE);
        return FALSE;
    }
    return pGoalitem->Delete();
}

//////////////////////////////////////////////////////////////////////////
QPlan::QPlan()
{
	m_nID = INVALID_ID;
    m_nIconID = INVALID_ID;
	m_nFlag = 0;
	m_eStatus = PLAN_STATUS_OK;
}

QPlan::QPlan( const QPlan& p )
{
    this->operator =(p);
}

QPlan::~QPlan(void)
{

}

BOOL QPlan::HungupTask()
{

	return FALSE;
}

BOOL QPlan::IsRunning() const
{
	return FALSE;
}

int QPlan::GetStageCount()
{

	return 0;
}

int QPlan::GetFinishStageCount()
{
	return 0;

}

int QPlan::GetAllStage( __out VecStage & vs )
{
	return QDBPlan::GetInstance()->GetPlanStages(ID(),vs);
}

int QPlan::GetPlanProcess()
{
	return 0;
}

int QPlan::GetTimeProcess()
{
    QTime tmNow = QTime::GetCurrentTime();
    if (tmNow.CompareDate(BeginTime()) > 0)
        return 100; // 时间已经过完了。

    int nTotalDays = (EndTime() - BeginTime()).GetTotalDays();
    if ( nTotalDays <= 0 )   // 开始和结束是同一天
        return 100;

    int nEllipseDay = (tmNow - BeginTime()).GetTotalDays();
    if (nEllipseDay <= 0)   // 不足一天，按照一天计算
        nEllipseDay = 1;

	return ( nEllipseDay * 100 / nTotalDays) / 100;
}

QString QPlan::GetTimeFlee()
{
	QTimeSpan tmSpan = QTime::GetCurrentTime() - BeginTime();

	return GetTimeFleeText(tmSpan);
}

QPlan& QPlan::operator=( const QPlan& o )
{
	m_nFlag = o.m_nFlag;
	m_tmBegin = o.m_tmBegin;
	m_tmEnd = o.m_tmEnd;
	m_sPlan = o.m_sPlan;
	m_sDes = o.m_sDes;
	//m_nIconID = o.m_nIconID;
	m_nID = o.ID();
	m_tmCreate = o.CreationTime();
	m_tmUpdate = o.UpdateTime();
	m_tmFinish = o.FinishedTime();
	m_eStatus = o.Status();

	return *this;
}

QStage* QPlan::AddStage( const QStage &s )
{
    QStage *pStage = new QStage(s);
    if (QDBPlan::GetInstance()->AddStage(ID(), pStage))
    {
        return pStage;
    }
    delete pStage;
    return NULL;
}

BOOL QPlan::Edit( const QString& sPlan, const QString& sDes, QTime tmBegin, QTime tmEnd )
{
    if (QDBPlan::GetInstance()->EditPlan(ID(), sPlan, sDes, tmBegin, tmEnd))
    {
        m_sDes = sDes;
        m_sPlan = sPlan;
        m_tmBegin = tmBegin;
        m_tmEnd = tmEnd;
        m_tmUpdate = QTime::GetCurrentTime();

        return TRUE;
    }
    return FALSE;
}

BOOL QPlan::DeleteStage( QStage *pStage )
{
    if (NULL == pStage)
    {
        ASSERT(FALSE);
        return FALSE;
    }
    return pStage->Delete();
}

BOOL QPlan::Delete()
{
    // 并非真正的删除掉自己，只是标记为删除而已
    return QDBPlan::GetInstance()->DeletePlan(ID());
}

QString QPlan::IconPath() const
{
    return QResMan::GetInstance()->GetDBResFilePath(IconID());
}

//////////////////////////////////////////////////////////////////////////
QStage::QStage()
{
	m_nFlag = 0;
	m_nID = INVALID_ID;
    m_nIconID = INVALID_ID;
	m_eStatus = STAGE_STATUS_WAITING;
}

QStage::QStage( const QStage& g )
{
    this->operator =(g);
}

QStage& QStage::operator=( const QStage& o )
{
    m_sStage = o.m_sStage;
    m_sDes = o.m_sDes;
    m_nID = o.m_nID;
    m_nFlag = o.m_nFlag;
    m_tmBegin = o.m_tmBegin;
    m_tmEnd = o.m_tmEnd;
    m_tmCreate = o.m_tmCreate;
    m_tmUpdate = o.m_tmUpdate;
    m_eStatus = o.m_eStatus;

    return *this;
}

int QStage::GetGoalNum()
{
    return QDBPlan::GetInstance()->GetGoalNumOfStage(ID());
}

int QStage::GetFinishedGoalNum()
{
    return QDBPlan::GetInstance()->GetFinishedGoalNumOfStage(ID());
}

int QStage::GetWorkingGoalNum()
{
    return QDBPlan::GetInstance()->GetWorkingGoalNumOfStage(ID());
}

int QStage::GetKonwledgeNum()
{
	return 0;
}

int QStage::GetAllGoals( __out VecGoal &vg )
{
	return QDBPlan::GetInstance()->GetStageGoals(ID(),vg);
}

QGoal* QStage::AddGoal( __in const QGoal& g )
{
    QGoal* pGoal = new QGoal(g);
    if ( QDBPlan::GetInstance()->AddGoal(ID(), pGoal) )
    {
        return pGoal;
    }
    delete pGoal;
    return NULL;
}

int QStage::GetStageProcess()
{
	return 0;
}

int QStage::GetTimeProcess()
{
    QTime tmNow = QTime::GetCurrentTime();
    if (tmNow.CompareDate(BeginTime()) > 0)
        return 100; // 时间已经过完了。

    int nTotalDays = (EndTime() - BeginTime()).GetTotalDays();
    if ( nTotalDays <= 0 )   // 开始和结束是同一天
        return 100;

    int nEllipseDay = (tmNow - BeginTime()).GetTotalDays();
    if (nEllipseDay <= 0)   // 不足一天，按照一天计算
        nEllipseDay = 1;

    return ( nEllipseDay * 100 / nTotalDays) / 100;
}

QString QStage::GetTimeFlee()
{
	return GetTimeFleeText(QTime::GetCurrentTime() - BeginTime());
}

BOOL QStage::Edit( const QString& sStage, 
    const QString& sDes, QTime tmBegin, QTime tmEnd )
{
    if (QDBPlan::GetInstance()->EditStage(ID(), sStage, sDes, tmBegin, tmEnd))
    {
        m_tmUpdate = QTime::GetCurrentTime();
        m_sStage = sStage;
        m_sDes = sDes;
        m_tmBegin = tmBegin;
        m_tmEnd = tmEnd;
        return TRUE;
    }
    return FALSE;
}

BOOL QStage::Delete()
{
    // 先删除下面所有的goal
    VecGoal vg;
    GetAllGoals(vg);
    QGoal* pGoal = NULL;
    while (!vg.empty())
    {
        pGoal = vg.back();
        VERIFY(pGoal->Delete());
        delete pGoal;
        vg.pop_back();
    }
    return QDBPlan::GetInstance()->DeleteStage(ID());
}

BOOL QStage::DeleteGoal( QGoal *pGoal )
{
    if (NULL == pGoal)
    {
        ASSERT(FALSE);
        return FALSE;
    }
    return pGoal->Delete();
}

QString QStage::IconPath() const
{
    return QResMan::GetInstance()->GetDBResFilePath(IconID());
}

BOOL QStage::SetIcon( int nIconID )
{
    if (QDBPlan::GetInstance()->SetStageIcon(ID(), nIconID))
    {
        m_nIconID = nIconID;
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
BOOL QPlanMan::Startup()
{
    QDBPlan *pDBPlan = QDBPlan::GetInstance();
    if (pDBPlan->Startup())
    {
        pDBPlan->GetAllPlan(m_plans);
        return TRUE;
    }
    ASSERT(FALSE);
    return FALSE;
}

QPlan* QPlanMan::GetPlan( int nID )
{
    VecPlanItr i = FindPlan(nID);
    if (_End() != i)
    {
        return (*i);
    }
    return NULL;
}

VecPlanItr QPlanMan::FindPlan( int nID )
{
    VecPlanItr iEnd = _End();
    for (VecPlanItr i = _Begin(); i != iEnd; ++i)
    {
        if ((*i)->ID() == nID)
        {
            return i;
        }
    }
    return iEnd;
}

BOOL QPlanMan::DeletePlan( int nID )
{
    VecPlanItr i = FindPlan(nID);
    if (_End() == i)
        return FALSE;
    QPlan *pPlan = *i;
    ASSERT(pPlan->ID() == nID);
    if (pPlan->Delete())
    {
        delete *i;
        m_plans.erase(i);
        return TRUE;
    }
    return FALSE;
}

QPlan* QPlanMan::AddPlan( QPlan plan )
{
    QPlan *pNew = new QPlan(plan);
    if (QDBPlan::GetInstance()->AddPlan(pNew))
    {
        m_plans.push_back(pNew);
        return pNew;
    }
    delete pNew;
    return NULL;
}

void QPlanMan::ClearMem()
{
    for (VecPlanItr i = m_plans.begin(); i != m_plans.end(); ++i)
    {
        delete *i;
    }
    m_plans.clear();
}

int QPlanMan::GetWorkingPlanNum()
{
    int nRet = 0;
    for (VecPlanItr i = m_plans.begin(); i != m_plans.end(); ++i)
    {
        if ((*i)->Status() == PLAN_STATUS_OK)
        {
            nRet++;
        }
    }
    return nRet;
}
