#include "QNote.h"


QNote::QNote(void)
{
}


QNote::~QNote(void)
{
}

QString QNote::TypeString()
{
	QString sRet;
	switch (Type())
	{
	case NOTE_TYPE_NOTSET:
		{
			sRet = L"非关联笔记";
			return sRet;
		}
	case NOTE_TYPE_GOALITEM:
		{
			sRet = L"计划目标项";
			break;
		}
	}
	QString sID;
	sID.Format(L"[%d]",ParentID());
	sRet += sID;
	return sRet;
}
