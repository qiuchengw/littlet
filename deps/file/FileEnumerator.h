/*------------------------------------------------------------------------------*
 * Creation: Kenny Liu 06/30/2010
 *
 * Latest version:
 * http://www.codeproject.com/KB/files/IterativeFileEnumerator.aspx
 *
 * Inspired by:
 * 1. http://www.codeproject.com/KB/files/CEnum_enumeration.aspx
 * 2. http://www.codeproject.com/KB/cpp/recursedir.aspx
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *------------------------------------------------------------------------------*/

#ifndef FILE_ENUMERATOR_H_
#define FILE_ENUMERATOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>

#pragma warning(push, 2)
#pragma warning(disable: 4786)
#include <string>
#include <vector>
#include <list>

using std::vector;
#pragma warning(pop)

#ifdef _UNICODE
	typedef std::wstring	tstring;
#else
	typedef std::string		tstring;
#endif


typedef WIN32_FIND_DATA			FindFileData, *PFileInfo;

#if defined(_AFX)
	#define CFE_TRACE	TRACE
	#define CFE_ASSERT	ATLASSERT
	#define CFE_VERIFY	ATLVERIFY
#elif defined(_ATL)
	#define CFE_TRACE	ATLTRACE
	#define CFE_ASSERT	ATLASSERT
	#define CFE_VERIFY	ATLVERIFY
#else
	#define CFE_TRACE	_cfe_trace
	#define CFE_ASSERT	assert
	#ifdef _DEBUG
	#define CFE_VERIFY	CFE_ASSERT
	#else
	#define CFE_VERIFY(f) ((void)(f))
	#endif // _DEBUG
	#include <assert.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <tchar.h>

	void _cfe_trace(LPCTSTR fmt, ...);
#endif

/*----------------------------------------------------------------------------*/
/* Global control
/*----------------------------------------------------------------------------*/
// http://msdn.microsoft.com/en-us/library/aa364418(VS.85).aspx
#define EXTEND_FILE_PATH_PREFIX			_T("\\\\?\\")

//#define RUN_32BIT_APP_ON_64BIT_OS

#ifdef _UNICODE
	#define EXTEND_FILE_PATH_LIMIT
#endif // _UNICODE

#ifdef EXTEND_FILE_PATH_LIMIT
	#define ENUMERATE_PATH_BUFFER_SIZE		32767
	#define EXTEND_FILE_PATH_PREFIX_LEN		4
#else
	#define ENUMERATE_PATH_BUFFER_SIZE		MAX_PATH
	#define EXTEND_FILE_PATH_PREFIX_LEN		0
#endif // EXTEND_FILE_PATH_LIMIT


//#define FILEENUMERATOR_RECURSION
//#define FILEENUMERATOR_DOCOUNTING

#ifndef FILEENUMERATOR_RECURSION
	#define FILEENUMERATOR_BFS
#endif

class CFileEnumeratorBase
{
public:
	CFileEnumeratorBase();
	virtual ~CFileEnumeratorBase();
public:
	virtual bool Enumerate(LPCTSTR lpcszInitDir, bool bFindSubDir = true, HANDLE hStopEvent = NULL);

	inline DWORD GetLastError() const				{ return m_dwLastError; }

#ifdef FILEENUMERATOR_DOCOUNTING
	inline size_t	GetFindFolderCount() const			{ return m_nFindFolderCount; }
	inline size_t	GetAcceptedFolderCount() const		{ return m_nAcceptedFolderCount; }
	inline size_t	GetFindFileCount() const			{ return m_nFindFileCount; }
	inline size_t	GetAcceptedFileCount() const		{ return m_nAcceptedFileCount; }
	inline void		ResetCounter()						{ m_nAcceptedFileCount = m_nFindFileCount = m_nFindFolderCount = m_nAcceptedFolderCount = 0; }
#endif // FILEENUMERATOR_DOCOUNTING

protected:
	// return true to enumerating files, or false to skit it
	virtual bool CheckUseFile(LPCTSTR /*lpcszPath*/, const FindFileData& /*ffd*/)		{return true;}

	// return true to enumerate a sub directory, or false to skip it
	virtual bool CheckUseDir(LPCTSTR /*lpcszPath*/, const FindFileData& /*ffd*/)		{return true;}

	// default handler for a file that currently enumerated
	virtual void HandleFile(LPCTSTR lpcszPath, const FindFileData& /*ffd*/)
	{
#ifdef FILEENUMERATOR_DOCOUNTING
		//CFE_TRACE(_T("[%d] %s\n"), m_nFindFileCount, lpcszPath);
#endif // FILEENUMERATOR_DOCOUNTING
	}

	// default handler for a directory that currently enumerated
	virtual void HandleDir(LPCTSTR /*lpcszPath*/, const FindFileData& /*ffd*/)			{}

	// Called when all files and/or sub-directories within a directory has been finished visiting.
	virtual void FinishedDir(LPCTSTR /*lpcszPath*/)										{}

	// return true to ignore error and continue enumerating (if possible).
	virtual bool OnError(LPCTSTR lpcszPath = NULL)
	{
		CFE_TRACE(_T("\t+++++++++ CFileEnumerator encountered an error for %s! lasterr = %08X\n"), lpcszPath, GetLastError());
		return true;
	}

#ifdef FILEENUMERATOR_RECURSION
	bool EnumerateRecursively(LPCTSTR lpcszInitDir, FindFileData& findFileData, bool bFindSubDir = true, HANDLE hStopEvent = NULL);
#else
	#ifdef FILEENUMERATOR_BFS
	// Breadth-first search
	bool EnumerateBFS(LPCTSTR lpcszInitDir, FindFileData& findFileData, HANDLE hStopEvent = NULL);
	#else
	// Depth-first search
	bool EnumerateDFS(LPCTSTR lpcszInitDir, FindFileData& findFileData, HANDLE hStopEvent = NULL);
	#endif // FILEENUMERATOR_BFS
#endif // FILEENUMERATOR_RECURSION

protected:
	DWORD	m_dwLastError;
#ifdef FILEENUMERATOR_DOCOUNTING
	size_t	m_nFindFolderCount;
	size_t	m_nAcceptedFolderCount;
	size_t	m_nFindFileCount;
	size_t	m_nAcceptedFileCount;
#endif // FILEENUMERATOR_DOCOUNTING
};

typedef std::vector<tstring>	stringlist;

class CFilteredFileEnumerator : public CFileEnumeratorBase
{
public:
	CFilteredFileEnumerator();
	virtual ~CFilteredFileEnumerator();
public:
	void SetFilterPatterns(LPCTSTR lpcszFileIncPattern = NULL, 
				LPCTSTR lpcszFileExcPattern = NULL, 
				LPCTSTR lpcszDirIncPattern = NULL, 
				LPCTSTR lpcszDirExcPattern = NULL
				);
protected:
	virtual bool CheckUseFile(LPCTSTR lpcszPath, const FindFileData& ffd);

	virtual bool CheckUseDir(LPCTSTR lpcszPath, const FindFileData& ffd);

	void Tokenize(stringlist& plsTokenized, tstring& sPattern);

	bool CompareList(stringlist& plsPattern, tstring& sFileName);
protected:
	stringlist		m_slFileIncludePattern;
	stringlist		m_slFileExcludePattern;
	stringlist		m_slDirIncludePattern;
	stringlist		m_slDirExcludePattern;
};

typedef std::vector<tstring> FileList;
typedef FileList::iterator FileListItr;
class QFileFinder : public CFilteredFileEnumerator
{
public:
	QFileFinder( LPCTSTR pszFolder,LPCTSTR pszFilePatten ,BOOL bIncSubPath=FALSE);
	void GetFileList(FileList & lst);
	void Reset();

protected:
	virtual void HandleFile(LPCTSTR lpcszPath, const FindFileData& ffd);

private:
	FileList	m_lstFile;	
};

#endif // #ifndef FILE_ENUMERATOR_H_
