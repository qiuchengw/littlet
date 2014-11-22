#pragma once

#include <vector>
#include <tchar.h>

#ifndef __ATLEXCEPT_H__
#include <atlexcept.h>
#endif

#ifndef __ATLCONV_H__
#include <atlconv.h>
#endif

#include "stdstring.h"
#include "QBuffer.h"
#include "time/QTime.h"

typedef TCHAR wxChar;

template <typename Ty>
class _Typ
{
public:
    _Typ(Ty t = Ty())
        :v_(t)
    {

    }

    Ty GetValue()const
    {
        return v_;
    }

protected:
    Ty  v_;
};

class wxLongLong : public _Typ<LONGLONG>
{
public:
    wxLongLong(LONGLONG ll = 0)
        :_Typ(ll)
    {

    }

    wxLongLong& operator+=(unsigned long l)
    {
       // unsigned long previous = v_;

        v_ += l;

        // 溢出
//         if ((v_ < previous) || (v_ < l))
//             m_hi++;

        return *this;
    }

    wxLongLong& operator*=(unsigned long l)
    {
        // 未处理溢出
        v_ *= l;
    
        return *this;
    }

    wxLongLong& operator-()
    {
        v_ = -v_;

        return *this;
    }

    operator int()const
    {
        return v_;
    }
};

class wxMemoryBuffer : public QBuffer
{
    typedef QBuffer _Base;

public:
    inline BOOL AppendData(void* p, int n)
    {
        return _Base::Write((LPCBYTE)p, n);
    }

    inline void* GetData()const
    {
        return const_cast<wxMemoryBuffer*>(this)->GetBuffer(0);
    }

    inline UINT GetDataLen()const
    {
        return const_cast<wxMemoryBuffer*>(this)->GetBufferLen();
    }

    inline void* GetWriteBuf(__in UINT nPos = 0)
    {
        return _Base::GetBuffer(nPos);
    }

    // Update the length after the write
    void UngetWriteBuf(__in size_t sizeUsed) 
    { 
        _Base::SetBufferLen(sizeUsed);
    }

    void* GetAppendBuf(__in size_t sizeNeed)
    {
        _Base::ReAllocateBuffer(GetBufferLen() + sizeNeed);

        return GetWriteBuf(GetBufferLen());
    }

    void UngetAppendBuf(size_t sizeUsed)
    {

    }

    void Copy(const wxMemoryBuffer& buffer)
    {
        QBuffer::Copy((QBuffer)buffer);
    }

};

class wxCharBuffer : public wxMemoryBuffer
{
public:
    wxCharBuffer()
    {

    }

    wxCharBuffer(const wxCharBuffer& o)
    {
        wxMemoryBuffer::Copy((wxMemoryBuffer) o);
    }

    wxCharBuffer(const char* p)
    {
        ASSERT(nullptr != p);

        wxMemoryBuffer::AppendData((void*)p, strlen(p));
    }

    wxCharBuffer& operator=(const wxCharBuffer& o)
    {
        wxMemoryBuffer::Copy((wxMemoryBuffer) o);

        return *this;
    }

    operator const char*()
    {
        return (const char*)GetBuffer(0);
    }

};

template<typename CT>
class wxStringT : public CStdStr<CT>
{
    typedef CStdStr<CT> _Base;
    typedef wxStringT<CT> MYTYPE;	 // myself

public:
    wxStringT()
    {
    }

    wxStringT(const MYTYPE& str) : _Base(str)
    {
    }

    wxStringT(const std::string& str) : _Base(str)
    {
    }

    wxStringT(const std::wstring& str) : _Base(str)
    {
    }

    wxStringT(PCWSTR str) : _Base(str)
    {

    }

    MYTYPE& operator=(const MYTYPE& str)
    { 
        _Base::operator =(str);

        return *this;
    }

    MYTYPE& operator=(const std::string& str)
    {
        _Base::operator =(str);

        return *this;
    }

    MYTYPE& operator=(const std::wstring& str)
    {
        _Base::operator =(str);

        return *this;
    }

    MYTYPE& operator=(PCSTR pA)
    {
        _Base::operator =(str);

        return *this;
    }

    MYTYPE& operator=(PCWSTR pW)
    {
        _Base::operator =(pW);

        return *this;
    }

    MYTYPE& operator<<(const CT* pW)
    {
        _Base::append(pW);

        return *this;
    }

    inline int Length()const
    {
        return _Base::GetLength();
    }

    inline int Len()const
    {
        return _Base::GetLength();
    }

    void ToLong(__in long* v)
    {
        *v = _wtol(c_str());
    }

    static wxStringT FromUTF8(const char * utf8, int len = -1)
    {

#ifndef _UNICODE
        ASSERT(FALSE);
        return "";
#else
        ATL::CA2WEX<128> c(utf8, CP_UTF8);
        return c.m_psz;
#endif
    }

    wxCharBuffer ToUTF8()const
    {
        ATL::CW2AEX<128> c(c_str(), CP_UTF8);
        return c.m_psz;
    }

    inline int IsSameAs(const CT* p)const
    {
        return _Base::Compare(p);
    }

    operator LPCWSTR()const
    {
#ifndef _UNICODE
        ASSERT(FALSE);
#endif
        return c_str();
    }

    operator LPCSTR()const
    {
#ifndef _UNICODE
        ASSERT(FALSE);
        return nullptr;
#else
        ATL::CW2AEX<128> c(c_str());
        return c.m_psz;
#endif
    }
};


typedef wxStringT<wxChar> wxString;

class wxDateTime : public QTime
{
public:
    wxDateTime()
    {

    }

    wxDateTime(wxLongLong ll)
        :QTime(ll.GetValue())
    {

    }

    wxDateTime(time_t t)
        :QTime(t)
    {

    }

    wxDateTime(double t)
        :QTime(t)
    {

    }

    wxLongLong GetTicks() const
    {
        ASSERT(FALSE);
        return wxLongLong();
    }

    void SetMillisecond(DWORD dwMilsec)
    {
        ASSERT(dwMilsec <= 1000);

        SYSTEMTIME sy;
        GetAsSystemTime(sy);
        sy.wMilliseconds = dwMilsec;

        this->QTime::operator=(sy);
    }

    inline BOOL IsValid()const
    {
        return (QTime::valid) == m_status;
    }

    const wxChar* ParseDate(LPCWSTR s)
    {
        if (QTime::ParseDateTime(s, VAR_DATEVALUEONLY))
        {
            // 嘿嘿…… 绝对不要使用这个返回值，紧紧判断是否是nullptr
            return &ch_;
        }
        return nullptr;
    }

    const wxChar* ParseTime(LPCWSTR s)
    {
        if (QTime::ParseDateTime(s, VAR_TIMEVALUEONLY))
        {
            return &ch_;
        }
        return nullptr;
    }

    const wxChar* ParseDateTime(LPCWSTR s)
    {
        if (QTime::ParseDateTime(s))
        {
            return &ch_;
        }
        return nullptr;
    }

    wxString FormatISOTime()const
    {
        //     %X   Time representation for current locale
        return const_cast<wxDateTime*>(this)->QTime::Format(L"%X").c_str();
    }

    wxString FormatISODate()const
    {
        //    %x Date representation for current locale 
        return const_cast<wxDateTime*>(this)->QTime::Format(L"%x").c_str();
    }

    double GetValue()const
    {
        return const_cast<wxDateTime*>(this)->QTime::SQLDateTime();
    }

    int GetJulianDayNumber()const
    {
        ASSERT(FALSE);
        return -1;
    }

private:
    static const wxChar ch_ = 0;
};

template <typename T>
class vectorT : public std::vector<T>
{
public:
    inline void Add(const T& t)
    {
        push_back(t);
    }

    inline void Empty()
    {
        clear();
    }

    inline const_reference Item(__in size_type i)const
    {
        return at(i);
    }

    inline reference Item(__in size_type i)
    {
        return at(i);
    }

    inline size_t GetCount()const
    {
        return size();
    }

    inline size_t Count()const
    {
        return size();
    }
};

typedef vectorT<wxString> wxArrayString;
typedef vectorT<int> wxArrayInt;


const wxString wxEmptyString = _T("");

#define WXDLLIMPEXP_FWD_SQLITE3
#define WXUNUSED(x) x
#define wxUnusedVar(x) x
#define wxT(x) _T(x)
#define wxTRANSLATE wxT
#define wxASSERT ASSERT

const wxDateTime wxInvalidDateTime = wxDateTime(0ll);

/////////////////////////////////////////////////////////////////////////////
//
// TCHAR based sqlite3 function names for Unicode/MCBS builds.
//
/*
#if defined(_UNICODE) || defined(UNICODE)
#pragma message("Unicode Selected")
#define _sqlite3_aggregate_context      sqlite3_aggregate_context
#define _sqlite3_aggregate_count        sqlite3_aggregate_count
#define _sqlite3_bind_blob              sqlite3_bind_blob
#define _sqlite3_bind_double            sqlite3_bind_double
#define _sqlite3_bind_int               sqlite3_bind_int
#define _sqlite3_bind_int64             sqlite3_bind_int64
#define _sqlite3_bind_null              sqlite3_bind_null
#define _sqlite3_bind_parameter_count   sqlite3_bind_parameter_count
#define _sqlite3_bind_parameter_index   sqlite3_bind_parameter_index
#define _sqlite3_bind_parameter_name    sqlite3_bind_parameter_name
#define _sqlite3_bind_text              sqlite3_bind_text16
#define _sqlite3_bind_text16            sqlite3_bind_text16
#define _sqlite3_busy_handler           sqlite3_busy_handler
#define _sqlite3_busy_timeout           sqlite3_busy_timeout
#define _sqlite3_changes                sqlite3_changes
#define _sqlite3_close                  sqlite3_close
#define _sqlite3_collation_needed       sqlite3_collation_needed16
#define _sqlite3_collation_needed16     sqlite3_collation_needed16
#define _sqlite3_column_blob            sqlite3_column_blob
#define _sqlite3_column_bytes           sqlite3_column_bytes16
#define _sqlite3_column_bytes16         sqlite3_column_bytes16
#define _sqlite3_column_count           sqlite3_column_count
#define _sqlite3_column_decltype        sqlite3_column_decltype16
#define _sqlite3_column_decltype16      sqlite3_column_decltype16
#define _sqlite3_column_double          sqlite3_column_double
#define _sqlite3_column_int             sqlite3_column_int
#define _sqlite3_column_int64           sqlite3_column_int64
#define _sqlite3_column_name            sqlite3_column_name16
#define _sqlite3_column_name16          sqlite3_column_name16
#define _sqlite3_column_text            sqlite3_column_text16
#define _sqlite3_column_text16          sqlite3_column_text16
#define _sqlite3_column_type            sqlite3_column_type
#define _sqlite3_commit_hook            sqlite3_commit_hook
#define _sqlite3_complete               sqlite3_complete16
#define _sqlite3_complete16             sqlite3_complete16
#define _sqlite3_create_collation       sqlite3_create_collation16
#define _sqlite3_create_collation16     sqlite3_create_collation16
#define _sqlite3_create_function        sqlite3_create_function16
#define _sqlite3_create_function16      sqlite3_create_function16
#define _sqlite3_data_count             sqlite3_data_count
#define _sqlite3_errcode                sqlite3_errcode
#define _sqlite3_errmsg                 sqlite3_errmsg16
#define _sqlite3_errmsg16               sqlite3_errmsg16
#define _sqlite3_exec                   sqlite3_exec
#define _sqlite3_finalize               sqlite3_finalize
#define _sqlite3_free                   sqlite3_free
#define _sqlite3_free_table             sqlite3_free_table
#define _sqlite3_get_table              sqlite3_get_table
#define _sqlite3_interrupt              sqlite3_interrupt
#define _sqlite3_last_insert_rowid      sqlite3_last_insert_rowid
#define _sqlite3_libversion             sqlite3_libversion
#define _sqlite3_mprintf                sqlite3_mprintf
#define _sqlite3_open                   sqlite3_open16
#define _sqlite3_open16                 sqlite3_open16
#define _sqlite3_prepare                sqlite3_prepare16
#define _sqlite3_prepare16              sqlite3_prepare16
#define _sqlite3_progress_handler       sqlite3_progress_handler
#define _sqlite3_reset                  sqlite3_reset
#define _sqlite3_result_blob            sqlite3_result_blob
#define _sqlite3_result_double          sqlite3_result_double
#define _sqlite3_result_error           sqlite3_result_error16
#define _sqlite3_result_error16         sqlite3_result_error16
#define _sqlite3_result_int             sqlite3_result_int
#define _sqlite3_result_int64           sqlite3_result_int64
#define _sqlite3_result_null            sqlite3_result_null
#define _sqlite3_result_text            sqlite3_result_text16
#define _sqlite3_result_text16          sqlite3_result_text16
#define _sqlite3_result_text16be        sqlite3_result_text16be
#define _sqlite3_result_text16le        sqlite3_result_text16le
#define _sqlite3_result_value           sqlite3_result_value
#define _sqlite3_set_authorizer         sqlite3_set_authorizer
#define _sqlite3_step                   sqlite3_step
#define _sqlite3_total_changes          sqlite3_total_changes
#define _sqlite3_trace                  sqlite3_trace
#define _sqlite3_user_data              sqlite3_user_data
#define _sqlite3_value_blob             sqlite3_value_blob
#define _sqlite3_value_bytes            sqlite3_value_bytes16
#define _sqlite3_value_bytes16          sqlite3_value_bytes16
#define _sqlite3_value_double           sqlite3_value_double
#define _sqlite3_value_int              sqlite3_value_int
#define _sqlite3_value_int64            sqlite3_value_int64
#define _sqlite3_value_text             sqlite3_value_text16
#define _sqlite3_value_text16           sqlite3_value_text16
#define _sqlite3_value_text16be         sqlite3_value_text16be
#define _sqlite3_value_text16le         sqlite3_value_text16le
#define _sqlite3_value_type             sqlite3_value_type
#define _sqlite3_vmprintf               sqlite3_vmprintf
#else
#pragma message("MCBS Selected")
#define _sqlite3_aggregate_context      sqlite3_aggregate_context
#define _sqlite3_aggregate_count        sqlite3_aggregate_count
#define _sqlite3_bind_blob              sqlite3_bind_blob
#define _sqlite3_bind_double            sqlite3_bind_double
#define _sqlite3_bind_int               sqlite3_bind_int
#define _sqlite3_bind_int64             sqlite3_bind_int64
#define _sqlite3_bind_null              sqlite3_bind_null
#define _sqlite3_bind_parameter_count   sqlite3_bind_parameter_count
#define _sqlite3_bind_parameter_index   sqlite3_bind_parameter_index
#define _sqlite3_bind_parameter_name    sqlite3_bind_parameter_name
#define _sqlite3_bind_text              sqlite3_bind_text
#define _sqlite3_bind_text16            sqlite3_bind_text16
#define _sqlite3_busy_handler           sqlite3_busy_handler
#define _sqlite3_busy_timeout           sqlite3_busy_timeout
#define _sqlite3_changes                sqlite3_changes
#define _sqlite3_close                  sqlite3_close
#define _sqlite3_collation_needed       sqlite3_collation_needed
#define _sqlite3_collation_needed16     sqlite3_collation_needed16
#define _sqlite3_column_blob            sqlite3_column_blob
#define _sqlite3_column_bytes           sqlite3_column_bytes
#define _sqlite3_column_bytes16         sqlite3_column_bytes16
#define _sqlite3_column_count           sqlite3_column_count
#define _sqlite3_column_decltype        sqlite3_column_decltype
#define _sqlite3_column_decltype16      sqlite3_column_decltype16
#define _sqlite3_column_double          sqlite3_column_double
#define _sqlite3_column_int             sqlite3_column_int
#define _sqlite3_column_int64           sqlite3_column_int64
#define _sqlite3_column_name            sqlite3_column_name
#define _sqlite3_column_name16          sqlite3_column_name16
#define _sqlite3_column_text            sqlite3_column_text
#define _sqlite3_column_text16          sqlite3_column_text16
#define _sqlite3_column_type            sqlite3_column_type
#define _sqlite3_commit_hook            sqlite3_commit_hook
#define _sqlite3_complete               sqlite3_complete
#define _sqlite3_complete16             sqlite3_complete16
#define _sqlite3_create_collation       sqlite3_create_collation
#define _sqlite3_create_collation16     sqlite3_create_collation16
#define _sqlite3_create_function        sqlite3_create_function
#define _sqlite3_create_function16      sqlite3_create_function16
#define _sqlite3_data_count             sqlite3_data_count
#define _sqlite3_errcode                sqlite3_errcode
#define _sqlite3_errmsg                 sqlite3_errmsg
#define _sqlite3_errmsg16               sqlite3_errmsg16
#define _sqlite3_exec                   sqlite3_exec
#define _sqlite3_finalize               sqlite3_finalize
#define _sqlite3_free                   sqlite3_free
#define _sqlite3_free_table             sqlite3_free_table
#define _sqlite3_get_table              sqlite3_get_table
#define _sqlite3_interrupt              sqlite3_interrupt
#define _sqlite3_last_insert_rowid      sqlite3_last_insert_rowid
#define _sqlite3_libversion             sqlite3_libversion
#define _sqlite3_mprintf                sqlite3_mprintf
#define _sqlite3_open                   sqlite3_open
#define _sqlite3_open16                 sqlite3_open16
#define _sqlite3_prepare                sqlite3_prepare
#define _sqlite3_prepare16              sqlite3_prepare16
#define _sqlite3_progress_handler       sqlite3_progress_handler
#define _sqlite3_reset                  sqlite3_reset
#define _sqlite3_result_blob            sqlite3_result_blob
#define _sqlite3_result_double          sqlite3_result_double
#define _sqlite3_result_error           sqlite3_result_error
#define _sqlite3_result_error16         sqlite3_result_error16
#define _sqlite3_result_int             sqlite3_result_int
#define _sqlite3_result_int64           sqlite3_result_int64
#define _sqlite3_result_null            sqlite3_result_null
#define _sqlite3_result_text            sqlite3_result_text
#define _sqlite3_result_text16          sqlite3_result_text16
#define _sqlite3_result_text16be        sqlite3_result_text16be
#define _sqlite3_result_text16le        sqlite3_result_text16le
#define _sqlite3_result_value           sqlite3_result_value
#define _sqlite3_set_authorizer         sqlite3_set_authorizer
#define _sqlite3_step                   sqlite3_step
#define _sqlite3_total_changes          sqlite3_total_changes
#define _sqlite3_trace                  sqlite3_trace
#define _sqlite3_user_data              sqlite3_user_data
#define _sqlite3_value_blob             sqlite3_value_blob
#define _sqlite3_value_bytes            sqlite3_value_bytes
#define _sqlite3_value_bytes16          sqlite3_value_bytes16
#define _sqlite3_value_double           sqlite3_value_double
#define _sqlite3_value_int              sqlite3_value_int
#define _sqlite3_value_int64            sqlite3_value_int64
#define _sqlite3_value_text             sqlite3_value_text
#define _sqlite3_value_text16           sqlite3_value_text16
#define _sqlite3_value_text16be         sqlite3_value_text16be
#define _sqlite3_value_text16le         sqlite3_value_text16le
#define _sqlite3_value_type             sqlite3_value_type
#define _sqlite3_vmprintf               sqlite3_vmprintf
#endif
*/