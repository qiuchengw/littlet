/* 
 * Kenny Liu
 * http://www.codeproject.com/Members/yonken
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */

#include "FileEnumerator.h"

#include <tchar.h>
#include <strsafe.h>
#include <iterator>

#if !defined(_AFX) && !defined(_ATL)
void _cfe_trace( LPCTSTR fmt, ... )
{
/*	TCHAR buf[2000];
	va_list args;
	
	va_start( args, fmt );
	_vsntprintf_s( buf, sizeof(buf), _TRUNCATE, fmt, args );
	va_end( args );
	
	::OutputDebugString(buf);
*/
}
#endif

/*----------------------------------------------------------------------------*/
/* CFileFinder
/*----------------------------------------------------------------------------*/

#define USE_STL_STRING_AS_MEMBER	// use heap instead of stack

#ifdef USE_STL_STRING_AS_MEMBER
	#define STRBUFFER(_str)			_str.c_str()
#else
	#define STRBUFFER(_str)			_str
#endif // USE_STL_STRING_AS_MEMBER

class CFileFinder
{
public:
	CFileFinder(LPCTSTR lpcszInitDir, FindFileData& fileInfo)
		: m_fileInfo(fileInfo)
	{
		Init(lpcszInitDir);
#ifdef _DEBUG
		++s_nInstances;
#endif // _DEBUG
	}

	virtual ~CFileFinder()
	{
#ifdef _DEBUG
		--s_nInstances;
#endif // _DEBUG
	}

public:
	inline bool FindFirst()
	{
		return EnumCurDirFirst();
	}

	inline bool FindCurDirNext()
	{
		bool bRet = ::FindNextFile(m_hFind, &m_fileInfo) != FALSE;
		if ( bRet )
		{
#ifdef USE_STL_STRING_AS_MEMBER
			m_szPathBuffer.resize(m_nFolderLen);
			//m_szPathBuffer[m_nFolderLen] = _T('\0');
			m_szPathBuffer += m_fileInfo.cFileName;
#else
	#ifdef MSVC_NEW_VER
			_tcsncpy_s(m_szPathBuffer+m_nFolderLen, ENUMERATE_PATH_BUFFER_SIZE-m_nFolderLen, m_fileInfo.cFileName, _TRUNCATE);
	#else
			_tcsncpy(m_szPathBuffer+m_nFolderLen, m_fileInfo.cFileName, ENUMERATE_PATH_BUFFER_SIZE-m_nFolderLen);
	#endif // MSVC_NEW_VER
#endif // USE_STL_STRING_AS_MEMBER
		}
		else
		{
			::FindClose(m_hFind);
			m_hFind = INVALID_HANDLE_VALUE;
		}
		return bRet;
	}

	virtual bool Finish() const					{ return INVALID_HANDLE_VALUE == m_hFind; }

	inline LPCTSTR GetPath() const
	{
		return STRBUFFER(m_szPathBuffer) + EXTEND_FILE_PATH_PREFIX_LEN;
	}

	inline const FindFileData& GetFileInfo() const	{ return m_fileInfo; }

	inline bool IsDirectory() const				{ return !!(m_fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	inline bool IsDot() const					{ return (m_fileInfo.cFileName[0] == '.') 
												&& ((m_fileInfo.cFileName[1] == '.') || (m_fileInfo.cFileName[1] == '\0')); }

	inline bool	IsReparsePoint() const			{ return !!(m_fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT); }
protected:
	virtual bool EnumCurDirFirst()
	{
#ifdef USE_STL_STRING_AS_MEMBER
		m_szPathBuffer.resize(m_nFolderLen+2);
#endif // USE_STL_STRING_AS_MEMBER

		m_szPathBuffer[m_nFolderLen++]	= _T('\\');
		m_szPathBuffer[m_nFolderLen]	= _T('*');
#ifndef USE_STL_STRING_AS_MEMBER
		m_szPathBuffer[m_nFolderLen+1]	= _T('\0');
#endif // USE_STL_STRING_AS_MEMBER

		HANDLE hFind = ::FindFirstFile(STRBUFFER(m_szPathBuffer), &m_fileInfo);
		bool bRet = hFind != INVALID_HANDLE_VALUE;
		if ( bRet )
		{
			m_hFind			= hFind;
#ifdef USE_STL_STRING_AS_MEMBER
			m_szPathBuffer.resize(m_nFolderLen);
			//m_szPathBuffer[m_nFolderLen] = _T('\0');
			m_szPathBuffer += m_fileInfo.cFileName;
#else
	#ifdef MSVC_NEW_VER
			_tcsncpy_s(m_szPathBuffer+m_nFolderLen, ENUMERATE_PATH_BUFFER_SIZE-m_nFolderLen, m_fileInfo.cFileName, _TRUNCATE);
	#else
			_tcsncpy(m_szPathBuffer+m_nFolderLen, m_fileInfo.cFileName, ENUMERATE_PATH_BUFFER_SIZE-m_nFolderLen);
	#endif // MSVC_NEW_VER
#endif // USE_STL_STRING_AS_MEMBER
		}
		return bRet;
	}

#ifdef USE_STL_STRING_AS_MEMBER
	void Init(LPCTSTR lpcszInitDir)
	{
		m_nFolderLen = _tcslen(lpcszInitDir);
		
	#ifdef EXTEND_FILE_PATH_LIMIT
		// MSDN stated:
		// Note  Prepending the string "\\?\" does not allow access to the root directory.
		m_szPathBuffer = EXTEND_FILE_PATH_PREFIX ;
		m_szPathBuffer += lpcszInitDir;
		m_nFolderLen += EXTEND_FILE_PATH_PREFIX_LEN;	// for EXTEND_FILE_PATH_PREFIX
	#else
		m_szPathBuffer = lpcszInitDir;
	#endif // EXTEND_FILE_PATH_LIMIT
	
		for (tstring::iterator i = m_szPathBuffer.begin();
			i != m_szPathBuffer.end(); ++i)
		{
			if (*i == _T('/'))
			{
				*i = _T('\\');
			}
		}
		if ( m_szPathBuffer[m_nFolderLen-1] == _T('\\') )
		{
			m_szPathBuffer.erase(m_nFolderLen-1);
			--m_nFolderLen;
		}
	}
#else
	void Init(LPCTSTR lpcszInitDir)
	{
		m_nFolderLen = _tcslen(lpcszInitDir);
	#ifdef EXTEND_FILE_PATH_LIMIT
		// MSDN stated:
		// Note  Prepending the string "\\?\" does not allow access to the root directory.
		#ifdef MSVC_NEW_VER
		_tcsncpy_s(m_szPathBuffer, ENUMERATE_PATH_BUFFER_SIZE, EXTEND_FILE_PATH_PREFIX, EXTEND_FILE_PATH_PREFIX_LEN);
		_tcsncat_s(m_szPathBuffer, ENUMERATE_PATH_BUFFER_SIZE, lpcszInitDir, _TRUNCATE);
		#else
		_tcsncpy(m_szPathBuffer, EXTEND_FILE_PATH_PREFIX, ENUMERATE_PATH_BUFFER_SIZE);
		_tcsncat(m_szPathBuffer, lpcszInitDir, ENUMERATE_PATH_BUFFER_SIZE);
		#endif // MSVC_NEW_VER
		m_nFolderLen += EXTEND_FILE_PATH_PREFIX_LEN;	// for EXTEND_FILE_PATH_PREFIX
	#else
		#ifdef MSVC_NEW_VER
		_tcsncpy_s(m_szPathBuffer, ENUMERATE_PATH_BUFFER_SIZE, lpcszInitDir, _TRUNCATE);
		#else
		_tcsncpy(m_szPathBuffer, lpcszInitDir, ENUMERATE_PATH_BUFFER_SIZE);
		#endif // MSVC_NEW_VER
	#endif // EXTEND_FILE_PATH_LIMIT
		for (int i = 0; i < m_nFolderLen; i++)
		{
			if (m_szPathBuffer[i] == _T('/'))
			{
				m_szPathBuffer[i] = _T('\\');
			}
		}
		if ( m_szPathBuffer[m_nFolderLen-1] == _T('\\') )
		{
			m_szPathBuffer[m_nFolderLen-1] = _T('\0');
			--m_nFolderLen;
		}
	}
#endif // USE_STL_STRING_AS_MEMBER

#ifdef _DEBUG
public:
	static inline bool ZeroInstance()
	{
		return 0 == s_nInstances;
	}
#endif // _DEBUG

protected:
	FindFileData&	m_fileInfo;

#ifdef USE_STL_STRING_AS_MEMBER
	tstring			m_szPathBuffer;
#else
	TCHAR			m_szPathBuffer[ENUMERATE_PATH_BUFFER_SIZE];
#endif // USE_STL_STRING_AS_MEMBER

	UINT			m_nFolderLen;
	HANDLE			m_hFind;

#ifdef _DEBUG
	static UINT		s_nInstances;
#endif // _DEBUG
};

#ifdef _DEBUG
UINT CFileFinder::s_nInstances = 0;
#endif // _DEBUG

#if !defined(FILEENUMERATOR_RECURSION) && !defined(FILEENUMERATOR_BFS)
class CDFSFileFinder : public CFileFinder
{
public:
	CDFSFileFinder(LPCTSTR lpcszInitDir, FindFileData& fileInfo)
		: CFileFinder(lpcszInitDir, fileInfo)
		, m_nFolderLevel(0)
	{
	}
	~CDFSFileFinder()
	{
		CFE_ASSERT(m_nFolderLevel <= static_cast<int>(m_arrEnumFolderCtx.size()));
		for (int ii = 0; ii < m_nFolderLevel; ++ii)
		{
			EnumFolderCtx& efc = m_arrEnumFolderCtx[ii];
			CFE_ASSERT(efc.hFind != INVALID_HANDLE_VALUE);
			::FindClose(efc.hFind);
		}
	}
public:
	virtual bool FindSubDirFirst()
	{
		CFE_ASSERT( IsDirectory() && !IsDot() );
		int nCurFolderLen = m_nFolderLen;
		m_nFolderLen += _tcslen(m_fileInfo.cFileName);
		bool bRet = EnumCurDirFirst();
		if ( !bRet )
		{
			m_nFolderLen = nCurFolderLen;
		}
		return bRet;
	}

	virtual bool FindParentDirNext()
	{
		--m_nFolderLevel;
		if ( m_nFolderLevel > 0 )
		{
			CFE_ASSERT( m_nFolderLevel < static_cast<int>(m_arrEnumFolderCtx.size()) );
			EnumFolderCtx& efcParent	= m_arrEnumFolderCtx[m_nFolderLevel-1];
			CFE_ASSERT(m_nFolderLen > efcParent.nFolderPathLen);
			m_nFolderLen				= efcParent.nFolderPathLen;
			m_hFind						= efcParent.hFind;
			return FindCurDirNext();
		}
		return false;
	}

	virtual bool Finish() const
	{
		return CFileFinder::Finish() && 0 == m_nFolderLevel;
	}
protected:
	virtual bool EnumCurDirFirst()
	{
		bool bRet = CFileFinder::EnumCurDirFirst();
		if ( bRet )
		{
			EnumFolderCtx efc(m_hFind, m_nFolderLen);
			if ( m_nFolderLevel >= static_cast<int>(m_arrEnumFolderCtx.size()) )
				m_arrEnumFolderCtx.push_back( efc );
			else
				m_arrEnumFolderCtx[m_nFolderLevel] = efc;
			++m_nFolderLevel;
		}
		return bRet;
	}
protected:
	struct EnumFolderCtx 
	{
		EnumFolderCtx(HANDLE _hFind = NULL, int _nFolderPathLen = -1)
			: hFind(_hFind)
			, nFolderPathLen(_nFolderPathLen)
		{
		}
		HANDLE hFind;
		int nFolderPathLen;
	};
	std::vector<EnumFolderCtx>	m_arrEnumFolderCtx;
	int							m_nFolderLevel;
};
#endif // !defined(FILEENUMERATOR_RECURSION) && !defined(FILEENUMERATOR_BFS)

//////////////////////////////////////////////////////////////////////
// CFileEnumeratorBase
//////////////////////////////////////////////////////////////////////

#define	IsStopEventSignaled()		(::WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0)

CFileEnumeratorBase::CFileEnumeratorBase()
	: m_dwLastError(ERROR_SUCCESS)
{
#ifdef FILEENUMERATOR_DOCOUNTING
	ResetCounter();
#endif // FILEENUMERATOR_DOCOUNTING
}

CFileEnumeratorBase::~CFileEnumeratorBase()
{
}

#ifdef FILEENUMERATOR_DOCOUNTING
	#define IncreaseFindFolderCounter()			(++m_nFindFolderCount)
	#define IncreaseAcceptedFolderCounter()		(++m_nAcceptedFolderCount)
	#define IncreaseFindFileCounter()			(++m_nFindFileCount)
	#define IncreaseAcceptedFindFileCounter()	(++m_nAcceptedFileCount)
#else
	#define IncreaseFindFolderCounter()
	#define IncreaseAcceptedFolderCounter()
	#define IncreaseFindFileCounter()
	#define IncreaseAcceptedFindFileCounter()
#endif // FILEENUMERATOR_DOCOUNTING

#ifdef FILEENUMERATOR_RECURSION
bool CFileEnumeratorBase::EnumerateRecursively( LPCTSTR lpcszInitDir, FindFileData& findFileData, bool bFindSubDir /*= true*/, HANDLE hStopEvent /*= NULL*/ )
{
	CFileFinder fileFinder(lpcszInitDir, findFileData);
	bool bRet = true;
	if ( !fileFinder.FindFirst() )
	{
		m_dwLastError = ::GetLastError();
		return OnError(lpcszInitDir);
	}
	else
	{
		while ( !fileFinder.Finish() && !IsStopEventSignaled() )
		{
			const FindFileData& fileInfo = fileFinder.GetFileInfo();
			if ( !fileFinder.IsReparsePoint() )
			{
				if ( fileFinder.IsDirectory() )
				{
					if ( !fileFinder.IsDot() && bFindSubDir )
					{
						IncreaseFindFolderCounter();
						if ( CheckUseDir(fileFinder.GetPath(), fileInfo) )
						{
							HandleDir(fileFinder.GetPath(), fileInfo);
							IncreaseAcceptedFolderCounter();
							bRet &= EnumerateRecursively(fileFinder.GetPath(), findFileData, bFindSubDir);
						}
					}
				}
				else
				{
					IncreaseFindFileCounter();
					if ( CheckUseFile(fileFinder.GetPath(), fileInfo) )
					{
						HandleFile(fileFinder.GetPath(), fileInfo);
						IncreaseAcceptedFindFileCounter();
					}
				}
			}
			if ( !fileFinder.FindCurDirNext() )
			{
				FinishedDir( fileFinder.GetPath() );
				break;
			}
		}
	}
	return bRet;
}
#else
#ifdef FILEENUMERATOR_BFS

#include <queue>

//#define USE_BOOST_SHARED_PTR

#ifdef USE_BOOST_SHARED_PTR
	#include <boost/shared_ptr.hpp>
	typedef boost::shared_ptr<CFileFinder>	FileFindPtr;
	#define DELETE_FILEFINDER(_pFinder)
#else
	typedef CFileFinder*					FileFindPtr;
	#define DELETE_FILEFINDER(_pFinder)		delete _pFinder
#endif // USE_BOOST_SHARED_PTR

typedef std::queue<FileFindPtr>				FileFindQueue;

bool CFileEnumeratorBase::EnumerateBFS( LPCTSTR lpcszInitDir, FindFileData& findFileData, HANDLE hStopEvent /*= NULL*/ )
{
	// Breadth-first searching, BFS:
	FileFindPtr finder = NULL;
	try
    {
		finder = new CFileFinder(lpcszInitDir, findFileData);
	}
	catch (...)
	{
		CFE_ASSERT(0);
		DELETE_FILEFINDER(finder);
		return false;
	}

	bool bRet = true;
	FileFindQueue finderQueue;
	
    if ( !finder->FindFirst() )
	{
		m_dwLastError = ::GetLastError();
		OnError(finder->GetPath());
		DELETE_FILEFINDER(finder);
		return false;
	}
	else
	{
		while( !finder->Finish() && !IsStopEventSignaled() )
        {
			const FindFileData& fileInfo = finder->GetFileInfo();
			
			// Skips the junction/symbolic link to avoid infinite looping
			if ( !finder->IsReparsePoint() )
			{
				if( finder->IsDirectory() )
				{
					if ( !finder->IsDot() )
					{
						IncreaseFindFolderCounter();
						if ( CheckUseDir(finder->GetPath(), fileInfo) )
						{
							HandleDir(finder->GetPath(), fileInfo);
							IncreaseAcceptedFolderCounter();
							
							FileFindPtr newFinder = NULL;
							try
							{
								newFinder = new CFileFinder(finder->GetPath(), findFileData);
								finderQueue.push(newFinder);
							}
							catch (...)
							{
								CFE_ASSERT(0);
								DELETE_FILEFINDER(newFinder);
							}
						}
					}
				}
				else
				{
					IncreaseFindFileCounter();
					if ( CheckUseFile(finder->GetPath(), fileInfo) )
					{
						HandleFile(finder->GetPath(), fileInfo);
						IncreaseAcceptedFindFileCounter();
					}
				}
			}
            if ( !finder->FindCurDirNext() )
			{
				FinishedDir( finder->GetPath() );
				if ( finderQueue.empty() )
					break;
				else
				{
					while ( !IsStopEventSignaled() )
					{
						FileFindPtr nextFinder = finderQueue.front();
						finderQueue.pop();

						DELETE_FILEFINDER(finder);

						finder = nextFinder;

						if ( !finder->FindFirst() )
						{
							m_dwLastError = ::GetLastError();
							if ( !OnError(finder->GetPath()) )
							{
								bRet = false;
								goto CleanupRet;
							}
						}
						else
							break;
					}
				}
			}
        }
    }
CleanupRet:
	#ifndef USE_BOOST_SHARED_PTR
	while (1)
	{
		DELETE_FILEFINDER(finder);
		if ( finderQueue.empty() )
			break;
		else
		{
			finder = finderQueue.front();
			finderQueue.pop();
		}
	};
	#endif // USE_BOOST_SHARED_PTR
	return bRet;
}

#else

bool CFileEnumeratorBase::EnumerateDFS( LPCTSTR lpcszInitDir, FindFileData& findFileData, HANDLE hStopEvent /*= NULL*/ )
{
	// depth-first searching, DFS:
	// whenever we meet a folder, we dig into it
	CDFSFileFinder fileFinder(lpcszInitDir, findFileData);
	if ( !fileFinder.FindFirst() )
	{
		m_dwLastError = ::GetLastError();
		OnError(fileFinder.GetPath());
		return false;
	}
	else
	{
		while ( !fileFinder.Finish() && !IsStopEventSignaled() )
		{
			const FindFileData& fileInfo = fileFinder.GetFileInfo();
			if ( !fileFinder.IsReparsePoint() )
			{
				if ( fileFinder.IsDirectory() )
				{
					if ( !fileFinder.IsDot() )
					{
						IncreaseFindFolderCounter();
						if ( CheckUseDir(fileFinder.GetPath(), fileInfo) )
						{
							HandleDir(fileFinder.GetPath(), fileInfo);
							IncreaseAcceptedFolderCounter();
							if ( !fileFinder.FindSubDirFirst() && !OnError(fileFinder.GetPath()) )
								return false;
						}
					}
				}
				else
				{
					IncreaseFindFileCounter();
					if ( CheckUseFile(fileFinder.GetPath(), fileInfo) )
					{
						HandleFile(fileFinder.GetPath(), fileInfo);
						IncreaseAcceptedFindFileCounter();
					}
				}
			}
			// Retrace
			if ( !fileFinder.FindCurDirNext() )
			{
				do 
				{
					FinishedDir( fileFinder.GetPath() );
				} while ( !fileFinder.FindParentDirNext() && !fileFinder.Finish() && !IsStopEventSignaled() );
			}
		}
	}
	return true;
}
#endif // FILEENUMERATOR_BFS
#endif // FILEENUMERATOR_RECURSION

bool CFileEnumeratorBase::Enumerate( LPCTSTR lpcszInitDir, bool bFindSubDir /*= true*/, HANDLE hStopEvent /*= NULL*/ )
{
	if ( !lpcszInitDir || !*lpcszInitDir )
		return false;

	bool bRet		= true;

#ifdef RUN_32BIT_APP_ON_64BIT_OS
#if _MSC_VER >= 1300 && !defined(_M_X64)
	// We are writing a 32-bit application to list all the files in a directory and the application may be run on a 64-bit computer
	// See http://msdn.microsoft.com/en-us/library/aa364418(VS.85).aspx for more detail
	PVOID OldValue = NULL;
	Wow64DisableWow64FsRedirection(&OldValue);
#endif
#endif // RUN_32BIT_APP_ON_64BIT_OS

	FindFileData findFileData;

#ifdef FILEENUMERATOR_RECURSION
	bRet = EnumerateRecursively(lpcszInitDir, findFileData, bFindSubDir, hStopEvent);
#else
	if ( bFindSubDir )
	{
	#ifdef FILEENUMERATOR_BFS
		bRet = EnumerateBFS(lpcszInitDir, findFileData, hStopEvent);
	#else
		bRet = EnumerateDFS(lpcszInitDir, findFileData, hStopEvent);
	#endif // FILEENUMERATOR_BFS
	}
	else
	{
		CFileFinder fileFinder(lpcszInitDir, findFileData);
		if ( !fileFinder.FindFirst() )
		{
			m_dwLastError = ::GetLastError();
			OnError(lpcszInitDir);
			bRet = false;
		}
		else
		{
			for ( ; !fileFinder.Finish() && !IsStopEventSignaled(); fileFinder.FindCurDirNext() )
			{
				const FindFileData& fileInfo = fileFinder.GetFileInfo();
				if ( !fileFinder.IsDirectory() )
				{
					if ( CheckUseFile(fileFinder.GetPath(), fileInfo) )
					{
						HandleFile(fileFinder.GetPath(), fileInfo);
						IncreaseFindFileCounter();
					}
				}
			}
		}
	}
#ifdef _DEBUG
	CFE_ASSERT( CFileFinder::ZeroInstance() );	// memory leaks?
#endif
#endif // FILEENUMERATOR_RECURSION

#ifdef RUN_32BIT_APP_ON_64BIT_OS
#if _MSC_VER >= 1300 && !defined(_M_X64)
	Wow64RevertWow64FsRedirection(OldValue);
#endif
#endif // RUN_32BIT_APP_ON_64BIT_OS
	return bRet;
}

//////////////////////////////////////////////////////////////////////
// CFilteredFileEnumerator
//////////////////////////////////////////////////////////////////////

CFilteredFileEnumerator::CFilteredFileEnumerator()
{
	
}

CFilteredFileEnumerator::~CFilteredFileEnumerator()
{
	
}

void CFilteredFileEnumerator::SetFilterPatterns( LPCTSTR lpcszFileIncPattern /*= NULL*/, 
												LPCTSTR lpcszFileExcPattern /*= NULL*/, 
												LPCTSTR lpcszDirIncPattern /*= NULL*/, 
												LPCTSTR lpcszDirExcPattern /*= NULL */ 
												)
{
	tstring strFileIncPattern = lpcszFileIncPattern ? lpcszFileIncPattern : _T("");
	tstring strFileExcPattern = lpcszFileExcPattern ? lpcszFileExcPattern : _T("");
	tstring strDirIncPattern = lpcszDirIncPattern ? lpcszDirIncPattern : _T("");
	tstring strDirExcPattern = lpcszDirExcPattern ? lpcszDirExcPattern : _T("");

	if (!strFileIncPattern.empty())
		Tokenize(m_slFileIncludePattern,  strFileIncPattern);
	if (!strFileExcPattern.empty())
		Tokenize(m_slFileExcludePattern, strFileExcPattern);
	if (!strDirIncPattern.empty())
		Tokenize(m_slDirIncludePattern,  strDirIncPattern);
	if (!strDirExcPattern.empty())
		Tokenize(m_slDirExcludePattern, strDirExcPattern);
}

bool CFilteredFileEnumerator::CheckUseFile( LPCTSTR lpcszPath, const FindFileData& ffd )
{
	tstring strFile = ffd.cFileName;
	return ( (m_slFileExcludePattern.empty() || !CompareList(m_slFileExcludePattern, strFile)) 
		&& (m_slFileIncludePattern.empty() || CompareList(m_slFileIncludePattern, strFile)) 
		);
}

bool CFilteredFileEnumerator::CheckUseDir( LPCTSTR lpcszPath, const FindFileData& ffd )
{
	tstring strFolder = ffd.cFileName;
	return ( (m_slDirExcludePattern.empty() || !CompareList(m_slDirExcludePattern, strFolder)) 
		&& (m_slDirIncludePattern.empty() || CompareList(m_slDirIncludePattern, strFolder)) 
		);
}

void CFilteredFileEnumerator::Tokenize( stringlist& plsTokenized, tstring& sPattern )
{
	// search strings are tokenized by ';' (semicolon) character
	
	tstring::size_type position = 0;
	tstring::size_type length = 0;
	
	while (true)
	{
		position = sPattern.find_first_not_of(';', length);
		length = sPattern.find_first_of(';', length + 1);
		if ( position == tstring::npos )
			break;
		
		plsTokenized.push_back(sPattern.substr(position, length - position));
	}
}

bool CompareStrings(LPCTSTR sPattern, LPCTSTR sFileName, bool bNoCase = true)
{
	TCHAR temp1[2] = _T("");
	TCHAR temp2[2] = _T("");
	LPCTSTR pStar  = 0;
	LPCTSTR pName  = 0;
	
	while(*sFileName)
	{
		switch (*sPattern)
		{
		case '?':
			++sFileName; ++sPattern;
			continue;
		case '*':
			if (!*++sPattern) return 1;
			pStar = sPattern;
			pName = sFileName + 1;
			continue;
		default:
			if(bNoCase) 
			{
				// _tcsicmp works with strings not chars !!
				*temp1 = *sFileName;
				*temp2 = *sPattern;
				if (!_tcsicmp(temp1, temp2))     // if equal
				{
					++sFileName; 
					++sPattern; 
					continue;
				}
			}
			else if (*sFileName == *sPattern)    // bNoCase == false, 
			{                                    // compare chars directly
				++sFileName; 
				++sPattern; 
				continue;
			}
			
			// chars are NOT equal, 
			// if there was no '*' thus far, strings don't match
			if(!pStar) return 0;
			
			// go back to last '*' character
			sPattern = pStar;
			sFileName = pName++;
			continue;
		}
	}
	// check is there anything left after last '*'
	while (*sPattern == '*') ++sPattern;
	return (!*sPattern);
}

bool CFilteredFileEnumerator::CompareList( stringlist& plsPattern, tstring& sFileName )
{
	stringlist::iterator iter = plsPattern.begin();
	
	for(; iter != plsPattern.end(); ++iter)
	{
		//if (str_pattern_match(iter->c_str(), sFileName.c_str()) )
		if (CompareStrings(iter->c_str(), sFileName.c_str()) )
			return true;
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
// QFileFinder
QFileFinder::QFileFinder( LPCTSTR pszFolder,LPCTSTR pszFilePatten ,BOOL bIncSubPath)
{
	CFilteredFileEnumerator::SetFilterPatterns(pszFilePatten);
	CFilteredFileEnumerator::Enumerate(pszFolder,bIncSubPath);
}

void QFileFinder::HandleFile( LPCTSTR lpcszPath, const FindFileData& ffd )
{
	m_lstFile.push_back(lpcszPath);
}

void QFileFinder::GetFileList( FileList & lst )
{
	std::copy(m_lstFile.begin(),m_lstFile.end(),
		std::back_insert_iterator<FileList>(lst));
}

void QFileFinder::Reset()
{
	m_lstFile.clear();
}
