#pragma once

#include "ui/QWindow.h"
#include "BaseType.h"
#include "../common/QAutoTask.h"

class QExecTimeDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP;

public:
	QExecTimeDlg();
	QExecTimeDlg(const QTime& tmBegin,const QTime& tmEnd,LPCWSTR sExpWhen);
	~QExecTimeDlg(void);

protected:
	void OnSelectDatetimeChanged(HELEMENT hSelc);
	void OnClkChkSpan(HELEMENT hBtn);
	void OnClkChkExecCount(HELEMENT hBtn);
	void OnClkMonthDaySelectAll(HELEMENT hBtn);
    void OnClkMonthDaySelectNone(HELEMENT hBtn);
    void OnClkAddTimeSpot(HELEMENT hBtn);
	void OnClkDeleteTimeRow(HELEMENT hBtn);

	virtual BOOL OnDefaultButton(INT_PTR nID);
	virtual LRESULT OnDocumentComplete();

    void OnCmbTaskWhenDoSelectChanged(ECombobox cmb, EOption item);
	BOOL GetRelateExp(__out QString & sExp);
	void GetLifeTime(QTime& tmBegin,QTime& tmEnd);
	BOOL GetAbsoluteExp( __out QString &sExp ,QTime&tmB,QTime &tmE);

    void OnAbsDateRioCheckChanged(HELEMENT);
protected:
    ETabCtrl _TabTime() { return GetCtrl("#TABS_TIME"); }
    ECtrl _TabItemRelate() { return _TabTime().GetTab(L"panel_relate"); }
    ECtrl _TabItemAbs() { return _TabTime().GetTab(L"panel_abs"); }
    ESheet _ListTimes() { return GetCtrl("#lst_times"); }

private:
	BOOL		m_bEditMode;

public:
	QTime		m_tmLifeBegin,m_tmLifeEnd;
	QString		m_sExp;
};

class LAddEventDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP;

    SINGLETON_ON_DESTRUCTOR(LAddEventDlg)
    {

    }

    LAddEventDlg::LAddEventDlg()
        :QDialog(L"qabs:addtask/autotask.htm")
    {

    }
public:
    static BOOL EditEvent(QAutoTask *pTask);
    static QAutoTask* NewEvent();

protected:
    void OnSelectChangedDoWhat( ECombobox cmb, EOption item );
    void OnClkChkTaskRemind(HELEMENT );
	void OnClkExecTime(HELEMENT );
	void OnClkChkRemindPlaySound(HELEMENT hBtn);
	void OnClkChkRemindCustomMsg(HELEMENT hBtn);

	BOOL OnDefaultButton(INT_PTR nID);

	virtual LRESULT OnDocumentComplete();

	ENUM_AUTOTASK_DOWHAT GetDoWhat();
	BOOL GetTaskString(QString &sTask);
	BOOL GetRemindExp(QString &sRmdExp);

	void SetDoWhat( ENUM_AUTOTASK_DOWHAT eDo ,const QString& sTask );
	void SetRemindMsg(const QString &sMsg);
	void SetExecProgPath(const QString &sMsg);
	void SetRemindExp( const QString &sRmdExp );

	EEdit _InputRemindText() { return GetCtrl("#INPUT-REMINDTEXT"); }
    void RefreshWhenDo(QTimer* pTimer);

private:
	BOOL		m_bEditMode;	// 添加模式还是编辑模式
	QTime		m_tmLifeBegin,m_tmLifeEnd;
	QString		m_sTimerExp;

public:
	QAutoTask*	m_pTask;
};

