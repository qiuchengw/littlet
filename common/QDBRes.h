#pragma once

#include "file/db/QDBMgr.h"
#include "ConstValues.h"
#include "time/QTime.h"
#include "AppHelper.h"

enum ENUM_DBFILE_TYPE
{
	DBFILE_TYPE_NONE = 0x0,
	DBFILE_TYPE_UNKOWN = 0x1,
	DBFILE_TYPE_IMAGE = 0x2,
	DBFILE_TYPE_TEXT = 0x4,
	DBFILE_TYPE_AUDIO = 0x8,
	DBFILE_TYPE_VIDEO = 0x10,
	DBFILE_TYPE_EXE = 0x20,
    DBFILE_TYPE_THUMBICON = 0x40,
	DBFILE_TYPE_ALL = 0xFF,
};

struct TFileData
{
	TFileData& operator=(const TFileData&o)
	{
		nID = o.nID;
		nDataSize = o.nDataSize;
//		bufData.Copy(o.bufData);
		nRefs = o.nRefs;
		sResName = o.sResName;
		sExt = o.sExt;
		tmCreate = o.tmCreate;
		tmUpdate = o.tmUpdate;
		eType = o.eType;

		return *this;
	}

	int			nID;
	QBuffer		bufData;
	DWORD		nDataSize;
	LONG		nRefs;		// 引用计数
	ENUM_DBFILE_TYPE	eType;
	QString		sResName;
	QString		sExt;		// 无 '.'
	QTime		tmCreate;
	QTime		tmUpdate;
};

typedef std::vector<TFileData> VecFileData;
typedef VecFileData::iterator VecFileDataItr;

class QResMan;
class QResDB : public DBMan
{
    friend class QResMan;

	SINGLETON_ON_DESTRUCTOR(QResDB){}

private:
	BOOL Startup();

	// 对 tbl_file 操作
	BOOL AddData( __inout TFileData &fd );
	BOOL DeleteData(int nID);
    BOOL GetData( int nID, BOOL bWithData, __out TFileData& fd );
    BOOL AddDataRef(int nDataID);
	BOOL ReleaseDataRef(int nDataID);
	int GetDataRef(int nDataID);
	int GetDataCount();

	/** 填充出Data以外的字段（ID，size，name...)
	 *	-return:	
     *      数据项目个数
	 *	-params:	
	 *		-[in]	eType 要获取的字段数据类型
	 *		-[out]	vfd
	 **/
	int GetDataItems( __out VecFileData& vfd ,DWORD eType = DBFILE_TYPE_ALL);

	// bIncludeData 包括BLOB数据
	int GetDataItems(__in IntArray& vi,__out VecFileData& vfd,BOOL bIncludeData=FALSE);
};

// QResDB 管理器
class QResMan
{
    SINGLETON_ON_DESTRUCTOR(QResMan)
    {

    }

public:
    BOOL Startup(QString sIconDir);

    /** 根据icon在数据库中的ID获取其路径
     *	return
     *      icon被释放到的路径,
     *	param
     *		-[in]
     *          tfd 
    **/
    QString GetDBResFilePath(__in TFileData& tfd);
    QString GetDBResFilePath(__in int nID);

    /** 将数据释放到szFile
     *	return
     *      TRUE    搞定
     *	param
     *		-[in]
     *          nID     数据ID
     *          szFile  文件路径，必须为全路径
    **/
    BOOL ReleaseDataToFile( __in int nID, __in LPCWSTR szFile );

    /**自动检测数据，根据数据类型释放到特定的目录
     *	return
     *      释放到的文件路径，失败返回空
     *	param
     *		-[in]
     *          nID     数据ID
    **/
    QString ReleaseDataToFile(__in int nID );

    int GetDataItems( __out VecFileData& vfd ,DWORD eType = DBFILE_TYPE_ALL);

    // bIncludeData 包括BLOB数据
    int GetDataItems(__in IntArray& vi,__out VecFileData& vfd,BOOL bIncludeData=FALSE);

    // 对 tbl_file 操作
    BOOL AddData( __inout TFileData &fd );
    BOOL DeleteData(int nID);
    BOOL GetData( int nID, BOOL bWithData, __out TFileData& fd );
    BOOL AddDataRef(int nDataID);
    BOOL ReleaseDataRef(int nDataID);
    int GetDataRef(int nDataID);
    int GetDataCount();

private:
    IntArray    m_vResIcons;    // 已经释放到cache中的icon的id会被记录这儿
    QString     m_sIconDir;
};


