#pragma once

#include "BaseType.h"
#include "time/QTime.h"
// CREATE TABLE [tbl_note] (
// 	[ID] INTEGER PRIMARY KEY AUTOINCREMENT DEFAULT (1), 
// 	[Typex] INT(4) NOT NULL,  
// 	[ParentID] INT(4) NOT NULL,  
// 	[Title] VARCHAR(50) NOT NULL, 
// 	[Outline] VARCHAR(50) NOT NULL, 
// 	[Content] BLOB NOT NULL, 
// 	[Attachs] TEXT NOT NULL,
// 	[Extra] TEXT,
// 	[ExtraFlag] INT(4) DEFAULT (0),
// 	[Flag] INT(4) NOT NULL DEFAULT (0),  
// 	[crtime] DOUBLE(8) NOT NULL, 
// 	[udtime] DOUBLE(8) NOT NULL);

enum ENUM_NOTE_TYPE
{
	NOTE_TYPE_NOTSET,	// ÎÞ¹ØÁª
	NOTE_TYPE_GOALITEM,
};

class QNote
{
	friend class QDBPlan;
public:
	QNote(void);
	~QNote(void);

	int ID()const { return m_nID; }
	ENUM_NOTE_TYPE Type()const { return m_eType; }
	int ParentID()const { return m_nParentID; }
	CStdString Extra()const { return m_sExtra; }
	LONG ExtraFlag()const { return m_lExtraFlag; }
	LONG Flag()const { return m_lFlag; }
	CStdString Title()const { return m_sTitle; }
	CStdString Content()const { return m_sContent; }
	CStdString Outline()const { return m_sOutline; }
	CStdString Attachs()const { return m_sAttachs; }
	CStdString Tags()const { return m_sTags; }
	QTime CreationTime()const { return m_tmCreate; }

	CStdString TypeString();
public:
	ENUM_NOTE_TYPE	m_eType;
	int				m_nParentID;
	LONG			m_lExtraFlag;
	CStdString			m_sExtra;
	LONG			m_lFlag;
	CStdString			m_sTitle;
	CStdString			m_sOutline;
	CStdString			m_sContent;
	CStdString			m_sAttachs;
	CStdString			m_sTags;

private:
	int			m_nID;
	QTime		m_tmCreate;
	QTime		m_tmUpdate;
};

typedef std::vector<QNote> VecNote;
