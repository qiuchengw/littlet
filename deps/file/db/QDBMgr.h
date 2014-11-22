#pragma once

#pragma warning(disable:4244 )
#include "BaseType.h"
#include "QBuffer.h"
#include "stdstring.h"
#include "time/QTime.h"

//     typedef QString wxString;
//     typedef LONGLONG wxLongLong;
//     typedef QBuffer wxMemoryBuffer;
//     typedef QTime wxDateTime;
//     typedef std::vector<int> wxArrayInt;
//     typedef std::vector<wxString> wxArrayString;
// 
//     const wxString wxEmptyString = _T("");
// 
// #define WXDLLIMPEXP_FWD_SQLITE3
// #define WXUNUSED(x) x
// #define wxT(x) _T(x)

#include "wx/wxsqlite3.h"

//#include "ui/XMsgBox.h"

#ifdef _DEBUG
#	define TRACE_SQLEXCEPTION(e) {ASSERT(FALSE); MessageBox(NULL,(e).GetMessage(),L"数据库异常",MB_OK);}
//MessageBox(NULL,(e).errorMessage(),L"error",MB_OK);
#else
#	define TRACE_SQLEXCEPTION(e) UNREFERENCED_PARAMETER(e);
#endif

#ifndef QDB_BEGIN_TRY
#	define QDB_BEGIN_TRY try{
#endif

#ifndef QDB_END_TRY
#	define QDB_END_TRY }catch(wxSQLite3Exception& e){TRACE_SQLEXCEPTION(e);}
#endif 

#ifndef INVALID_ID
#define INVALID_ID (-1)
#endif

#ifndef INVALID_NUM
#define INVALID_NUM (0xffffffff)
#endif
inline CStdString StrFiled_HandleSingleQuote(LPCWSTR sFiled)
{
//    mpBuf = sqlite3_mprintf(, va);
    CStdString sRet(sFiled);
    sRet.Replace(L"'",L"''"); 
    return sRet;
}

// 处理插入字符串中存在单引号的问题
#ifndef SFHSQ
#	define SFHSQ(x) (LPCWSTR)StrFiled_HandleSingleQuote(x)
#endif

// 此类不能有任何数据成员
class SqlQuery : public wxSQLite3ResultSet
{
    typedef wxSQLite3ResultSet _Base;
public:
    int IntValue(LPCTSTR pszField) 
    { 
        return GetInt(pszField); 
    }

    int IntValue(int nField) 
    { 
        return GetInt(nField); 
    }

    double DoubleValue(LPCTSTR pszField) 
    { 
        return GetDouble(pszField); 
    }

    double DoubleValue(int nField) 
    { 
        return GetDouble(nField); 
    }

    double DateTimeValue(LPCTSTR pszField) 
    { 
        return DoubleValue(pszField); 
    }

    CStdString StrValue(LPCTSTR pszField) 
    { 
        return (LPCWSTR)GetString(pszField); 
    }

    BOOL BlobValue(LPCTSTR pszField,__out QBuffer& buf)
    {
        buf.ClearBuffer();

        int len = 0;
        auto *p = GetBlob(pszField, len);
        if (len > 0)
        {
            return buf.Write(p, len);
        }

        return FALSE;
    }

    inline bool eof()
    {
        return _Base::Eof();
    }

    // 遍历应该使用这种形式：
//     q1 = ExecuteQuery(...)
//     while (q1.NextRow())
//     {
//         cout << (const char*)(q1.GetString(0).mb_str()) << endl;
//     }

    inline bool nextRow()
    {
        return _Base::NextRow();
    }
};

class SqlDB : public wxSQLite3Database
{
public:
	inline SqlQuery ExecQuery(__in LPCTSTR sql) 
    { 
        return *((SqlQuery*)&(wxSQLite3Database::ExecuteQuery(sql)));
    }

	inline BOOL SetPassword(const std::wstring& sps) 
    { 
        try
        {
            wxSQLite3Database::ReKey(sps);
        }
        catch(wxSQLite3Exception& e)
        {
            UNREFERENCED_PARAMETER(e);

            return FALSE;
        }

        return TRUE;
    }
};

/**
 *        A scalar function is a function that operates on scalar values 
 *            -- that is, it takes one (or more) input values as arguments directly 
 *                and returns a value.
 *            -- Scalar函数基本等同于C/C++函数
 *
 *        An aggregate function is a function that operates on aggregate data 
 *            -- that is, it takes a complete set of data as input 
 *                and returns a value that is computed from all the values in the set.
 *  
 ***    对于需要多次执行的语句，使用 预编译和事务 加快速度
 *     CppSQLiteStatement stmt = db.compileStatement(
 *         "insert into emp values (?, ?);");
 *     for (i = 0; i < nRowsToCreate; i++)
 *     {
 *         char buf[16];
 *         sprintf(buf, "EmpName%06d", i);
 *         stmt.bind(1, i);
 *         stmt.bind(2, buf);
 *         stmt.execDML();
 *         stmt.reset();
 *     }
 * 
 *     db.execDML("commit transaction;");
**/
class IDBAdapter
{
public:
    IDBAdapter(SqlDB *pDB)
    {
        ASSERT(nullptr != pDB);
        pdb_ = pDB;
    }

    inline BOOL IsOpen() const 
    { 
        return pdb_->IsOpen(); 
    }

    inline SqlQuery ExecQuery(LPCWSTR szQuery) 
    { 
        return pdb_->ExecQuery(szQuery); 
    }

    inline void BeginTransaction() 
    { 
        pdb_->Begin();
    }

    inline void ExecTransaction(LPCTSTR sql) 
    {
        pdb_->ExecQuery(sql);
        //m_pDB->execDML(sql); 
    }

    inline void RollbackTransaction() 
    { 
        pdb_->Rollback();
    };

    inline void CommitTransaction() 
    { 
        pdb_->Commit();
    }

    inline int GetLastRowID() 
    { 
        return pdb_->GetLastRowId();
    }

    int _ExecSQL_RetInt(LPCWSTR sSQL) throw()
    {
        try
        {
            return pdb_->ExecQuery(sSQL).IntValue(0);
        }
        catch (wxSQLite3Exception& e)
        {
            TRACE_SQLEXCEPTION(e);
            return INVALID_NUM;
        }
    }

    BOOL _ExecSQL_RetBOOL(LPCWSTR sSQL) throw()
    {
        try
        {
            pdb_->ExecQuery(sSQL);
            return TRUE;
        }
        catch (wxSQLite3Exception& e)
        {
            TRACE_SQLEXCEPTION(e);
            return FALSE;
        }
    }

    BOOL _ExecSQL_RetBOOL(LPCWSTR sSQL,SqlQuery &q) throw()
    {
        try
        {
            q = pdb_->ExecQuery(sSQL);
            return TRUE;
        }
        catch (wxSQLite3Exception& e)
        {
            TRACE_SQLEXCEPTION(e);
            return FALSE;
        }
    }

    int _ExecSQL_RetLastID(LPCWSTR sSQL) throw()
    {
        try
        {
            pdb_->ExecQuery(sSQL);
            return GetLastRowID();
        }
        catch (wxSQLite3Exception& e)
        {
            TRACE_SQLEXCEPTION(e);
            return INVALID_ID;
        }
    }

    // 写入大的二进制数据，并返回其数据项id
    int _ExecSQL_RetLastID(LPCWSTR sSQL,QBuffer& buf) throw()
    {
        try
        {
            wxSQLite3Statement stat = pdb_->PrepareStatement(sSQL);
            //将文件数据绑定到insert语句中，替换“？”部分
            // You can use sqlite3_bind_blob to pass arbitrary 
            // binary data to an SQL statement without worrying about special quoting. 
            stat.Bind(1, buf.GetBuffer(), buf.GetBufferLen());
            //执行绑定之后的SQL语句
            stat.ExecuteQuery();

            return GetLastRowID();
        }
        catch (wxSQLite3Exception&e)
        {
            TRACE_SQLEXCEPTION(e);
        }
        return INVALID_ID;
    }

    inline SqlDB *GetDB()
    {
        return pdb_;
    }

protected:
    SqlDB*		pdb_;
};

class DBMan : public IDBAdapter
{
public:
    DBMan() : IDBAdapter(&db_)
    {

    }
        
	virtual ~DBMan()
    {
        Close();
    }

	inline BOOL Open(__in LPCWSTR szDB, __in LPCWSTR szPswd = NULL)
    {
        try
        {
            if (IsOpen())
            {
                db_.Close();
            }

            db_.Open(szDB, szPswd, WXSQLITE_OPEN_READWRITE);
        }
        catch (wxSQLite3Exception& e)
        {
            UNREFERENCED_PARAMETER(e);

            ASSERT(FALSE);
        	// e.GetMessage();
            return FALSE;
        }

        return IsOpen();
    }

	inline BOOL IsOpen() const 
    { 
        return db_.IsOpen(); 
    }

	inline void Close() 
    { 
        if (IsOpen())
        {
            db_.Close(); 
        }
    }

    inline IDBAdapter* GetAdapter()
    {
        return dynamic_cast<IDBAdapter*>(this);
    }

protected:
	SqlDB		db_;
};

