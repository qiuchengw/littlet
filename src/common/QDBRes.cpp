#include "QDBRes.h"
#include "AppHelper.h"
#include "crypt/zlibdata.h"

//////////////////////////////////////////////////////////////////////////
BOOL QResDB::Startup()
{
	return Open(L"res.db");
}

BOOL QResDB::AddData( __inout TFileData &fd  )
{
    fd.nDataSize = fd.bufData.GetBufferLen();
	ASSERT(fd.nDataSize > 0);
	// 对图像数据进行压缩后再存储
	QBuffer buf;
	if (ZlibData::CompressData(fd.bufData, buf))
	{
		CStdString sQ;
		sQ.Format(L"INSERT INTO tbl_data(DataType,DataSize,Refs,Name,Ext,Datax,crtime,udtime)"
			L"  VALUES(%d,%d,0,'%s','%s',?,%lf,%lf)",
			fd.eType, fd.nDataSize, SFHSQ(fd.sResName), SFHSQ(fd.sExt),
			QTime::SQLDateTimeCurrent(),QTime::SQLDateTimeCurrent());
		fd.nID = _ExecSQL_RetLastID(sQ, buf);
		return (INVALID_ID != fd.nID);
	}
	return FALSE;
}

BOOL QResDB::GetData( int nID, BOOL bWithData, __out TFileData& fd )
{
	QDB_BEGIN_TRY
		CStdString sQ;
        if (bWithData)
        {
            sQ.Format(L"SELECT * FROM tbl_data WHERE (ID=%d)",nID);
        }
        else
        {
            sQ.Format(L"SELECT DataType,DataSize,Refs,Name,Ext,crtime,udtime"
                      L" FROM tbl_data WHERE (ID=%d)",nID);
        }
		SqlQuery q = ExecQuery(sQ);
		if (!q.eof())
		{
			fd.nID = nID;
			fd.eType = (ENUM_DBFILE_TYPE)q.IntValue(L"DataType");
			fd.nDataSize = q.IntValue(L"DataSize");
			fd.nRefs = q.IntValue(L"Refs");
			fd.sResName = q.StrValue(L"Name");
			fd.sExt = q.StrValue(L"Ext");
			fd.tmCreate = q.DateTimeValue(L"crtime");
			fd.tmUpdate = q.DateTimeValue(L"udtime");

            if (bWithData)
            {
			    // buf
			    if (!q.BlobValue(L"Datax",fd.bufData))
				    return FALSE;
			    return ZlibData::DecompressData(fd.bufData,fd.nDataSize);
            }
            return TRUE;
		}
	QDB_END_TRY
		return FALSE;
}

BOOL QResDB::AddDataRef( int nDataID )
{
	int nRef = GetDataRef(nDataID);
	nRef++;
	CStdString sQ;
	sQ.Format(L"UPDATE tbl_data SET Refs=%d WHERE (ID=%d)",nRef,nDataID);
	return _ExecSQL_RetBOOL(sQ);
}

BOOL QResDB::ReleaseDataRef( int nDataID )
{
	int nRef = GetDataRef(nDataID);
	nRef--;
	CStdString sQ;
	sQ.Format(L"UPDATE tbl_data SET Refs=%d WHERE (ID=%d)",nRef,nDataID);
	return _ExecSQL_RetBOOL(sQ);
}

int QResDB::GetDataRef( int nDataID )
{
	CStdString sQ;
	sQ.Format(L"SELECT Refs FROM tbl_data WHERE (ID=%d)",nDataID);
	return _ExecSQL_RetInt(sQ);
}

int QResDB::GetDataCount()
{
	return _ExecSQL_RetInt(L"SELECT COUNT(ID) FROM tbl_data");
}

int QResDB::GetDataItems( __out VecFileData& vfd ,DWORD eType)
{
	CStdString sCondtion;
	if (DBFILE_TYPE_NONE == eType)
		return 0;
	if (DBFILE_TYPE_ALL == eType)
		sCondtion = L"";
	else
	{
		CStdString sSub;
		for (int i = 0; i < sizeof(int)*8; i++)
		{
			if ( (0x1<<i) & eType )
			{
				sSub.Format( L"(DataType=%d)", 0x1<<i );
				sCondtion += L" OR ";
				sCondtion += sSub;
			}
		}
		sCondtion = sCondtion.Mid(4);
	}
	CStdString sQ = L"SELECT ID,DataType,DataSize,Name,Ext,Refs FROM tbl_data";
	if (!sCondtion.IsEmpty())
    {
        sQ += L" WHERE " + sCondtion;
    }

	QDB_BEGIN_TRY
		SqlQuery q = ExecQuery(sQ);
		for (q.nextRow(); !q.eof(); q.nextRow())
		{
			TFileData t;
			t.nID = q.IntValue(L"ID");
			t.eType = (ENUM_DBFILE_TYPE)q.IntValue(L"DataType");
			t.nDataSize = q.IntValue(L"DataSize");
			t.sResName = q.StrValue(L"Name");
			t.sExt = q.StrValue(L"Ext");
			t.nRefs = q.IntValue(L"Refs");

			vfd.push_back(t);
		}
	QDB_END_TRY
		return vfd.size();
}

BOOL QResDB::DeleteData( int nID )
{
	CStdString sQ;
	sQ.Format(L"DELETE FROM tbl_data WHERE (ID=%d)",nID);
	return _ExecSQL_RetBOOL(sQ);
}

int QResDB::GetDataItems( __in IntArray& vi,__out VecFileData& vfd,BOOL bIncludeData/*=FALSE*/ )
{
	for (unsigned i = 0; i < vi.size(); i++)
	{
		TFileData t;
		if (GetData(vi[i], bIncludeData, t))
		{
			if (!bIncludeData)
				t.bufData.ClearBuffer();
			vfd.push_back(t);
		}
	}
	return vfd.size();
}

//////////////////////////////////////////////////////////////////////////
BOOL QResMan::Startup( CStdString sIconDir )
{
    QResDB* pDB = QResDB::GetInstance();
    if (!pDB->Startup())
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_sIconDir = sIconDir;

    // 释放所有的icon到cache中
    VecFileData vfd;
    pDB->GetDataItems(vfd, DBFILE_TYPE_THUMBICON);
    for (VecFileData::iterator i = vfd.begin(); i != vfd.end(); ++i)
    {
        ReleaseDataToFile((*i).nID, GetDBResFilePath(*i));
    }

    return TRUE;
}

CStdString QResMan::GetDBResFilePath( __in TFileData& tfd )
{
    CStdString sRet;
    switch (tfd.eType)
    {
    case DBFILE_TYPE_NONE:// = 0x0
    case DBFILE_TYPE_ALL:// = 0xFF,
        {
            ASSERT(FALSE);
            return L"";
        }
    case DBFILE_TYPE_IMAGE:// = 0x2,
    case DBFILE_TYPE_TEXT:// = 0x4,
    case DBFILE_TYPE_AUDIO:// = 0x8,
    case DBFILE_TYPE_VIDEO://  = 0x10,
    case DBFILE_TYPE_EXE:// = 0x20,
    case DBFILE_TYPE_UNKOWN:// = 0x1,
        {
            ASSERT(FALSE);  // 未实现
            break;
        }
    case DBFILE_TYPE_THUMBICON:// = 0x40,
        {
            sRet.Format(L"%s_icon_%d.%s",
                m_sIconDir,
                tfd.nID, tfd.sExt);
            break; 
        }
    }
    return sRet;
}

CStdString QResMan::GetDBResFilePath( __in int nID )
{
    if (INVALID_ID != nID)
    {
        TFileData tfd;
        if ( QResDB::GetInstance()->GetData(nID,FALSE, tfd))
        {
            return GetDBResFilePath(tfd);
        }
    }
    return m_sIconDir + L"default.png";
}

BOOL QResMan::ReleaseDataToFile( int nID,LPCWSTR szFile )
{
    if (quibase::IsFileExist(szFile))
        return TRUE;

    TFileData t;
    if (QResDB::GetInstance()->GetData(nID, TRUE, t))
    {
        return t.bufData.FileWrite(szFile);
    }
    return FALSE;
}

CStdString QResMan::ReleaseDataToFile( int nID )
{
	TFileData t;
	if (QResDB::GetInstance()->GetData(nID,TRUE, t))
    {
        CStdString sRet = GetDBResFilePath(t);
        if (!quibase::IsFileExist(sRet))
        {
            t.bufData.FileWrite(sRet);
            return sRet;
        }
    }
    return L"";
}

int QResMan::GetDataItems( __out VecFileData& vfd ,DWORD eType /*= DBFILE_TYPE_ALL*/ )
{
    return QResDB::GetInstance()->GetDataItems(vfd, eType);
}

int QResMan::GetDataItems( __in IntArray& vi,__out VecFileData& vfd,BOOL bIncludeData/*=FALSE*/ )
{
    return QResDB::GetInstance()->GetDataItems(vi, vfd, bIncludeData);
}

BOOL QResMan::AddData( __inout TFileData &fd )
{
    return QResDB::GetInstance()->AddData(fd);
}

BOOL QResMan::DeleteData( int nID )
{
    return QResDB::GetInstance()->DeleteData(nID);
}

BOOL QResMan::GetData( int nID, BOOL bWithData, __out TFileData& fd )
{
    return QResDB::GetInstance()->GetData(nID, bWithData, fd);
}

BOOL QResMan::AddDataRef( int nDataID )
{
    return QResDB::GetInstance()->AddDataRef(nDataID);
}

BOOL QResMan::ReleaseDataRef( int nDataID )
{
    return QResDB::GetInstance()->ReleaseDataRef(nDataID);
}

int QResMan::GetDataRef( int nDataID )
{
    return QResDB::GetInstance()->GetDataRef(nDataID);
}

int QResMan::GetDataCount()
{
    return QResDB::GetInstance()->GetDataCount();
}

