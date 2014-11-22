#pragma once

#include <Windows.h>
#include <vector>
#include <ShlObj.h>
#include <comdef.h> // for _bstr_t extractor only, can be removed if you don't need CXShellPath::GetBStr()
#include <shlwapi.h>
#include "time/QTime.h"

/// The nsPath namespace contains the CXShellPath class and global helper functions.
namespace quibase
{

/// stateless path methods, path can be greater than MAX_PATH
class  CWinPathApi
{
public:

    /// constant
    enum EM_WIN_PATH_API_CONST
    {
        WIN_PATH_MAX_UNICODE_PATH           = 32767 + 4,    ///< max size of buffer to store path, in bytes
        WIN_PATH_UNICODE_PATH_PREFIX        = 4,            ///< length of "\\\\??\\"
        WIN_PATH_UNICODE_UNC_PATH_PREFIX    = 7,            ///< length of "\\\\??\\UNC\\"
    };

    // return S_FALSE, 目录已存在
    // return S_OK, 目录被新创建
    // return E_XX, 失败
    static HRESULT CreateFullPath(LPCWSTR lpszFileName, DWORD dwFileAttribute = FILE_ATTRIBUTE_NORMAL);

    /// see Win32 API GetFullPathName
    static HRESULT  ExpandFullPathName(CStdString& strFullPathName);

    /// see Win32 API GetLongPathName
    static HRESULT  ExpandLongPathName(CStdString& strFullPathName);


    /// replace 'lpPattern' at begging of 'strPath' with 'csidl'
    static BOOL     ExpandSpecialFolderPathAtBeginning(LPWSTR lpszPath, DWORD cchBuf, LPCWSTR lpszPattern, int csidl);
    static BOOL     ExpandSpecialFolderPathAtBeginning(CStdString& strPath, LPCWSTR lpszPattern, int csidl);

    /// replace 'lpPattern' at begging of 'strPath' with 'lpExpandAs'
    static BOOL     ExpandPatternAtBeginning(LPWSTR lpszPath, DWORD cchBuf, LPCWSTR lpszPattern, LPCWSTR lpszExpandAs);
    static BOOL     ExpandPatternAtBeginning(CStdString& strPath, LPCWSTR lpszPattern, LPCWSTR lpszExpandAs);

    /// see Win32 API ExpandEnvironmentStrings
    static BOOL     ExpandEnvironmentStrings(LPCWSTR lpszSrc, LPWSTR lpszDest, DWORD cchBuf);
    static BOOL     ExpandEnvironmentStrings(CStdString& strPath);


    /// replace special string in unaccessible path, such as '%SYSTEM%' '\??\' '\\SystemRoot'
    static BOOL     ExpandAsAccessiblePath(LPWSTR lpszPath, DWORD cchBuf);
    static BOOL     ExpandAsAccessiblePath(CStdString& strPath);


    /// check if path begin with "\\\\??\\" or "\\\\??\\UNC\\"
    static BOOL     HasUnicodePrefix(LPCWSTR pszPath);

    // for non-unicode unc path, it returns the position right after first "\\\\"
    // for unicode path, it returns the position right after "\\\\?\\" or "\\\\?\\UNC\\"
    // for other case, it returns the whole string
    static LPCWSTR  FindAfterAnyPrefix(LPCWSTR pszPath);

    // for non-unicode unc path, it returns the position right after first "\\\\"
    // for unicode path, it returns the position right after "\\\\?\\"
    // for other case, it returns the whole string
    static LPCWSTR  FindAfterUnicodePrefix(LPCWSTR pszPath);

    static LPCWSTR  FindFileName(LPCWSTR pszPath);

    static LPCWSTR  FindExtension(LPCWSTR pszPath);

    /// create short cut
    static HRESULT  CreateLnkFile(LPCWSTR pszPath, LPCWSTR pszArguments, LPCWSTR pszDesc, LPCWSTR pszLnkFilePath);
    /// resolve short cut
    static HRESULT  ResolveLnkFileNoSafe(LPCWSTR pszLnkFile, CStdString& strTargetPath, DWORD dwFlag = SLR_NOUPDATE | SLR_NOTRACK | SLR_NOSEARCH | SLR_NO_UI);
    /// resolve short cut
    static HRESULT  ResolveLnkFile(LPCWSTR pszLnkFile, CStdString& strTargetPath, DWORD dwFlag = SLR_NOUPDATE | SLR_NOTRACK | SLR_NOSEARCH | SLR_NO_UI);

    /// path is "." or ".."
    static BOOL     IsDots(LPCWSTR pszPath);
    static BOOL     IsDirectory(LPCWSTR pszPath);
    static BOOL     IsRelative(LPCWSTR pszPath);
    static BOOL     IsRoot(LPCWSTR pszPath);
    static BOOL     IsUNC(LPCWSTR pszPath);
    static BOOL     IsUNCServer(LPCWSTR pszPath);
    static BOOL     IsUNCServerShare(LPCWSTR pszPath);
    static BOOL     IsFileExisting(LPCWSTR pszPath);

    static BOOL     IsFileNoDirExisting(LPCWSTR pszPath);
    static BOOL     IsLnkFile(LPCWSTR pszPath);
};


/// path class
class  CWinPath
{
    // Constructors
public:
    CWinPath();
    CWinPath(const CWinPath& path);
    CWinPath(LPCWSTR pszPath);


    // Operators
public:
    operator LPCWSTR() const;
    CWinPath& operator+=(LPCWSTR pszMore);

    // Operations
public:
    void        AddBackslash();
    BOOL        AddExtension(LPCWSTR pszExtension);
    BOOL        Append(LPCWSTR pszMore);
    void        BuildRoot(int iDrive);
    //void        Canonicalize();
    void        Combine(LPCWSTR pszDir, LPCWSTR pszFile);
    //CWinPath    CommonPrefix(LPCWSTR pszOther );
    //BOOL        CompactPath(HDC hDC, UINT nWidth);
    BOOL        CompactPathEx(UINT nMaxChars, DWORD dwFlags = 0);
    //BOOL        FileExists() const;
    int         FindExtension() const;
    int         FindFileName() const;
    //int         GetDriveNumber() const;
    CStdString     GetExtension() const;
    BOOL        IsDirectory() const;
    //BOOL        IsFileSpec() const;
    //BOOL        IsPrefix(LPCWSTR pszPrefix ) const;
    BOOL        IsRelative() const;
    BOOL        IsRoot() const;
    //BOOL        IsSameRoot(LPCWSTR pszOther) const;
    BOOL        IsUNC() const;
    BOOL        IsUNCServer() const;
    BOOL        IsUNCServerShare() const;
    //BOOL        MakePretty();
    //BOOL        MatchSpec(LPCWSTR pszSpec) const;
    //void        QuoteSpaces();
    //BOOL        RelativePathTo(LPCWSTR pszFrom, DWORD dwAttrFrom, LPCWSTR pszTo, DWORD dwAttrTo );
    void        RemoveArgs();
    void        RemoveBackslash();
    //void        RemoveBlanks();
    void        RemoveExtension();
    BOOL        RemoveFileSpec();
    //BOOL        RenameExtension(LPCWSTR pszExtension);
    //int         SkipRoot() const;
    void        StripPath();
    BOOL        StripToRoot();
    void        UnquoteSpaces();


    // Extra Operation
public:

    BOOL        IsExisting() const;
    void        RemoveSingleArg();

    BOOL        HasUnicodePrefix() const;
    void        RemoveUnicodePrefix();
    void        AddUnicodePrefix();
    CWinPath    GetPathWithoutUnicodePrefix() const;

    HRESULT     ExpandFullPathName();
    HRESULT     ExpandLongPathName();
    void        ExpandEnvironmentStrings();
    void        ExpandNormalizedPathName();
    BOOL        ExpandAsAccessiblePath();

    DWORD       GetModuleFileName(HMODULE hModule, DWORD dwMaxSize = MAX_PATH);
    CStdString m_strPath;
};

inline CWinPath::CWinPath() throw()
{
}

inline CWinPath::CWinPath(const CWinPath& path):
    m_strPath(path.m_strPath)
{
}

inline CWinPath::CWinPath(LPCWSTR pszPath):
    m_strPath(pszPath )
{
}

inline CWinPath::operator LPCWSTR() const throw()
{
    return m_strPath;
}

inline CWinPath& CWinPath::operator+=(LPCWSTR pszMore)
{
    Append( pszMore );

    return *this;
}

class CWinPathAttr // dummyz@126.com
{
public:
    CWinPathAttr()
    {
    }

    CWinPathAttr(LPCTSTR lpFilePath)
    {
        m_strFilePath = lpFilePath;
        _Init();
    }

    CWinPathAttr(const CStdString& strFilePath)
    {
        m_strFilePath = strFilePath;
    }

    BOOL	IsChanged()
    {
        WIN32_FILE_ATTRIBUTE_DATA wfad;

        if ( GetFileAttributesEx(m_strFilePath, GetFileExInfoStandard, &wfad) )
        {
            if ( CompareFileTime(&wfad.ftLastWriteTime, &m_wfad.ftLastWriteTime) != 0 )
            {
                memcpy(&m_wfad, &wfad, sizeof (wfad));
                return TRUE;
            }
        }

        return FALSE;
    }

protected:
    void	_Init()
    {
        if ( !GetFileAttributesEx(m_strFilePath, GetFileExInfoStandard, &m_wfad) )
        {
            memset(&m_wfad, 0, sizeof (m_wfad));
        }
    }

protected:
    WIN32_FILE_ATTRIBUTE_DATA m_wfad;
    CStdString		m_strFilePath;
};


//////////////////////////////////////////////////////////////////
// brief	:	2011/03/25
// copyright:	qiuchengw @ 2011
//////////////////////////////////////////////////////////////////
const CStdString	sCEmptyString = _T("");
enum		{PATH_CMDLINE, PATH_MODULE};
enum		{FILE_CREATION, FILE_ACCESS, FILE_WRITE};

class  CPath
{
public:
    CPath();
    CPath(LPCTSTR szPath, BOOL bIsFolderPath = FALSE, BOOL bHasArguments = FALSE);
    CPath(DWORD dwSpecial);
    virtual ~CPath();

    // Parses a path or PATH_CMDLINE, PATH_MODULE
    void	SetPath(LPCTSTR szPath, BOOL bIsFolderPath = FALSE, BOOL bHasArguments = FALSE);
    void	SetPath(DWORD dwSpecial);
    CStdString	GetPath(BOOL bAppendArgs = FALSE, BOOL bOriginal = FALSE);
    CStdString	GetShortPath();
    CStdString	GetLongPath();

    BOOL IsLocalPath();
    BOOL IsRelativePath();
    BOOL IsFilePath();

    BOOL ExistFile();
    BOOL ExistLocation();


    // If the path set in the object is not a relative one returns empty
    CStdString	GetAbsolutePath(LPCTSTR szBaseFolder);

    // If the path set in the object is a relative one returns empty
    CStdString	GetRelativePath(LPCTSTR szBaseFolder);


    // Get drive string (empty for a network path) [e.g.: "c:"]
    CStdString	GetDrive();

    // Get drive label (pc name for a network path) [e.g.: "MAIN_HD"]
    CStdString	GetDriveLabel(BOOL bPCNameIfNetwork = FALSE);

    // Get folder count in path [e.g.: 2 for "c:\folder\subfolder\file.ext"]
    int		GetDirCount();

    // Get 0 based nIndex folder string [e.g.: "folder" for nIndex = 0]
    // If nIndex = -1 the return string is the full dir string
    // [e.g.: "\folder\subfolder\" or "\\pcname\folder\" for non-local]
    // If it's a relative path no "\" is added at the beginning [e.g.: "..\sub\"]
    CStdString	GetDir(int nIndex = -1);

    // File location or directory path [e.g.: "c:\folder\subfolder\"]
    CStdString	GetLocation();

    // File title string (without extension) [e.g.: "file" for "..\file.ext"]
    CStdString	GetFileTitlex();

    // Filename = File title + extension [e.g.: "file.ext"]
    CStdString	GetFileName();

    // Extension string (dot included) [e.g.: ".ext"]
    CStdString	GetExtension();

    // Extension name (dot not included) [e.g.: "ext"]
    CStdString	GetExtName();

    // Get argument count [e.g.: 2 for <c:\app.exe param1 "param 2">]
    int		GetArgCount();

    // Get 0 based nIndex argument string
    // If nIndex = -1 the return string is the argument part of the path
    // if bGetFlag is true, return the 0 based nIndex argument flag
    CStdString	GetArgument(int nIndex = -1, BOOL bGetFlag = FALSE);

    // Set the arguments for the current file path
    void	SetArguments(LPCTSTR szArgs);

    // Add or set an argument
    void	AddSetArgument(LPCTSTR szFlag, LPCTSTR szArgument);

    // Remove argument nIndex
    void	RemoveArgument(int nIndex);

    // Return 0 based index of the argument whose flag matches szFlag
    // If it's not found, the return value is -1
    int		FindArgument(LPCTSTR szFlag);


    // Get the size in bytes of the current file
    BOOL	GetFileSize(__int64 &nSize);

    // Get the size in bytes of the current file
    // values: FILE_CREATION, FILE_ACCESS, FILE_WRITE
    BOOL	GetFileTime(QTime &time, DWORD dwType = FILE_WRITE);


    // Return a temporary character pointer to the path data.
    operator LPCTSTR ();

    // Same as SetPath(szPath, FALSE, FALSE)
    const CPath& operator = (LPCTSTR szPath);

    // Makes a copy of the object
    const CPath& operator = (CPath &ref);

    // Add a back slash ('\' or '/' if bInverted is TRUE) if necessary
    static CStdString AddBackSlash(LPCTSTR szFolderPath, BOOL bInverted = FALSE);

    // Removes a trailing back slash if found
    static CStdString RemoveBackSlash(LPCTSTR szFolderPath);

private:
    void FillDirArray();
    void FillArgArray();
    BOOL FillFileInfoStruct();

    CStdString			_sOriginalPath;

    CStdString			_sDrive;
    CStdString			_sDriveLabel;
    std::vector<CStdString>	_aDir;
    CStdString			_sDir;
    CStdString			_sFileTitle;
    CStdString			_sExtName;

    class CArgument
    {
    public:
        CArgument()
        {
            cFlagMark = '/';
        }
        virtual ~CArgument() {}

        CStdString GetString()
        {
            CStdString sArg;
            if (!sFlag.IsEmpty()) sArg.Format(L" %c%s", cFlagMark, sFlag);
            if (!sValue.IsEmpty())
            {
                if (sValue.Find(L' ') != -1)
                    sArg += CStdString(L" \"") + sValue + CStdString(L"\"");
                else
                    sArg += CStdString(L" ") + sValue;
            }

            return sArg;
        }

        void SetFlag(CStdString sFlagValue)
        {
            sFlag = sFlagValue;
            if (sFlag.Remove(L'/') > 0) cFlagMark = L'/';
            if (sFlag.Remove(L'-') > 0) cFlagMark = L'-';
            sFlag.Remove(L' ');
        }

        char	cFlagMark;
        CStdString	sFlag;
        CStdString	sValue;
    };

    typedef std::vector<CArgument> ATArguments;

    CStdString			_sArgs;
    ATArguments		_aArgs;

    BOOL			_bIsRelative;

    BY_HANDLE_FILE_INFORMATION _fis;

    CStdString			_sLPCTSTRPath;
};


// ==================================================================
//
//  Path.h
//
//  Created:       03.03.2005
//
//  Copyright (C) Peter Hauptmann
//
// ------------------------------------------------------------------
//
/// \page pgDisclaimer Copyright & Disclaimer
///
/// Copyright (C) 2004-2005 Peter Hauptmann
///     all rights reserved
/// more info: http://www.codeproject.com/phShellPath.asp
/// Please contact the author with improvements / modifications.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted under the following conditions:
///     - Redistribution of source must retain the copyright above,
///       and the disclaimer below.
///     - Modifications to the source should be marked clearly, to be
///       distinguishable from the original sources.
///
/// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
/// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
/// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
/// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
/// SUCH DAMAGE.
///


// ---------------- DECLARATIONS --------------------------------------------

// ----- CString Helpers ---------
void  Trim(CStdString & s);
TCHAR GetFirstChar(CStdString const & s);      ///< returns the first char of the string, or 0 if the string length is 0.
TCHAR GetLastChar(CStdString const & s);       ///< returns the last char of the string, or 0 if the string length is 0.


TCHAR GetDriveLetter(TCHAR ch);
TCHAR GetDriveLetter(LPCTSTR s);

CStdString QuoteSpaces(CStdString const & str);


// ==================================================================
//  ERootType
// ------------------------------------------------------------------
///
/// Recognized root types for a path.
/// see nsPath::GetRootType "GetRootType" for more.
///
/// \c len refers to the rootLen parameter optionally returned by \c GetRootType.
///
enum ERootType
{
    rtNoRoot       =  0,    ///< no, or unknown root (\c len = 0)
    rtDriveRoot    =  1,    ///< a drive specification with backslash ("C:\", \c len = 3)
    rtDriveCur     =  2,    ///< a drive specification without backslash ("C:", \c len = 2)
    //   rtPathRoot     =  3,    ///< a path root, i.e. staring with a single backslash (nyi, \c len = 1)
    rtLongPath     =  4,    ///< a UNC long path specification (e.g. "\\?\C:\", \c len is length of path root + 4),  no distinction for "root / current" is made
    rtServerOnly   =  5,    ///< a server only specification (e.g. "\\fileserv" or "\\fileserv\", \c len is the string length)
    rtServerShare  =  6,    ///< server + share specification ("\\server\share", \c len includes the backslash after share if given)
    rtProtocol     =  7,    ///< protocol, ("http://", \c len includes the "://" part)
    rtPseudoProtocol = 8,   ///< pseudo protocol (no slahes) ("mailto:", \c len includes the colon)
    rtServer         = 9,    ///< server with share following (for GetRootType(_,_,greedy=false)
};

ERootType GetRootType(LPCTSTR path, int * pLen, bool greedy = true);



// ==================================================================
//  nsPath::EPathCleanup
// ------------------------------------------------------------------
/// Flags for nsPath::CXShellPath::Cleanup
///
enum EPathCleanup
{
    epcTrim             =   1,      ///< trim outer whitespace
    epcUnquote          =   2,      ///< remove single or double quotes
    epcTrimInQuote      =   4,      ///< trim whitespaces inside quotes
    epcCanonicalize     =   8,      ///< Canonicalize (collapse "\\..\\" and "\\.\\")
    epcRemoveXXL        =   16,     ///< Remove "\\\\?\\" and "\\\\?\\UNC\\" markers
    epcSlashToBackslash = 32,   ///< replace forward slashes with backslashes
    epcMakePretty       =   64,     ///< Windows' idea of a pretty path
    epcRemoveArgs       =  128,     ///< calls PathRemoveArgs (before trimming/unquoting)
    epcRemoveIconLocation = 256,    ///< Remove Icon location from the path
    epcExpandEnvStrings   = 512,    ///< Expand environment strings

    epc_Default = epcTrim |
                  epcUnquote |
                  epcTrimInQuote |
                  epcCanonicalize |
                  epcRemoveXXL |
                  epcExpandEnvStrings, ///< default for CTors and Assignment operators
};


// ==================================================================
//  nsPath::EPathPacking
// ------------------------------------------------------------------
///
/// Flags for nsPath::CXShellPath::GetStr
/// \note
/// eppAutoQuote was ignored in Version 1.2 and before.
/// It is fixed since V.1.3 (August 2005), /// but was removed from
/// epp_Default for backward compatibility
///
enum EPathPacking
{
    eppAutoQuote        =   1,      ///< Quote the path if it contains spaces
    eppAutoXXL          =   2,      ///< If path length is > MAX_PATH, use "\\\\?\\" syntax
    eppBackslashToSlash =   4,      ///< turn backslashes into forward slashes

    epp_Default = eppAutoXXL,
};




// ==================================================================
//  nsPath::CXShellPath
// ------------------------------------------------------------------
///
///
///
class  CXShellPath
{
protected:
    CStdString     m_path;
    void        CAssign(CStdString const & src);

public:

    //@{ \name Construction
    CXShellPath()                     {}
    CXShellPath(LPCSTR path);             ///< Assigns \c path. CXShellPath::Clean(epc_Default) is called for cleanup
    CXShellPath(LPCWSTR path);            ///< Assigns \c path. CXShellPath::Clean(epc_Default) is called for cleanup
    CXShellPath(CStdString const & path);    ///< Assigns \c path  CXShellPath::Clean(epc_Default) is called for cleanup
    CXShellPath(CXShellPath const & path);      ///< Assigns \c path to the path. Does \b not modify the assigned path!
    CXShellPath(CStdString const & path, DWORD cleanup); ///< Assigns \c path, using custom cleanup options (see CXShellPath::Clean)
    //@}


    //@{ \name Assignment
    CXShellPath & operator=(LPCSTR rhs);  ///< Assigns \c path, and calls CXShellPath::Clean(epc_Default)
    CXShellPath & operator=(LPCWSTR rhs); ///< Assigns \c path, and calls CXShellPath::Clean(epc_Default)

    CXShellPath & operator=(CStdString const & rhs); ///< Assigns \c path, and calls CXShellPath::Clean(epc_Default)
    CXShellPath & operator=(CXShellPath const & rhs);   ///< Assigns \c path Does \b not call CXShellPath::Clean!
    CXShellPath & Assign(CStdString const & str, DWORD cleanup = epc_Default);
    //@}

    //@{ \name Miscellaneous Query
    operator LPCTSTR () const
    {
        return m_path.operator LPCTSTR();
    }
    int      GetLength() const
    {
        return m_path.GetLength();    ///< returns the length of the path, in characters
    }
    //@}

    //@{ \name Path concatenation
    CXShellPath & operator &=(LPCTSTR rhs);
    CXShellPath & Append(LPCTSTR appendix);
    CXShellPath & AddBackslash();
    CXShellPath & RemoveBackslash();
    //@}

    //@{ \name Splitting into Path Segments
    CStdString ShellGetRoot() const;
    CXShellPath   GetPath(bool includeRoot  = true) const;
    CStdString GetName() const;
    CStdString GetTitle() const;
    CStdString GetExtension() const;
    ERootType GetRootType(int * len = 0, bool greedy = true) const;
    CStdString GetRoot(ERootType * rt = NULL, bool greedy = true) const;
    CStdString SplitRoot(ERootType * rt = NULL);
    int     GetDriveNumber();
    TCHAR   GetDriveLetter();
    //@}

    //@{ \name  Add / Modify / Remove parts
    CXShellPath & AddExtension(LPCTSTR extension, int len = -1);
    CXShellPath & RemoveExtension();
    CXShellPath & RenameExtension(LPCTSTR newExt);
    CXShellPath & RemoveFileSpec();
    //@}


    //@{ \name Cleanup
    CXShellPath & Trim();
    CXShellPath & Unquote();
    CXShellPath & Canonicalize();
    CXShellPath & ShrinkXXLPath();
    CXShellPath & MakePretty();
    CXShellPath & Clean(DWORD cleanup = epc_Default);
    //@}


    //@{ \name Extractors (with special "packing")
    CStdString GetStr(DWORD packing = epp_Default) const;
    _bstr_t GetBStr(DWORD packing = epp_Default) const;
    //@}

    //@{ \name Static checking (not accessing file system, see also GetRootType)
    bool    IsValid() const;

    bool    IsDot() const;
    bool    IsDotDot() const;
    bool    IsDotty() const;    // IsDot || IsDotDot

    bool    MatchSpec(LPCTSTR spec);
    bool    IsContentType(LPCTSTR contentType)
    {
        return 0 != ::PathIsContentType(m_path, contentType);    ///< compare content type registered for this file, see also MSDN: PathIsContentType
    }
    bool    IsFileSpec()
    {
        return 0 != ::PathIsFileSpec(m_path);            ///< true if path does not contain backslash, see MSDN: PathIsFileSpec
    }
    bool    IsPrefix(LPCTSTR prefix)
    {
        return 0 != ::PathIsPrefix(m_path, prefix);      ///< checks if the path starts with a prefix like "C:\\", see MSDN: PathIsPrefix
    }
    bool    IsRelative()
    {
        return 0 != ::PathIsRelative(m_path);            ///< returns true if the path is relative, see MSDN: PathIsRelative
    }
    bool    IsRoot()
    {
        return 0 != ::PathIsRoot(m_path);                ///< returns true if path is a directory root, see MSDN: PathIsRoot
    }
    bool    IsSameRoot(LPCTSTR other)
    {
        return 0 != ::PathIsSameRoot(m_path, other);     ///< returns true if the path has the same root as \c otherPath, see MSDN: IsSameRoot
    }

    bool    IsUNC()
    {
        return 0 != ::PathIsUNC(m_path);                 ///< returns true if the path is a UNC specification, see MSDN: PathIsUNC
    }
    bool    IsUNCServer()
    {
        return 0 != ::PathIsUNCServer(m_path);           ///< returns true if the path is a UNC server specification, see MSDN: PathIsUNCServer
    }
    bool    IsUNCServerShare()
    {
        return 0 != ::PathIsUNCServerShare(m_path);      ///< returns true if the path is a UNC server + share specification, see MSDN: PathIsUNCServerShare
    }
    bool    IsURL()
    {
        return 0 != ::PathIsURL(m_path);                 ///< returns true if the path is an URL, see MSDN: PathIsURL
    }

    //  bool    IsHTMLFile()                { return 0 != ::PathIsHTMLFile(m_path);         } ///< (missing?) true if content type registered for this file is HTML, see MSDN: PathIsHTMLFile
    //  bool    IsLFNFileSpec()             { return 0 != ::PathISLFNFileSpec(m_path);      } ///< (missing?) true if file is not a 8.3 file, see MSDN: PathIsLFNFileSpec
    //  bool    IsNetworkPath()             { return 0 != ::PathIsNetworkPath(m_path);      } ///< (missing?) returns true if the path is on a network,  see MSDN: PathIsNetworkPath

    //@}


    //@{ \name Relative Paths
    CXShellPath   GetCommonPrefix(LPCTSTR secondPath);
    CXShellPath   RelativePathTo(LPCTSTR pathTo, bool srcIsDir = true);
    bool    MakeRelative(CXShellPath const & basePath);
    bool    MakeAbsolute(CXShellPath const & basePath);
    //@}

    //@{ \name Dialog control operations
    CStdString GetCompactStr(HDC dc, UINT dx, DWORD eppFlags = 0);
    CStdString GetCompactStr(UINT cchMax, DWORD eppFlags = 0, DWORD flags = 0);
    void    SetDlgItem(HWND dlg, UINT dlgCtrlID, DWORD eppFlags = 0);
    //@}


    //@{ \name File System / Environment-Dependent operations
    CXShellPath & SearchAndQualify();
    CXShellPath & FindOnPath(LPCTSTR * additionalDirs = 0);
    bool    Exists() const;
    bool    IsDirectory() const;
    bool    IsSystemFolder(DWORD attrib = FILE_ATTRIBUTE_SYSTEM) const;
    CXShellPath & MakeSystemFolder(bool make = true);
    DWORD   GetAttributes();
    bool    GetAttributes(WIN32_FILE_ATTRIBUTE_DATA & fad);
    CXShellPath & MakeFullPath();
    CXShellPath & ExpandEnvStrings();
    bool    EnvUnexpandRoot(LPCTSTR envVar);
    bool    EnvUnexpandDefaultRoots();
    long    ToRegistry(HKEY baseKey, LPCTSTR subkey, LPCTSTR name, bool replaceEnv = true);
    //@}





    // TODO: Shell 5.0 support
    // V5:    CXShellPath & UnexpandEnvStrings();
    // V5:    LPCTSTR FindSuffixArray(LPCTSTR suffixes, int numSuffixes);
    // V5:    void    PathUndecorate();
    // V5:    CXShellPath PathCreateFromURL(LPCTSTR path, DWORD dwReserved = 0);
    // V5:    bool    IsDirectoryEmpty() const;
    // might be useful for later extensions: PathGetCharType


};

// creation functions:
CXShellPath SplitArgs(CStdString const & path_args, CStdString * args = NULL, DWORD cleanup = epc_Default);
CXShellPath SplitIconLocation(CStdString const & path_icon, int * pIcon = NULL, DWORD cleanup = epc_Default);
CXShellPath BuildRoot(int driveNumber);
CXShellPath GetModuleFileName(HMODULE module = NULL);
CXShellPath GetCurrentDirectory();
CXShellPath FromRegistry(HKEY baseKey, LPCTSTR subkey, LPCTSTR name);


CStdString ReplaceInvalid(CStdString const & str, TCHAR replaceChar = '_');

// concatenation

inline CXShellPath operator & (CXShellPath const & lhs, LPCTSTR rhs)
{
    CXShellPath ret = lhs;
    ret &= rhs;
    return ret;
}

// ---------------- INLIME IMPLEMENTATIONS ----------------------------------

// ==============================================
// GetFirstChar
// ----------------------------------------------
/// \return [TCHAR]: the first char of the string, or 0 if the string length is 0.
/// \note The implementation takes care that the string is not copied when there are no spaces.
inline TCHAR GetFirstChar(CStdString const & s)
{
    if (s.GetLength() == 0)
        return 0;
    else
        return s[0];
}

// ==============================================
// GetLastChar
// ----------------------------------------------
/// \return [TCHAR]: the last character in the string, or 0 if the string length is 0.
/// \par Note
/// \b MBCS: if the string ends with a Multibyte character, this
/// function returns the lead byte of the multibyte sequence.
inline TCHAR GetLastChar(CStdString const & s)
{
    LPCTSTR pstr = s;
    LPCTSTR pLastChar = _tcsdec(pstr, pstr + s.GetLength());
    if (pLastChar == NULL)
        return 0;
    else
        return *pLastChar;
}

} // namespace nsPath
