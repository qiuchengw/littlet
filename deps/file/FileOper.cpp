#include "fileoper.h"
#include <tchar.h>
#include <strsafe.h>
#include <cassert>
#include <atlcomcli.h>

namespace quibase
{

//////////////////////////////////////////////////////////////////////////
// brief	:	
// 
//////////////////////////////////////////////////////////////////////////
HRESULT CWinPathApi::CreateFullPath(LPCWSTR lpszFileName, DWORD dwFileAttribute/* = FILE_ATTRIBUTE_NORMAL*/)
{
	CStdString strPath = lpszFileName;

	if (!strPath.IsEmpty() && L'\\' != strPath[strPath.GetLength() - 1])
	{
		strPath.AppendFormat(L"\\");
	}

	int nPos = strPath.Find(L'\\');
	while (-1 != nPos)
	{
		CStdString strParent    = strPath.Left(nPos);
		DWORD   dwFileAttrib = ::GetFileAttributes(strParent);
		if (INVALID_FILE_ATTRIBUTES == dwFileAttrib)
		{
			BOOL bRet = ::CreateDirectory(strParent, NULL);
			if (!bRet)
				return S_FALSE;//return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

			::SetFileAttributes( strParent, dwFileAttribute );
		}

		nPos = strPath.Find(_T('\\'), nPos + 1);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// brief	:	
// 
//////////////////////////////////////////////////////////////////////////

// used in CWinPathApi::IsDirectory
#pragma comment(lib, "mpr.lib")

#define LEN_MID_ELLIPSES        4   // for "...\\"
#define LEN_END_ELLIPSES        3   // for "..."
#define MIN_CCHMAX              (LEN_MID_ELLIPSES + LEN_END_ELLIPSES)

HRESULT CWinPathApi::ExpandFullPathName(CStdString& strPathName)
{
	DWORD dwLen = ::GetFullPathName(strPathName, 0, NULL, NULL);
	if (0 == dwLen)
		return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


	assert(dwLen <= WIN_PATH_MAX_UNICODE_PATH);
	if (dwLen > WIN_PATH_MAX_UNICODE_PATH)
		return E_UNEXPECTED;


	CStdString strFullPathName;
	dwLen = ::GetFullPathName(strPathName, dwLen, strFullPathName.GetBuffer(dwLen), NULL);
	if (0 == dwLen)
	{
		strFullPathName.ReleaseBuffer(0);
		return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
	}

	strFullPathName.ReleaseBuffer();
	strPathName = strFullPathName;
	return S_OK;
}

static BOOL IsLongPathName(LPCTSTR lpFilePath)
{
	BOOL bResult = FALSE;

	if ( lpFilePath != NULL && lpFilePath[0] != 0 )
	{
		if ( (lpFilePath[1] == _T(':'))
			|| (lpFilePath[0] == _T('\\') && lpFilePath[1] == _T('\\'))
			)
		{
			bResult = (_tcschr(lpFilePath, _T('~')) == NULL);
		}
	}

	return bResult;
}


HRESULT CWinPathApi::ExpandLongPathName(CStdString& strPathName)
{
	if ( IsLongPathName(strPathName) )
	{
		return S_OK;
	}

	DWORD dwLen = ::GetLongPathName(strPathName, NULL, 0);
	if (0 == dwLen)
		return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


	assert(dwLen <= WIN_PATH_MAX_UNICODE_PATH);
	if (dwLen > WIN_PATH_MAX_UNICODE_PATH)
		return E_UNEXPECTED;


	CStdString strLongPathName;
	dwLen = ::GetLongPathName(strPathName, strLongPathName.GetBuffer(dwLen), dwLen);
	if (0 == dwLen)
	{
		strLongPathName.ReleaseBuffer(0);
		return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
	}


	strLongPathName.ReleaseBuffer();
	strPathName = strLongPathName;
	return S_OK;
}

BOOL CWinPathApi::ExpandSpecialFolderPathAtBeginning(LPWSTR lpszPath, DWORD cchBuf, LPCWSTR lpszPattern, int csidl)
{
	assert(lpszPath);
	assert(lpszPattern);
	assert(cchBuf >= MAX_PATH);

	// find pattern
	size_t cchPatternLen = wcslen(lpszPattern);
	if (0 != StrCmpNIW(lpszPath, lpszPattern, (int)min(cchPatternLen, (size_t)cchBuf)))
		return FALSE;

	// retrieve csidl
	WCHAR szSpecFolder[MAX_PATH];
	BOOL br = ::SHGetSpecialFolderPath(NULL, szSpecFolder, csidl, FALSE);
	if (!br)
		return FALSE;

	StringCchCat(szSpecFolder, MAX_PATH, L"\\");

	// replace pattern at beginning
	size_t cchSpecFolderLen = wcslen(szSpecFolder);


	// calculate new length
	size_t cchOrigPathLen = wcslen(lpszPath);
	assert(cchOrigPathLen + cchSpecFolderLen > cchPatternLen);
	size_t cchNewPathLen  = cchOrigPathLen + cchSpecFolderLen - cchPatternLen;
	if (cchOrigPathLen + cchSpecFolderLen <= cchPatternLen)
		return FALSE;

	if (cchNewPathLen + 1 >= cchBuf)
		return FALSE;

	// "memmove" can safely handle the situation where the source string overlaps the destination string 
	// move string right after pattern
	memmove(lpszPath + cchSpecFolderLen, lpszPath + cchPatternLen, sizeof(WCHAR) * (cchOrigPathLen - cchPatternLen));
	memcpy(lpszPath, szSpecFolder, sizeof(WCHAR) * cchSpecFolderLen);
	lpszPath[cchNewPathLen] = L'\0';


	return TRUE;
}

BOOL CWinPathApi::ExpandSpecialFolderPathAtBeginning(CStdString& strPath, LPCWSTR lpszPattern, int csidl)
{
	assert(lpszPattern);

	BOOL br = CWinPathApi::ExpandSpecialFolderPathAtBeginning(strPath.GetBuffer(MAX_PATH + 1), MAX_PATH, lpszPattern, csidl);
	strPath.ReleaseBuffer();
	return br;
}

BOOL CWinPathApi::ExpandPatternAtBeginning(LPWSTR lpszPath, DWORD cchBuf, LPCWSTR lpszPattern, LPCWSTR lpszExpandAs)
{
	assert(lpszPath);
	assert(lpszPattern);
	assert(lpszExpandAs);
	if (!cchBuf)
		return FALSE;


	// find pattern
	size_t cchPatternLen = wcslen(lpszPattern);
	if (0 != StrNCmpI(lpszPath, lpszPattern, (int)min(cchPatternLen, (size_t)cchBuf)))
		return FALSE;


	size_t cchExpandAsLen = wcslen(lpszExpandAs);


	// calculate new length
	size_t cchOrigPathLen = wcslen(lpszPath);
	assert(cchOrigPathLen + cchExpandAsLen > cchPatternLen);
	size_t cchNewPathLen  = cchOrigPathLen + cchExpandAsLen - cchPatternLen;
	if (cchOrigPathLen + cchExpandAsLen <= cchPatternLen)
		return FALSE;

	if (cchNewPathLen + 1 >= cchBuf)
		return FALSE;

	// "memmove" can safely handle the situation where the source string overlaps the destination string 
	// move string right after pattern
	memmove(lpszPath + cchExpandAsLen, lpszPath + cchPatternLen, sizeof(WCHAR) * (cchOrigPathLen - cchPatternLen));
	memcpy(lpszPath, lpszExpandAs, sizeof(WCHAR) * cchExpandAsLen);
	lpszPath[cchNewPathLen] = L'\0';


	return TRUE;
}

BOOL CWinPathApi::ExpandPatternAtBeginning(CStdString& strPath, LPCWSTR lpszPattern, LPCWSTR lpszExpandAs)
{
	assert(lpszPattern);
	assert(lpszExpandAs);

	int nMaxLen = strPath.GetLength() + (int)wcslen(lpszExpandAs);
	BOOL br = CWinPathApi::ExpandPatternAtBeginning(strPath.GetBuffer(nMaxLen), nMaxLen, lpszPattern, lpszExpandAs);
	strPath.ReleaseBuffer();
	return br;
}

BOOL CWinPathApi::ExpandEnvironmentStrings(LPCWSTR lpszSrc, LPWSTR lpszDest, DWORD cchBuf)
{
	assert(lpszSrc);
	assert(lpszDest);
	assert(cchBuf);

	DWORD dwLen = 0;
	if( lpszSrc && *lpszSrc==_T('%') )
	{
		dwLen = ::ExpandEnvironmentStrings(lpszSrc, lpszDest, cchBuf);
	}

	dwLen = ::ExpandEnvironmentStrings(lpszSrc, lpszDest, cchBuf);
	if (0 == dwLen)
	{
		StrNCpy(lpszDest, lpszSrc, cchBuf);
		return FALSE;
	}

	return TRUE;
}

BOOL CWinPathApi::ExpandEnvironmentStrings(CStdString& strPath)
{
	if (strPath.IsEmpty() || L'%' != strPath[0])
	{
		return FALSE;
	}

	DWORD dwLen = ::ExpandEnvironmentStrings(strPath, NULL, 0);
	if (0 == dwLen)
		return FALSE;


	assert(dwLen <= WIN_PATH_MAX_UNICODE_PATH);
	if (dwLen > WIN_PATH_MAX_UNICODE_PATH)
		return FALSE;


	CStdString strLongPathName;
	BOOL br = CWinPathApi::ExpandEnvironmentStrings(strPath, strLongPathName.GetBuffer(dwLen), dwLen);
	if (!br)
	{
		strLongPathName.ReleaseBuffer(0);
		return br;
	}

	strLongPathName.ReleaseBuffer();
	strPath = strLongPathName;
	return br;
}

void CWinPath::ExpandEnvironmentStrings()
{
	CWinPathApi::ExpandEnvironmentStrings(m_strPath);
}

void CWinPath::ExpandNormalizedPathName()
{
	if (m_strPath.IsEmpty())
		return;

	this->ExpandEnvironmentStrings();
	this->ExpandAsAccessiblePath();

	if (-1 != m_strPath.Find(L'~', 0))
	{   // 仅在必要的时候展开成长路径
		CWinPathApi::ExpandLongPathName(m_strPath);
	}

	this->AddUnicodePrefix();
	m_strPath.MakeLower();
}

BOOL CWinPathApi::ExpandAsAccessiblePath(LPWSTR lpszPath, DWORD cchBuf)
{
	assert(lpszPath);
	assert(cchBuf);

	if (CWinPathApi::ExpandSpecialFolderPathAtBeginning(lpszPath, cchBuf, L"System32\\", CSIDL_SYSTEM))
	{
		return TRUE;
	}
	else if (CWinPathApi::ExpandSpecialFolderPathAtBeginning(lpszPath, cchBuf, L"\\SystemRoot\\", CSIDL_WINDOWS))
	{
		return TRUE;
	}
	else if (CWinPathApi::ExpandPatternAtBeginning(lpszPath, cchBuf, L"\\??\\", L""))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CWinPathApi::ExpandAsAccessiblePath(CStdString& strPath)
{
	if (CWinPathApi::ExpandSpecialFolderPathAtBeginning(strPath, L"System32\\", CSIDL_SYSTEM))
	{
		return TRUE;
	}
	else if (CWinPathApi::ExpandSpecialFolderPathAtBeginning(strPath, L"\\SystemRoot\\", CSIDL_WINDOWS))
	{
		return TRUE;
	}
	else if (CWinPathApi::ExpandPatternAtBeginning(strPath, L"\\??\\", L""))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CWinPathApi::HasUnicodePrefix(LPCWSTR pszPath)
{
	if (!pszPath)
		return FALSE;

	if (0 != StrCmpNW(pszPath, L"\\\\?\\", 4))
		return FALSE;

	return TRUE;
}


LPCWSTR CWinPathApi::FindAfterAnyPrefix(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return pszPath;

	if (HasUnicodePrefix(pszPath))
	{
		pszPath += WIN_PATH_UNICODE_PATH_PREFIX;
		if (0 == StrCmpNIW(pszPath, L"UNC\\", 4))
		{
			pszPath += 4;
		}
	}
	else if (IsUNC(pszPath))
	{
		pszPath += 2;
	}

	return pszPath;
}

LPCWSTR CWinPathApi::FindAfterUnicodePrefix(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return pszPath;

	if (HasUnicodePrefix(pszPath))
	{
		pszPath += WIN_PATH_UNICODE_PATH_PREFIX;
	}

	return pszPath;
}

LPCWSTR CWinPathApi::FindFileName(LPCWSTR pszPath)
{
	if (!pszPath)
		return pszPath;

	if (L'\\' == pszPath[0] && L'\\' == pszPath[1] && L'\0' == pszPath[2])
		return pszPath;

	pszPath = FindAfterAnyPrefix(pszPath);
	LPCWSTR lpFound = pszPath;
	for (NULL; *pszPath; ++pszPath)
	{
		if (L'\\' == pszPath[0] ||
			L':'  == pszPath[0] ||
			L'/'  == pszPath[0])
		{
			if (L'\0' != pszPath[1] &&
				L'\\' != pszPath[1] &&
				L'/'  != pszPath[1])
			{
				lpFound = pszPath + 1;
			}
		}
	}

	return lpFound;
}

LPCWSTR CWinPathApi::FindExtension(LPCWSTR pszPath)
{
	if (!pszPath)
		return NULL;

	LPCWSTR lpExtension = NULL;
	for (NULL; *pszPath; ++pszPath)
	{
		switch(*pszPath)
		{
		case L'.':              // find last dot
			lpExtension = pszPath;
			break;

		case L'\\':
			lpExtension = NULL;
			break;
		}
	}

	return lpExtension ? lpExtension : pszPath; /// return dot or last '\0'
}

HRESULT CWinPathApi::CreateLnkFile(LPCWSTR pszPath, LPCWSTR pszArguments, LPCWSTR pszDesc, LPCWSTR pszLnkFilePath)
{
	assert(pszPath);
	assert(pszLnkFilePath);
	assert(false);

/*	CComPtr<IShellLink> spiShellLink;

	QString strFullPath = pszPath;
	HRESULT hr = ExpandFullPathName(strFullPath);
	if (FAILED(hr))
		return hr;


	hr = ::CoCreateInstance(
		CLSID_ShellLink,
		NULL, 
		CLSCTX_INPROC_SERVER,
		IID_IShellLink,
		(void**)&spiShellLink);
	if (FAILED(hr))
		return hr;


	CComPtr<IPersistFile> spiPersistFile;
	hr = spiShellLink.QueryInterface(&spiPersistFile);
	if (FAILED(hr))
		return hr;

	// Set the path to the shortcut target and add the description
	hr = spiShellLink->SetPath(strFullPath);
	if (FAILED(hr))
		return hr;


	if (pszArguments)
	{
		hr = spiShellLink->SetArguments(pszArguments);
		if (FAILED(hr))
			return hr;
	}

	if (pszDesc)
	{
		hr = spiShellLink->SetDescription(pszDesc);
		if (FAILED(hr))
			return hr;
	}



	// Write the shortcut to disk
	hr = spiPersistFile->Save(pszLnkFilePath, TRUE);
	if (FAILED(hr))
		return hr;

*/
	return S_OK;

}

HRESULT CWinPathApi::ResolveLnkFileNoSafe(LPCWSTR pszLnkFile, CStdString& strTargetPath, DWORD dwFlag)
{
	assert(pszLnkFile);
	assert(false);
	/*
	CComPtr<IShellLink> spiShellLink;
	strTargetPath = L"";


	// Get a pointer to the IShellLink interface
	HRESULT hr = ::CoCreateInstance(
		CLSID_ShellLink,
		NULL, 
		CLSCTX_INPROC_SERVER,
		IID_IShellLink,
		(void**)&spiShellLink); 
	if (FAILED(hr))
		return hr;


	CComPtr<IPersistFile> spiPersistFile;
	hr = spiShellLink.QueryInterface(&spiPersistFile);
	if (FAILED(hr))
		return hr;


	// Open the shortcut file and initialize it from its contents
	hr = spiPersistFile->Load(pszLnkFile, STGM_READ); 
	if (FAILED(hr))
		return hr;


	// Try to find the target of a shortcut, even if it has been moved or renamed
	hr = spiShellLink->Resolve(NULL, dwFlag);
	if (FAILED(hr))
		return hr;



	// Get the path to the shortcut target
	WIN32_FIND_DATA findData;
	hr = spiShellLink->GetPath(
		strTargetPath.GetBuffer(32768), 
		32768,
		&findData,
		SLGP_RAWPATH);
	if (FAILED(hr))
	{
		strTargetPath.ReleaseBuffer(0);
		return hr;
	}
	strTargetPath.ReleaseBuffer(-1);

	*/
	return S_OK;
}

HRESULT CWinPathApi::ResolveLnkFile(LPCWSTR pszLnkFile, CStdString& strTargetPath, DWORD dwFlag)
{
	__try
	{
		return ResolveLnkFileNoSafe(pszLnkFile, strTargetPath, dwFlag);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_POINTER;
	}
}


BOOL CWinPathApi::IsDots(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;

	if (L'.' != pszPath[0])
		return FALSE;

	if (pszPath[1] == L'\0')
	{
		return TRUE;
	}
	else if (pszPath[1] == L'.' &&
		pszPath[2] == L'\0')
	{
		return TRUE;
	}

	return FALSE;
}

// copied from source code of win2k
BOOL CWinPathApi::IsDirectory(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;


	if (IsUNCServer(pszPath))
	{
		return FALSE;
	}
	else if (IsUNCServerShare(pszPath))
	{
		union {
			NETRESOURCE nr;
			TCHAR buf[512];
		} nrb;

		LPTSTR lpSystem;
		DWORD dwRet;
		DWORD dwSize = sizeof(nrb);

		nrb.nr.dwScope = RESOURCE_GLOBALNET;
		nrb.nr.dwType = RESOURCETYPE_ANY;
		nrb.nr.dwDisplayType = 0;
		nrb.nr.lpLocalName = NULL;
		nrb.nr.lpRemoteName = (LPTSTR)pszPath;
		nrb.nr.lpProvider = NULL;
		nrb.nr.lpComment = NULL;

		dwRet = ::WNetGetResourceInformation(&nrb.nr, &nrb, &dwSize, &lpSystem);

		if (dwRet != WN_SUCCESS)
			goto TryGetFileAttrib;

		if (nrb.nr.dwDisplayType == RESOURCEDISPLAYTYPE_GENERIC)
			goto TryGetFileAttrib;

		if ((nrb.nr.dwDisplayType == RESOURCEDISPLAYTYPE_SHARE) &&
			((nrb.nr.dwType == RESOURCETYPE_ANY) ||
			(nrb.nr.dwType == RESOURCETYPE_DISK)))
		{
			return TRUE;
		}
	}
	else
	{
		DWORD dwAttribs;
TryGetFileAttrib:

		dwAttribs = ::GetFileAttributes(pszPath);
		if (INVALID_FILE_ATTRIBUTES != dwAttribs)
			return (BOOL)(dwAttribs & FILE_ATTRIBUTE_DIRECTORY);
	}

	return FALSE;
}

BOOL CWinPathApi::IsRelative(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return TRUE;    // The NULL path is assumed relative

	if (L'\\' == pszPath[0])
		return FALSE;   // Does it begin with a slash ?

	if (L'\0' != pszPath[0] && L':' == pszPath[1])
		return FALSE;   // Does it begin with a drive and a colon ?

	return TRUE;
}

BOOL CWinPathApi::IsRoot(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;

	// single '\'
	if (L'\\' == pszPath[0] && L'\0' == pszPath[1])
		return TRUE;

	if (IsUNC(pszPath))
	{
		pszPath = FindAfterAnyPrefix(pszPath);
		assert(pszPath);

		// nothing more
		if (L'\0' == pszPath[0])
			return TRUE;

		// prefix + "\"
		if (L'\\' == pszPath[0] && L'\0' == pszPath[1])
			return FALSE;

		LPCWSTR lpNextBackSlash = ::StrChrW(pszPath + 1, L'\\');
		if (!lpNextBackSlash)
			return TRUE;    // only server name

		// prefix + "\\"
		if (lpNextBackSlash == pszPath)
			return FALSE;

		// \\abc\?
		if (L'\0' == lpNextBackSlash[1])
			return FALSE;

		if (L'\\' == lpNextBackSlash[1])
			return FALSE;

		if (::StrChrW(lpNextBackSlash + 2, L'\\'))
			return FALSE; // too many backslash

		return TRUE;
	}
	else
	{   // "\\?\"
		pszPath = FindAfterUnicodePrefix(pszPath);
		assert(pszPath);
	}

	// for normal
	if (L'\0' != pszPath[0])
	{
		if (L':' == pszPath[1])
		{
			if (L'\\' == pszPath[2] && L'\0' == pszPath[3])
				return TRUE;
		}
	}


	return FALSE;
}

BOOL CWinPathApi::IsUNC(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;

	if (CWinPathApi::HasUnicodePrefix(pszPath))
	{
		pszPath += WIN_PATH_UNICODE_PATH_PREFIX;
		if (0 == StrCmpNIW(pszPath, L"UNC\\", 4))
			return TRUE;

		return FALSE;
	}

	if (L'\\' == pszPath[0])
	{
		if (L'\\' == pszPath[1])
		{
			if (L'?' != pszPath[2])
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CWinPathApi::IsUNCServer(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;

	if (!IsUNC(pszPath))
		return FALSE;

	pszPath = FindAfterAnyPrefix(pszPath);
	pszPath = ::StrChrW(pszPath, L'\\');
	if (pszPath)
		return FALSE;

	return TRUE;
}

BOOL CWinPathApi::IsUNCServerShare(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;

	if (!IsUNC(pszPath))
		return FALSE;

	pszPath = FindAfterAnyPrefix(pszPath);
	pszPath = ::StrChrW(pszPath, L'\\');
	if (!pszPath)
		return FALSE;

	pszPath = ::StrChrW(pszPath + 1, L'\\');
	if (pszPath)
		return FALSE;

	return TRUE;
}

BOOL CWinPathApi::IsFileExisting(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;

	DWORD dwAttr = ::GetFileAttributes(pszPath);
	if (INVALID_FILE_ATTRIBUTES == dwAttr)
		return FALSE;

	return TRUE;
}

BOOL CWinPathApi::IsFileNoDirExisting(LPCWSTR pszPath)
{
	if (!pszPath || !*pszPath)
		return FALSE;

	DWORD dwAttr = ::GetFileAttributes(pszPath);
	if (INVALID_FILE_ATTRIBUTES == dwAttr)
		return FALSE;
	if (FILE_ATTRIBUTE_DIRECTORY == dwAttr)
		return FALSE;

	return TRUE;
}

BOOL CWinPathApi::IsLnkFile(LPCWSTR pszPath)
{
	if (!pszPath)
		return FALSE;

	LPCWSTR lpszExtension = FindExtension(pszPath);
	if (0 != StrCmpIW(lpszExtension, L".lnk"))
		return FALSE;

	return TRUE;
}


void CWinPath::AddBackslash()
{
	if (m_strPath.IsEmpty())
		return;

	int nLen = m_strPath.GetLength();
	if (L'\\' == m_strPath[nLen - 1])
		return;  

	m_strPath.AppendFormat(L"\\");
}


BOOL CWinPath::AddExtension(LPCWSTR pszExtension)
{
	LPCWSTR lpOldExtension = CWinPathApi::FindExtension(m_strPath);
	assert(lpOldExtension);
	if (*lpOldExtension)
		return FALSE;   // extension already exist

	if (pszExtension == NULL)
		pszExtension = L".exe";

	m_strPath.AppendFormat(pszExtension);
	return TRUE;
}

BOOL CWinPath::Append(LPCWSTR pszMore)
{
	if (!pszMore)
		return FALSE;

	CWinPath pathResult;
	pathResult.Combine(m_strPath, pszMore);

	m_strPath = pathResult.m_strPath;
	return TRUE;
}

void CWinPath::BuildRoot(int iDrive)
{
	if (iDrive >= 0 && iDrive <= 26)
	{
		m_strPath.Format(L"%c:\\",(WCHAR)(L'A' + (WCHAR)iDrive));
	}
}
//
//void CWinPath::Canonicalize()
//{
//    WStdString strResult;
//    LPCWSTR lpszSource = (LPCWSTR)m_strPath;
//
//    // copy prefix
//    LPCWSTR lpAfterPrefix = CWinPathApi::FindAfterAnyPrefix(m_strPath);
//    if (lpszSource != lpAfterPrefix)
//    {
//        strResult.AppendFormat(lpszSource, int(lpAfterPrefix - lpszSource));
//        lpszSource = lpAfterPrefix;
//    }
//
//
//    while (lpszSource[0])
//    {
//        LPCWSTR lpPathNodeEnd = StrChrW(lpszSource, L'\\');
//        if (!lpPathNodeEnd)
//        {
//            lpPathNodeEnd = (LPCWSTR)m_strPath + m_strPath.GetLength();
//        }
//
//        int cbPathNodeCount = int(lpPathNodeEnd - lpszSource);
//
//        if (1 == cbPathNodeCount && L'.' == lpszSource[0])    
//        {   // single dot
//            lpszSource = lpPathNodeEnd;
//
//            // skip backslash
//            if (L'\\' == lpszSource[0])
//                ++lpszSource;
//        }
//        else if (2 == cbPathNodeCount && L'.' == lpszSource[0] && L'.' == lpszSource[1])
//        {
//            if (!CWinPathApi::IsRoot(strResult))
//            {
//                int nPos = strResult.ReverseFind(L'\\');
//                if (-1 != nPos)
//                    strResult.Left(nPos + 1);
//            }
//
//            lpszSource = lpPathNodeEnd;
//
//            // skip backslash
//            if (L'\\' == lpszSource[0])
//                ++lpszSource;
//        }
//        else
//        {
//            strResult.AppendFormat(lpszSource, cbPathNodeCount);
//            lpszSource = lpPathNodeEnd;
//
//            if (L'\0' != lpszSource[0])
//            {
//                strResult.AppendFormat(lpszSource[0]);
//                ++lpszSource;
//            }
//        }
//    }
//
//
//    
//    int nLen = strResult.GetLength();
//    if (strResult.IsEmpty())
//    {   // empty path
//        strResult = L'\\';
//    }
//    else if (L':' == strResult[nLen - 1])
//    {
//        LPCWSTR lpNewAfterPrefix = CWinPathApi::FindAfterAnyPrefix(strResult);
//        if (L'\0' != lpNewAfterPrefix[0] && 
//            L':'  == lpNewAfterPrefix[1] &&
//            L'\0' == lpNewAfterPrefix[2])
//        {   // prefix + L"X:"
//            strResult.AppendFormat(L'\\');
//        }
//    }
//    else if (L'.' == strResult[nLen - 1])
//    {
//        while (nLen && L'.' == strResult[nLen - 1])
//        {
//            --nLen;
//        }
//
//        strResult.Left(nLen);
//    }
//
//    m_strPath = strResult;
//}



void CWinPath::Combine(LPCWSTR pszDir, LPCWSTR pszFile)
{
	if (!pszDir && !pszFile)
		return;

	if (!pszFile)
	{
		m_strPath = pszDir;
		return;
	}

	while (*pszFile && L'\\' == *pszFile)
	{
		++pszFile;
	}
	if (!*pszFile)
	{
		m_strPath = pszDir;
		return;
	}

	CStdString strResult = pszDir;
	while (!strResult.IsEmpty() && L'\\' == strResult[strResult.GetLength() - 1])
	{
		strResult.Left(strResult.GetLength() - 1);
	}
	strResult.AppendFormat(L"\\");
	strResult.AppendFormat(pszFile);


	m_strPath = strResult;
}

BOOL CWinPath::CompactPathEx(UINT nMaxChars, DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(dwFlags);
	if (m_strPath.IsEmpty())
		return TRUE;


	if (0 == nMaxChars)
	{
		m_strPath.Empty();
		return TRUE;
	}


	UINT uMaxLength = nMaxChars - 1;


	CStdString strResult;
	if (m_strPath.GetLength() < (int)uMaxLength)
	{
		return TRUE;
	}


	LPCWSTR pszFileName = CWinPathApi::FindFileName(m_strPath);
	UINT    uiFNLen     = UINT((LPCWSTR)m_strPath + m_strPath.GetLength() - pszFileName);


	// if the whole string is a file name
	if(pszFileName == (LPCWSTR)m_strPath && uMaxLength > LEN_END_ELLIPSES)
	{
		strResult = m_strPath.Left(uMaxLength - LEN_END_ELLIPSES);
		//		strResult.assign(m_strPath, uMaxLength - LEN_END_ELLIPSES);
		strResult.AppendFormat(L"...");
		
		assert(0 == nMaxChars || strResult.GetLength() == int(uMaxLength));
		m_strPath = strResult;
		return TRUE;
	}


	// Handle all the cases where we just use ellipses ie '.' to '.../...'
	if ((uMaxLength <= MIN_CCHMAX))
	{
		strResult = L"";
		for (int n = 0; n < (int)uMaxLength - 1; ++n)
		{
			if ((n + 1) == LEN_MID_ELLIPSES)
			{
				strResult.AppendFormat(L"\\");
			}
			else
			{
				strResult.AppendFormat(L".");
			}
		}

		assert(0 == nMaxChars || strResult.GetLength() == int(uMaxLength));
		m_strPath = strResult;
		return TRUE;
	}

	// Ok, how much of the path can we copy ? Buffer - (Length of MID_ELLIPSES + Len_Filename)
	int cchToCopy = uMaxLength - (LEN_MID_ELLIPSES + uiFNLen);

	if (cchToCopy < 0)
		cchToCopy = 0;

	strResult = m_strPath.Left(cchToCopy);
	// Now throw in the ".../" or "...\"
	strResult.AppendFormat(L"...\\");

	//Finally the filename and ellipses if necessary
	if (uMaxLength > (LEN_MID_ELLIPSES + uiFNLen))
	{
		strResult.AppendFormat(pszFileName);
	}
	else
	{
		cchToCopy = uMaxLength - LEN_MID_ELLIPSES - LEN_END_ELLIPSES;

		strResult.AppendFormat(pszFileName, cchToCopy);
		strResult.AppendFormat(L"...");
	}

	assert(0 == nMaxChars || strResult.GetLength() == int(uMaxLength));
	m_strPath = strResult;
	return TRUE;
}

int CWinPath::FindExtension() const
{
	LPCWSTR lpszBuffer    = (LPCWSTR)m_strPath;
	LPCWSTR lpszExtension = NULL;

	lpszExtension = CWinPathApi::FindExtension(lpszBuffer);
	if(*lpszExtension == 0)
		return -1;
	else
		return int(lpszExtension - lpszBuffer);
}

int CWinPath::FindFileName() const
{
	LPCWSTR lpFound = CWinPathApi::FindFileName((LPCWSTR)m_strPath);
	if (!lpFound || !*lpFound)
		return -1;

	return int(lpFound - (LPCWSTR)m_strPath);
}

CStdString CWinPath::GetExtension() const
{
	return CWinPathApi::FindExtension(m_strPath);
}

BOOL CWinPath::IsDirectory() const
{
	return CWinPathApi::IsDirectory(m_strPath);
}

BOOL CWinPath::IsRelative() const
{
	return CWinPathApi::IsRelative(m_strPath);
}

BOOL CWinPath::IsRoot() const
{
	return CWinPathApi::IsRoot(m_strPath);
}

BOOL CWinPath::IsUNC() const
{
	return CWinPathApi::IsUNC(m_strPath);
}

BOOL CWinPath::IsUNCServer() const
{
	return CWinPathApi::IsUNCServer(m_strPath);
}

BOOL CWinPath::IsUNCServerShare() const
{
	return CWinPathApi::IsUNCServerShare(m_strPath);
}

void CWinPath::RemoveArgs()
{
	for (int i = 0; i < m_strPath.GetLength(); ++i)
	{
		switch (m_strPath[i])
		{
		case L'\"':
			return;
		case L' ' :
			m_strPath.Left(i + 1);
			m_strPath.TrimRight();
			return;
		}
	}
}

void CWinPath::RemoveBackslash()
{
	if (m_strPath.IsEmpty())
		return;

	if (IsRoot())
		return;

	int nLen = m_strPath.GetLength();
	if (L'\\' == m_strPath[nLen - 1])
		m_strPath.Left(nLen - 1);
}

void CWinPath::RemoveExtension()
{
	int nPos = FindExtension();
	if (-1 == nPos)
		return;

	m_strPath.Left(nPos);
	m_strPath.TrimRight();
}

BOOL CWinPath::RemoveFileSpec()
{
	if (m_strPath.IsEmpty())
		return FALSE;


	LPCWSTR lpszPathBegin = CWinPathApi::FindAfterAnyPrefix(m_strPath);
	LPCWSTR lpszPathEnd   = (LPCWSTR)m_strPath + m_strPath.GetLength();
	assert(lpszPathBegin <= lpszPathEnd);


	// find after root
	if (L'\0' == lpszPathBegin[0])
	{
		NULL;
	}
	else if (L'\\' == lpszPathBegin[0] && L':' == lpszPathBegin[1])
	{
		if (L'\\' == lpszPathBegin[2])
		{
			if (L'\0' == lpszPathBegin[3])
			{
				lpszPathBegin += 2;     // prefix + "\:\"       <- strange implement in vista
			}
			else
			{
				lpszPathBegin += 3;     // go through "\:\"     <- strange implement in vista
			}
		}
	}
	else if (L':' == lpszPathBegin[1])
	{
		if (L'\0' == lpszPathBegin[2])
		{
			return FALSE;               // prefix + "C:"
		}
		else if (L'\\' == lpszPathBegin[2])
		{
			if (L'\0' == lpszPathBegin[3])
			{
				return FALSE;           // prefix + "C:\"
			}
			else
			{
				lpszPathBegin += 3;     // go through "C:\"
			}
		}
		else
		{

			lpszPathBegin += 2;         // go through "C:"
		}
	}


	LPCWSTR lpszRemoveBegin = StrRChrW(lpszPathBegin, lpszPathEnd, L'\\');
	if (!lpszRemoveBegin)
	{
		lpszRemoveBegin = lpszPathBegin;
	}
	else if ((LPCWSTR)m_strPath == lpszRemoveBegin)
	{
		++lpszRemoveBegin;      // "\" + others
	}
	else if (lpszPathBegin < lpszRemoveBegin &&
		'\\' == *(lpszRemoveBegin - 1))
	{
		--lpszRemoveBegin;      // ..."\\"...   eat 2 backslash
	}


	if (L'\0' == lpszRemoveBegin)
		return FALSE;   // stripped nothing


	int nOldLength = m_strPath.GetLength();
	int nNewLength = int(lpszRemoveBegin - m_strPath);
	if (nOldLength == nNewLength)
		return FALSE;


	m_strPath.Left(nNewLength);
	return TRUE;
}

void CWinPath::StripPath()
{
	LPCWSTR lpFound = CWinPathApi::FindFileName(m_strPath);
	m_strPath = lpFound;
}

BOOL CWinPath::StripToRoot()
{
	// for vista
	if (L'\\' == m_strPath[0] &&
		L'\0' == m_strPath[1])
	{
		return TRUE;        // "\"
	}


	LPCWSTR lpAfterAnyPrefix = CWinPathApi::FindAfterAnyPrefix(m_strPath);
	assert(lpAfterAnyPrefix);


	// for vista
	if (L'\0' == lpAfterAnyPrefix[0])
	{
		NULL;
	}
	else if (L'\\' == lpAfterAnyPrefix[0] && L':' == lpAfterAnyPrefix[1])
	{   // strange implement in vista
		m_strPath.Left(int(lpAfterAnyPrefix + 1 - (LPCWSTR)m_strPath));
		return TRUE;            // prefix + "\:" + others
	}
	else if (L':' == lpAfterAnyPrefix[1])
	{
		if (L'\\' == lpAfterAnyPrefix[2])
		{
			m_strPath.Left(int(lpAfterAnyPrefix + 3 - (LPCWSTR)m_strPath));
			return TRUE;        // prefix + "X:\" + others
		}
		else
		{
			m_strPath.Left(int(lpAfterAnyPrefix + 2 - (LPCWSTR)m_strPath));
			return TRUE;        // prefix + "X:" + others
		}
	}


	if (IsUNC())
	{
		int nFirstBackSlashAfterUNC = m_strPath.Find(L'\\', int(lpAfterAnyPrefix - (LPCWSTR)m_strPath));
		if (-1 == nFirstBackSlashAfterUNC)
			return TRUE;        // "\\Server"


		if (L'\0' == m_strPath[nFirstBackSlashAfterUNC + 1])
		{
			m_strPath.Left(nFirstBackSlashAfterUNC);
			return TRUE;        // "\\Server\"
		}


		int nSecondBackSlashAfterUNC = m_strPath.Find(L'\\', nFirstBackSlashAfterUNC + 1);
		if (-1 == nSecondBackSlashAfterUNC)
			return TRUE;        // "\\Server\\foo"


		if (nSecondBackSlashAfterUNC == nFirstBackSlashAfterUNC + 1)
		{
			m_strPath.Left(nFirstBackSlashAfterUNC);
			return TRUE;        // "\\Server\\\\foo"
		}


		m_strPath.Left(nSecondBackSlashAfterUNC);
		return TRUE;
	}

	while(!IsRoot())
	{
		if (!RemoveFileSpec())
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CWinPath::UnquoteSpaces()
{
	m_strPath.Trim();

	if (m_strPath.GetLength() < 2)
		return;

	if (L'\"' == m_strPath[m_strPath.GetLength() - 1] && L'\"' == m_strPath[0])
	{
		m_strPath.Left(m_strPath.GetLength() - 1);
		m_strPath = (LPCWSTR)m_strPath + 1;
	}
}

BOOL CWinPath::IsExisting() const
{
	return CWinPathApi::IsFileExisting(m_strPath);
}

void CWinPath::RemoveSingleArg()
{
	m_strPath.Trim();
	for (int i = m_strPath.GetLength(); i > 0; --i)
	{
		switch (m_strPath[i - 1])
		{
		case L'\"':
			return;
		case L' ':
			m_strPath.Left(i);
			m_strPath.TrimRight();
			return;
		}
	}
}

BOOL CWinPath::HasUnicodePrefix() const
{
	return CWinPathApi::HasUnicodePrefix(m_strPath);
}

void CWinPath::RemoveUnicodePrefix()
{
	if (CWinPathApi::HasUnicodePrefix(m_strPath))
	{
		LPCWSTR lpszAfterUnicodePrefix = (LPCWSTR)m_strPath + CWinPathApi::WIN_PATH_UNICODE_PATH_PREFIX;
		if (0 == StrCmpNIW(lpszAfterUnicodePrefix, L"UNC\\", 4))
		{   // remove 'UN', and set 'C' as '\\'
			m_strPath.SetAt(CWinPathApi::WIN_PATH_UNICODE_PATH_PREFIX + 2, L'\\');
			m_strPath = lpszAfterUnicodePrefix + 2;
		}
		else
		{
			m_strPath = lpszAfterUnicodePrefix;
		}
	}
}

void CWinPath::AddUnicodePrefix()
{
	if (HasUnicodePrefix())
		return;

	CStdString strResult;
	if (IsUNC())
	{
		assert(m_strPath.GetLength() >= 2);
		strResult = L"\\\\?\\UNC\\";
		strResult.AppendFormat((LPCWSTR)m_strPath + 2);
	}
	else
	{
		strResult = L"\\\\?\\";
		ExpandLongPathName();
		strResult.AppendFormat(m_strPath);
	}

	m_strPath = strResult;
}

CWinPath CWinPath::GetPathWithoutUnicodePrefix() const
{
	if (!HasUnicodePrefix())
		return (LPCWSTR)m_strPath;

	CWinPath pathResult = (LPCWSTR)m_strPath;
	pathResult.RemoveUnicodePrefix();
	return pathResult;
}

HRESULT CWinPath::ExpandFullPathName()
{
	return CWinPathApi::ExpandFullPathName(m_strPath);
}

HRESULT CWinPath::ExpandLongPathName()
{
	return CWinPathApi::ExpandLongPathName(m_strPath);
}

BOOL CWinPath::ExpandAsAccessiblePath()
{
	return CWinPathApi::ExpandAsAccessiblePath(m_strPath);
}

DWORD CWinPath::GetModuleFileName(HMODULE hModule, DWORD dwMaxSize)
{
	DWORD dwRet = ::GetModuleFileName(hModule, m_strPath.GetBuffer(dwMaxSize + 1), dwMaxSize);
	dwRet = min(dwRet, dwMaxSize);
	m_strPath.ReleaseBuffer(dwRet);

	return dwRet;
}




//////////////////////////////////////////////////////////////////////
//	Implemented by Samuel Gonzalo 
//
//	You may freely use or modify this code 
//////////////////////////////////////////////////////////////////////
//
// Path.cpp: implementation of the CPath class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPath::CPath()
{
	_bIsRelative = FALSE;
}

CPath::CPath(LPCTSTR szPath, BOOL bIsFolderPath, BOOL bHasArguments)
{
	SetPath(szPath, bIsFolderPath, bHasArguments);
}

CPath::CPath(DWORD dwSpecial)
{
	SetPath(dwSpecial);
}

CPath::~CPath()
{

}

void CPath::SetPath(DWORD dwSpecial)
{
	switch (dwSpecial)
	{
	case PATH_CMDLINE:
		SetPath(GetCommandLine(), FALSE, TRUE);
		break;

	case PATH_MODULE:
		{
			TCHAR szPath[_MAX_PATH];
			::GetModuleFileName(0, szPath, _MAX_PATH);
			SetPath(szPath);
		}
		break;
	}
}

void CPath::SetPath(LPCTSTR szPath, BOOL bIsFolderPath, BOOL bHasArguments)
{
	TCHAR szParamPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME], szExt[_MAX_EXT];

	// Reset
	_sOriginalPath.Empty();
	_sDriveLabel.Empty();
	_bIsRelative = FALSE;
	_aDir.clear();
	_sExtName.Empty();
	_aArgs.clear();

	// Original path
	_sOriginalPath = szPath;

	// Get args and remove them from path
	szParamPath[0] = 0x0;
	StringCchCopy(szParamPath,_MAX_PATH, szPath);

	if (bHasArguments)
	{
		_sArgs = PathGetArgs(szParamPath);
		PathRemoveArgs(szParamPath);
	}

	PathUnquoteSpaces(szParamPath);
	if (szParamPath[0] == 0x0) return;

	_tsplitpath_s(szParamPath, szDrive, _MAX_DRIVE,szDir,_MAX_DIR, szName,_MAX_FNAME, szExt,_MAX_EXT);

	// Drive
	_sDrive = szDrive;

	// Directory
	_sDir = szDir;
	_sDir.Replace(L'/', L'\\');
	if (!_sDir.IsEmpty()) _bIsRelative = (_sDir[0] != L'\\');

	// FileTitle
	if (bIsFolderPath)
	{
		_sDir = CPath::AddBackSlash(_sDir);
		_sDir += szName;
		_sDir = CPath::AddBackSlash(_sDir);
	}
	else
	{
		_sFileTitle = szName;
	}

	// Get extension name (e.g.: "txt")
	if (IsFilePath())
	{
		_sExtName = szExt;
		_sExtName.Remove(L'.');
	}
}

BOOL CPath::IsLocalPath()
{
	return !_sDrive.IsEmpty() && !_bIsRelative;
}

BOOL CPath::IsRelativePath()
{
	return _bIsRelative;
}

BOOL CPath::IsFilePath()
{
	return !_sFileTitle.IsEmpty();
}

BOOL CPath::ExistFile()
{
	if (!IsFilePath()) return FALSE;

	return PathFileExists(GetPath());
}

BOOL CPath::ExistLocation()
{
	return PathFileExists(GetLocation());
}

CStdString CPath::GetAbsolutePath(LPCTSTR szBaseFolder)
{
	if (!IsRelativePath()) return sCEmptyString;

	TCHAR	szAbsolutePath[_MAX_PATH];
	CStdString sFullPath(szBaseFolder);

	if (sFullPath.IsEmpty()) return sCEmptyString;

	sFullPath = CPath::AddBackSlash(sFullPath);
	sFullPath += GetPath();

	if (!PathCanonicalize(szAbsolutePath, sFullPath)) return sCEmptyString;

	return szAbsolutePath;
}

CStdString CPath::GetRelativePath(LPCTSTR szBaseFolder)
{
	if (IsRelativePath()) return sCEmptyString;

	TCHAR	szRelativePath[_MAX_PATH];
	CStdString	sRelPath;

	PathRelativePathTo(szRelativePath, szBaseFolder, FILE_ATTRIBUTE_DIRECTORY, 
		GetPath(), IsFilePath() ? 0 : FILE_ATTRIBUTE_DIRECTORY);

	sRelPath = szRelativePath;
	if (sRelPath.GetLength() > 1)
	{
		// Remove ".\" from the beginning
		if ((sRelPath[0] == L'.') && (sRelPath[1] == L'\\'))
			sRelPath.Right(sRelPath.GetLength() - 2);
	}

	return sRelPath;
}

CStdString CPath::GetPath(BOOL bAppendFormatArgs, BOOL bOriginal)
{
	CStdString sPath;

	if (bOriginal)
		sPath = _sOriginalPath;
	else
		sPath = GetLocation() + GetFileName();

	if (bAppendFormatArgs) sPath += GetArgument();

	return sPath;
}

CStdString	CPath::GetShortPath()
{
	TCHAR szShortPath[_MAX_PATH];
	szShortPath[0] = 0x0;

	GetShortPathName(GetPath(), szShortPath, _MAX_PATH);

	return szShortPath;
}

CStdString	CPath::GetLongPath()
{
	TCHAR szLongPath[_MAX_PATH];
	szLongPath[0] = 0x0;

	GetLongPathName(GetPath(), szLongPath, _MAX_PATH);

	return szLongPath;
}

CStdString CPath::GetDrive()
{
	return _sDrive;
}

CStdString	CPath::GetDriveLabel(BOOL bPCNameIfNetwork)
{
	if (_sDriveLabel.IsEmpty() && !IsRelativePath())
	{
		if ((bPCNameIfNetwork) && (!IsLocalPath()))
			_sDriveLabel = GetDir(0);
		else
		{
			CStdString sRoot;
			TCHAR	szVolumeName[256];

			szVolumeName[0] = L'\0';
			if (IsLocalPath())
			{
				sRoot = _sDrive + CStdString(L"\\");
			}
			else if (GetDirCount() > 1)
			{
				sRoot.Format(L"\\\\%s\\%s\\", GetDir(0), GetDir(1));
			}

			GetVolumeInformation(sRoot, szVolumeName, 255, NULL, NULL, NULL, NULL, 0);

			_sDriveLabel = szVolumeName;
		}
	}

	return _sDriveLabel;
}

int	CPath::GetDirCount()
{
	FillDirArray();
	return _aDir.size();
}

CStdString CPath::GetDir(int nIndex)
{
	if (nIndex < 0)
		return _sDir;
	else if (nIndex < GetDirCount())
	{
		FillDirArray();
		return _aDir[nIndex];
	}

	return sCEmptyString;
}

CStdString	CPath::GetLocation()
{
	return _sDrive + GetDir();
}

CStdString CPath::GetFileTitlex()
{
	return _sFileTitle;
}

CStdString CPath::GetFileName()
{
	return _sFileTitle + GetExtension();
}

CStdString CPath::GetExtension()
{
	if (_sExtName.IsEmpty()) return sCEmptyString;

	return CStdString(".") + _sExtName;
}

CStdString CPath::GetExtName()
{
	return _sExtName;
}

int CPath::GetArgCount()
{
	FillArgArray();
	return _aArgs.size();
}

CStdString CPath::GetArgument(int nIndex, BOOL bGetFlag)
{
	if (nIndex < 0)
	{
		if (_sArgs.IsEmpty())
		{
			for (unsigned int nItem = 0; nItem < _aArgs.size(); nItem++)
				_sArgs += _aArgs[nItem].GetString();

			_sArgs.TrimLeft();
		}

		return _sArgs;
	}
	else if (nIndex < GetArgCount())
	{
		FillArgArray();

		if (bGetFlag)
			return _aArgs[nIndex].sFlag;
		else
			return _aArgs[nIndex].sValue;
	}

	return sCEmptyString;
}

void CPath::SetArguments(LPCTSTR szArgs)
{
	_aArgs.clear();
	_sArgs = szArgs;
}

void CPath::AddSetArgument(LPCTSTR szFlag, LPCTSTR szArgument)
{
	int	nIndex;

	nIndex = FindArgument(szFlag);

	if (nIndex != -1)
	{
		// An argument with the same flag already exists. Update it!
		_aArgs[nIndex].sValue = szArgument;
	}
	else
	{
		CArgument arg;

		arg.sValue = szArgument;
		arg.SetFlag(szFlag);

		_aArgs.push_back(arg);
		_sArgs.Empty();
	}
}

void CPath::RemoveArgument(int nIndex)
{
	if ((nIndex >= 0) && (nIndex < _aArgs.size()))
	{
		_aArgs.erase(_aArgs.begin() +nIndex);
		_sArgs.Empty();
	}
}

int CPath::FindArgument(LPCTSTR szFlag)
{
	bool		bFound = false;
	unsigned int	nIndex;
	CArgument	arg;

	FillArgArray();

	arg.SetFlag(szFlag);

	if (!arg.sFlag.IsEmpty())
	{
		for (nIndex = 0; nIndex < _aArgs.size(); nIndex++)
		{
			bFound = (_aArgs[nIndex].sFlag.CompareNoCase(arg.sFlag) == 0);
			if (bFound) break;
		}
	}

	return (bFound ? nIndex : -1);
}

BOOL CPath::GetFileSize(__int64 &nSize)
{
	BOOL bResult;

	bResult = FillFileInfoStruct();
	nSize = ((__int64)_fis.nFileSizeHigh * (__int64)MAXDWORD) + (__int64)_fis.nFileSizeLow;
	return bResult;
}

BOOL CPath::GetFileTime(QTime &time, DWORD dwType)
{
	BOOL bResult;
	FILETIME *pTime = NULL;

	bResult = FillFileInfoStruct();
	switch (dwType)
	{
	case FILE_CREATION:	pTime = &_fis.ftCreationTime;	break;
	case FILE_WRITE:	pTime = &_fis.ftLastWriteTime;	break;
	case FILE_ACCESS:	
	default:			pTime = &_fis.ftLastAccessTime;	break;
	}

	if (pTime != NULL) time = QTime(*pTime);
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Private methods

// This function must be called whenever _aDir array is needed, since this
// method is the one which parses _sDir and fill _aDir
void CPath::FillDirArray()
{
	if (_sDir.IsEmpty() || (_aDir.size() > 0)) return;

	int nFrom, nTo;

	// nFrom: 0 - relative / 1 - local / 2 - network
	nFrom = IsLocalPath() ? 1 : (IsRelativePath() ? 0 : 2);

	while ((nTo = _sDir.Find('\\', nFrom)) != -1)
	{
		_aDir.push_back(_sDir.Mid(nFrom, nTo - nFrom));
		nFrom = nTo + 1;
	}
}

// This function must be called whenever _aArgs array is needed, since this
// method is the one which parses _sArgs and fill _aArgs
void CPath::FillArgArray()
{
	if (_sArgs.IsEmpty() || (_aArgs.size() > 0)) return;
	
	CStdString		sArgs(_sArgs);
	int			nFrom, nTo;
	bool		bQuotedArg = false;
	CArgument	arg;

	sArgs.TrimLeft();
	sArgs.TrimRight();

	// add a blank space at the end of the string to include the last argument
	sArgs += " ";
	while ((nTo = sArgs.FindOneOf(L" \"")) != -1)
	{
		bQuotedArg = (sArgs[nTo] == L'\"');

		if (bQuotedArg)
		{
			nFrom = nTo + 1;
			if ((nTo = sArgs.Find(L'\"', nFrom)) == -1) break;

			arg.sValue = sArgs.Mid(nFrom, nTo - nFrom);
		}
		else
		{
			CStdString *pStr;

			if ((sArgs[0] == L'/') || (sArgs[0] == L'-'))
			{
				if (!arg.sFlag.IsEmpty())
				{
					arg.SetFlag(arg.sFlag);
					_aArgs.push_back(arg);
					arg.sFlag.Empty();
					arg.sValue.Empty();
				}

				pStr = &arg.sFlag;
			}
			else
				pStr = &arg.sValue;

			*pStr = sArgs.Left(nTo);
		}

		if (!arg.sValue.IsEmpty())
		{
			arg.SetFlag(arg.sFlag);
			_aArgs.push_back(arg);
			arg.sFlag.Empty();
			arg.sValue.Empty();
		}

		sArgs = sArgs.Right(sArgs.GetLength() - nTo - 1);
		sArgs.TrimLeft();
	}

	// if the last argument is only a flag it hasn't been added yet
	if (!arg.sFlag.IsEmpty())
	{
		arg.SetFlag(arg.sFlag);
		_aArgs.push_back(arg);
	}
}

BOOL CPath::FillFileInfoStruct()
{
	HANDLE	hFile;
	BOOL	bResult;

	::memset(&_fis, 0, sizeof(_fis));

	hFile = CreateFile(GetPath(), GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN |
		FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM, NULL);

	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	bResult = GetFileInformationByHandle(hFile, &_fis);

	CloseHandle(hFile);

	return bResult;
}

CStdString CPath::AddBackSlash(LPCTSTR szFolderPath, BOOL bInverted)
{
	CStdString	sResult(szFolderPath);
	int		nLastChar = sResult.GetLength() - 1;

	if (nLastChar >= 0)
	{
		if ((sResult[nLastChar] != '\\') && (sResult[nLastChar] != '/'))
			sResult += bInverted ? '/' : '\\';
	}

	return sResult;
}

CStdString CPath::RemoveBackSlash(LPCTSTR szFolderPath)
{
	CStdString	sResult(szFolderPath);
	int		nLastChar = sResult.GetLength() - 1;

	if (nLastChar >= 0)
	{
		if ((sResult[nLastChar] == '\\') || (sResult[nLastChar] == '/'))
			sResult = sResult.Left(nLastChar);
	}

	return sResult;
}

CPath::operator LPCTSTR ()
{
	_sLPCTSTRPath = GetPath();
	return _sLPCTSTRPath;
}

const CPath& CPath::operator = (LPCTSTR szPath)
{
	SetPath(szPath);
	return *this;
}

const CPath& CPath::operator = (CPath &ref)
{
	_aArgs.clear();
	_aArgs = ref._aArgs;
	_aDir.clear();
	_aDir = ref._aDir;

	_bIsRelative = ref._bIsRelative;

	_fis = ref._fis;

	_sArgs = ref._sArgs;
	_sDir = ref._sDir;
	_sDrive = ref._sDrive;
	_sDriveLabel = ref._sDriveLabel;
	_sExtName = ref._sExtName;
	_sFileTitle = ref._sFileTitle;
	_sOriginalPath = ref._sOriginalPath;
	return *this;
}


// ==================================================================
// 
//  Path.cpp   
//  
//  Created:       20.06.2004
//
//  Copyright (C) Peter Hauptmann
//              
// ------------------------------------------------------------------
/// 
/// for copyright & disclaimer see accompanying Path.h
/// 
/// \page pgChangeLog Change Log
/// 
/// June 20, 2004:  Initial release
/// 
/// June 22, 2004   
///     - \c fixed: quibase::CXShellPath::MakeSystemFolder implements unmake correctly 
///     - \c added: quibase::CXShellPath::MakeSystemFolder and quibase::CXShellPath::SearchOnPath 
///       set the windows error code to zero if the function succeeds (thanks Hans Dietrich)
///     - \c fixed: quibase::CXShellPath compiles correctly with warning level -W4
/// 
/// Mar 3, 2005
///     - fixed eppAutoQuote bug in GetStr (thanks Stlan)
///     - Added: 
///         - \ref quibase::FromRegistry "FromRegistry"
///         - \ref quibase::CXShellPath::ToRegistry "ToRegistry"
///         - \ref quibase::CXShellPath::GetRootType, "GetRootType"
///         - \ref quibase::CXShellPath::GetRoot "GetRoot" has a new implementation
///         - \ref quibase::CXShellPath::MakeAbsolute "MakeAbsolute"
///         - \ref quibase::CXShellPath::MakeRelative "MakeRelative"
///         - \ref quibase::CXShellPath::MakeFullPath "MakeFullPath"
///         - \ref quibase::CXShellPath::EnvUnexpandRoot "EnvUnexpandRoot"
///         - \ref quibase::CXShellPath::EnvUnexpandDefaultRoots "EnvUnexpandDefaultRoots"
///     - \b Breaking \b Changes (sorry)
///         - GetRoot -> ShellGetRoot (to distinct from extended GetRoot implementation)
///         - GetFileName --> GetName (consistency)
///         - GetFileTitle --> GetTitle (consistency)
///         - made the creation functions independent functions in the nsPath namespace
///           (they are well tugged away in the namespace so conflicts are easy to avoid) 
/// 
/// Mar 17, 2005
///     - fixed bug in GetFileName (now: GetName): If the path ends in a backslash,
///       GetFileName did return the entire path instead of an empty string. (thanks woodland)
/// 
/// Aug 21, 2005
///     - fixed bug in GetStr(): re-quoting wasn't applied (doh!)
///     - fixed incompatibility with CStlString
///     - Added IsDot, IsDotDot, IsDotty (better names?)
///     - Added IsValid, ReplaceInvalid
///     - Added SplitRoot
///     - 
///        
/// 
///
// ------ Main Page --------------------
/// @mainpage
/// 
/// \ref pgDisclaimer, \ref pgChangeLog (recent changes August 2005, \b breaking \b changes Mar 2005)
///
///	\par Introduction
///
/// \ref quibase::CXShellPath "CXShellPath" is a helper class to make manipulating file system path strings easier. 
/// It is complementedby a few non-class functions (see nsPath namespace documentation)
/// 
/// CXShellPath is based on the Shell Lightweight Utility API, but modifies / and extends this functionality
/// (and removes some quirks). It requires CString (see below why, and why this is not too bad).
/// 
/// \par Main Features:
/// 
/// CXShellPath acts as a string class with special "understanding" and operations for path strings.
/// 
/// \code CXShellPath path("c:\\temp"); \endcode
/// constructs a path string, and does some default cleanup (trimming white space, removing quotes, etc.)
/// The cleanup can be customized (see \ref quibase::CXShellPath::CXShellPath "CXShellPath Constructor", 
///  \ref quibase::EPathCleanup "EPathCleanup"). You can pass an ANSI or UNICODE string.
/// 
/// \code path = path & "file.txt"; \endcode
/// AppendFormats "foo" to the path, making sure that the two segments are separated by exactly one backslash
/// no matter if the parts end/begin with a backslash.
/// 
/// The following functions give you access to the individual elements of the path:
/// 
///  - \ref quibase::CXShellPath::GetRoot "GetRoot"
///  - \ref quibase::CXShellPath::GetPath "GetPath"
///  - \ref quibase::CXShellPath::GetName "GetName"
///  - \ref quibase::CXShellPath::GetTitle "GetTitle"
///  - \ref quibase::CXShellPath::GetExtension "GetExtension"
/// 
/// \code CString s = path.GetStr() \endcode
/// returns a CString that is cleaned up again (re-quoted if necessary, etc). GetBStr() returns an _bstr_t
/// with the same features (that automatically casts to either ANSI or UNICODE).
/// To retrieve the unmodified CXShellPath string, you can rely on the \c operator \c LPCTSTR 
/// 
/// There's much more - see the full nsPath documentation for details!
/// 
///	@sa MSDN library: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/shlwapi/path/pathAppendFormat.asp
/// 
/// \par Why not CPathT ?
///  -# the class is intended for a VC6 project that won't move to VC7 to soon
///  -# CPathT contains the same quirks that made me almost give up on the Shell Helper functions. 
///  -# I wanted the class to have additional features (such as the & operator, and automatic cleanup)
/// 
/// \par Why CString ?
///  -# The CString implementation provides a known performance (due to the guaranteed reference counted "copy 
///     on write" implementation). I consider this preferrable over the weaker guarantees made b STL, especially
///     when designing a  "convenient" class interface.
///  -# CString's ref counting mechanism is automatically reused by CXShellPath, constructing a CXShellPath from a CString
///     does not involve a copy operation.
///  -# CString is widely availble independent of MFC (WTL, custom implementations, "extract" macros are
///     available, and VC7 makes CString part of the ATL)
/// 
/// \note if you want to port to STL, it's probably easier to use a vector<TCHAR> instead of std:string 
///   to hold the data internally
/// 
/// \par Why _bstr_t ?
/// To make implementation easier, the class internally works with "application native" strings (that is, 
/// TCHAR strings - which are either ANSI or UNICODE depending on a compile setting). GetBStr provides
/// conversion to ANSI or UNICODE, whichever is required.\n
/// An independent implementation would return a temporary object with cast operators to LPCSTR and LPWSTR 
/// - BUT _bstr_t does exactly that (admittedly, with some overhead). 
/// 

	/// contains helper classes for nsPath namespace
	/// 
	namespace nsDetail
	{
		// ==================================================================
		//  CStringLock
		// ------------------------------------------------------------------
		// 
		// Helper class for CString::GetBuffer and CString::ReleaseBuffer
		// \todo: consider moving to common utility 
		// \todo: additional debug verification on release
		//
		class CStringLock
		{
		public:
			CStdString * m_string;
			LPTSTR    m_buffer;

			static LPTSTR NullBuffer;

		public:
			CStringLock(CStdString & s) : m_string(&s)
			{
				m_buffer = m_string->GetBuffer(0);
				// fixes an incompatibility with CStdString, see "NullBuffer" in .cpp 
				if (!s.GetLength())
					m_buffer = NullBuffer; 
			}

			CStringLock(CStdString & s, int minChars) : m_string(&s)
			{
				m_buffer = m_string->GetBuffer(minChars);

				// fixes an incompatibility with CStdString, see "NullBuffer" in .cpp 
				if (!s.GetLength() && !minChars)
					m_buffer = NullBuffer; 

			}

			operator LPTSTR() { return m_buffer; }

			void Release(int newLen = -1)
			{
				if (m_string)
				{
					m_string->ReleaseBuffer(newLen);
					m_string = NULL;
					m_buffer = NULL;
				}
			}

			~CStringLock()  { Release(); }

		};



		/// CStdString incompatibility:
		/// http://www.codeproject.com/string/stdstring.asp
		/// If the contained string is empty, CStdString.GetBuffer returns a pointer to a constant
		/// empty string, which may cause an access violation when I write the terminating zero
		/// (which is in my understanding implicitely allowed the way I read the MSDN docs)
		/// Solution: we return a pointer to another buffer
		TCHAR NullBufferData[1] = { 0 };
		LPTSTR CStringLock::NullBuffer = NullBufferData;


		// Helper class for Close-On-Return HKEY
		// \todo migrate template class 
		class CAutoHKEY
		{
		private:
			CAutoHKEY const & operator =(CAutoHKEY const & ); // not implemented
			CAutoHKEY(CAutoHKEY const &);   // not implemented

		protected:
			HKEY    key;

		public:
			CAutoHKEY() : key(0) {}
			CAutoHKEY(HKEY key_) : key(key_) {}
			~CAutoHKEY()          { Close(); }

			void Close()
			{
				if (key) 
				{
					RegCloseKey(key); 
					key = 0;
				}
			}

			HKEY * OutArg()
			{
				Close();
				return &key;
			}

			operator HKEY() const { return key; }

		}; // CAutoHKEY

		/// Reads an environment variable into a CString
		CStdString GetEnvVar(LPCTSTR envVar)
		{
			SetLastError(0);

			// get length of buffer
			DWORD result = GetEnvironmentVariable(envVar, L"", 0);
			if (!result)
				return CStdString();

			CStdString s;
			result = GetEnvironmentVariable(envVar, CStringLock(s, result), result);
			return s;
		}


		/// Replace path root with environment variable
		/// If the beginning of \c s matches the value of the environment variable %envVar%,
		/// it is replaced with the %envVar% value
		/// (e.g. "C:\Windows" with "%windir%"
		/// \param s [CString &, in/out]: the string to modify
		/// \param envVar [LPCTSTR]: name of the environment variable
		/// \returns true if s was modified, false otherwise.
		bool EnvUnsubstRoot(CStdString & s, LPCTSTR envVar)
		{
			// get environment value string
			CStdString envValue = GetEnvVar(envVar);
			if (!envValue.GetLength())
				return false;

			if (s.GetLength() >= envValue.GetLength() && 
				_tcsnicmp(s, envValue, envValue.GetLength())==0)
			{
				CStdString modified(_T("%"));
				modified += envVar;
				modified += _T('%');
				modified += s.Mid(envValue.GetLength());
				s = modified;
				return true;
			}
			return false;
		}

	} // namespace quibase::nsDetail


	using namespace nsDetail;

	const TCHAR Backslash = '\\';


	// ==============================================
	//  Trim
	// ----------------------------------------------
	/// Trims whitespaces from left and right side. 
	/// \param s [CString]: String to modify in-place.
	void Trim(CStdString & string)
	{
		if (_istspace(GetFirstChar(string)))
			string.TrimLeft();

		if (_istspace(GetLastChar(string)))
			string.TrimRight();
	}



	// ==================================================================
	//  GetDriveLetter(ch)
	// ------------------------------------------------------------------
	/// checks if the specified letter \c ch is a drive letter, and casts it to uppercase
	/// 
	/// \returns [TCHAR]: if \c is a valid drive letter (A..Z, or a..z), returns the drive letter
	///     cast to uppercase (A..Z). >Otherwise, returns 0
	TCHAR GetDriveLetter(TCHAR ch)
	{
		if ( (ch >= 'A' && ch <= 'Z'))
			return ch;

		if (ch >= 'a' && ch <= 'z')
			return (TCHAR) (ch - 'a' + 'A');

		return 0;
	}


	// ==================================================================
	//  GetDriveLetter(string)
	// ------------------------------------------------------------------
	/// returnd the drive letter of a path.
	/// The drive letter returned is always uppercase ('A'.`'Z'). 
	/// \param s [LPCTSTR]: the path string
	/// \returns [TCHAR]: the drive letter, converted to uppercase, if the path starts with an 
	///                     X: drive specification. Otherwise, returns 0
	// 
	TCHAR GetDriveLetter(LPCTSTR s)
	{
		if (s == NULL || *s == 0 || s[1] != ':')
			return 0;

		return GetDriveLetter(s[0]);
	}



	// ==================================================================
	//  QuoteSpaces
	// ------------------------------------------------------------------
	///
	/// Quotes the string if it is not already quoted, and contains spaces
	/// see also MSDN: \c PathQuoteSpaces
	/// \note If the string is already quoted, an additional pair of quotes is added.
	/// \param str [CString const &]: path string to add quotes to
	/// \returns [CString]: path string with quotes added if required
	// 
	CStdString QuoteSpaces(CStdString const & str)
	{
		// preserve refcounting if no changes will be made
		if (str.Find(' ')>=0)  // if the string contains any spaces...
		{
			CStdString copy(str);
			CStringLock buffer(copy, copy.GetLength() + 2);
			PathQuoteSpaces(buffer);
			buffer.Release();
			return copy;
		}

		return str; // unmodified
	}



	/// helper function for GetRootType
	inline ERootType GRT_Return(ERootType type, int len, int * pLen)
	{
		if (pLen)
			*pLen = len;
		return type;
	}

	// ==================================================================
	//  GetRootType
	// ------------------------------------------------------------------
	/// 
	/// returns the type of the path root, and it's length.
	/// For supported root types, see \ref quibase::ERootType "ERootType" enumeration
	/// 
	/// \param path [LPCTSTR]: The path to analyze
	/// \param pLen [int *, out]: if not NULL, receives the length of the root part (in characters)
	/// \param greedy [bool=true]: Affects len and type of the following root types:
	///     - \c "\\server\share" : with greedy=true, it is treated as one \c rtServerShare root,
	///       otherwise, it is treated as \c rtServer root
	///     
	/// \returns [ERootType]: type of the root element 
	///         
	/// 
	ERootType GetRootType(LPCTSTR path, int * pLen, bool greedy)
	{
		//   ERootType   type = rtNoRoot;
		//   int len = 0;

		const TCHAR * invalidChars = _T("\\/:*/\"<>|");
		const TCHAR bk = '\\';

		if (!path || !*path)
			return GRT_Return(rtNoRoot, 0, pLen);

		// drive spec
		if (_istalpha(*path) && path[1] == ':')
		{
			if (path[2] == bk) { return GRT_Return(rtDriveRoot, 3, pLen); }
			else               { return GRT_Return(rtDriveCur, 2, pLen); }
		}

		// anything starting with two backslashes
		if (path[0] == bk && path[1] == bk)
		{
			// UNC long path?
			if (path[2] == '?' && path[3] == bk)
			{
				int extraLen = 0;
				GetRootType(path+4, &extraLen) ;
				return GRT_Return(rtLongPath, 4 + extraLen, pLen);
			}

			// position of next backslash or colon
			int len = 2 + _tcscspn(path+2, invalidChars);
			TCHAR const * end = path+len;

			// server only, no backslash
			if (*end == 0) 
				return GRT_Return(rtServerOnly, len, pLen);

			// server only, terminated with backslash
			if (*end == bk && end[1] == 0) 
				return GRT_Return(rtServerOnly, len+1, pLen); 

			// server, backslash, and more...
			if (*end == bk)
			{
				if (!greedy)  // return server only
					return GRT_Return(rtServer, len, pLen);

				len += 1 + _tcscspn(end+1, invalidChars);
				end = path + len;

				// server, share, no backslash
				if (*end == 0) 
					return GRT_Return(rtServerShare, len, pLen); 

				// server, share, backslash
				if (*end == '\\') 
					return GRT_Return(rtServerShare, len+1, pLen);
			}
			// fall through to other tests
		}

		int len = _tcscspn(path, invalidChars);
		TCHAR const * end = path + len;

		// (pseudo) protocol:
		if (len > 0 && *end == ':')
		{
			if (end[1] == '/' && end[2] == '/') 
				return GRT_Return(rtProtocol, len+3, pLen);
			else 
				return GRT_Return(rtPseudoProtocol, len+1, pLen); 
		}

		return GRT_Return(rtNoRoot, 0, pLen);
	}





	// ==================================================================
	//  CXShellPath::Trim
	// ------------------------------------------------------------------
	/// 
	/// removes leading and trailing spaces.
	// 
	CXShellPath &  CXShellPath::Trim()
	{
		quibase::Trim(m_path);
		return *this;
	}

	// ==================================================================
	//  CXShellPath::Unquote
	// ------------------------------------------------------------------
	/// 
	/// removes (double) quotes from around the string
	// 
	CXShellPath & CXShellPath::Unquote()
	{
		if (GetFirstChar(m_path) == '"' && GetLastChar(m_path) == '"')
			m_path = m_path.Mid(1, m_path.GetLength()-2);
		return *this;
	}

	// ==================================================================
	//  CXShellPath::Canonicalize
	// ------------------------------------------------------------------
	///
	/// Collapses "\\..\\" and "\\.\\" path parts.
	/// see also MSDN: PathCanonicalize
	/// \note
	/// PathCanonicalize works strange on relative paths like "..\\..\\x" -
	/// it is changed to "\x", which is clearly not correct. SearchAndQualify is affected
	/// by the same problem
	/// \todo handle this separately?
	/// 
	/// \par Implementation Differences
	/// \c PathCanonicalize does turn an empty path into a single backspace.
	/// CXShellPath::Canonicalize does not modify an empty path.
	// 
	CXShellPath & CXShellPath::Canonicalize()
	{
		if (!m_path.GetLength())  // PathCanonicalize turns an empty path into "\\" - I don't want this..
			return *this;

		if (m_path.Find(_T("\\."))>=0)
		{
			CStdString target = m_path;  // PathCanonicalize requires a copy to work with
			CStringLock buffer(target, m_path.GetLength()+2); // might add a backslash sometimes !
			PathCanonicalize(buffer, m_path);
			buffer.Release();
			m_path = target;
		}

		return *this;
	}


	// ==================================================================
	//  CXShellPath::ShrinkXXLPath
	// ------------------------------------------------------------------
	///
	/// Removes an "Extra long file name" specification
	/// Unicode API allows pathes longer than MAX_PATH, if they start with "\\\\?\\". This function
	/// removes such a specification if present. See also MSDN: "File Name Conventions".
	// 
	CXShellPath & CXShellPath::ShrinkXXLPath()
	{
		if (m_path.GetLength() >= 6 &&   // at least 6 chars for [\\?\C:]
			_tcsncmp(m_path, _T("\\\\?\\"), 4) == 0)
		{
			LPCTSTR path = m_path;

			if (quibase::GetDriveLetter(path[4]) != 0 && path[5] == ':')
				m_path = m_path.Mid(4);

			else if (m_path.GetLength() >= 8)  // at least 8 chars for [\\?\UNC\]
			{
				if (_tcsnicmp(path + 4, _T("UNC\\"), 4) == 0)
				{
					// remove chars [2]..[7]
					int len = m_path.GetLength() - 8; // 
					CStringLock buffer(m_path);
					memmove(buffer+2, buffer+8, len * sizeof(TCHAR));
					buffer.Release(len+2);
				}

			}
		}

		return *this;
	}


	// ==================================================================
	//  CXShellPath::Assign
	// ------------------------------------------------------------------
	/// 
	/// Assigns a string to the path object, optionally applying cleanup of the path name
	///
	/// \param str [CString const &]: The string to assign to the path
	/// \param cleanup [DWORD, default = epc_Default]: operations to apply to the path
	/// \returns [CXShellPath &]: reference to the path object itself
	/// 
	/// see CXShellPath::Clean for a description of the cleanup options
	// 
	CXShellPath & CXShellPath::Assign(CStdString const & str, DWORD cleanup)
	{
		m_path = str;
		Clean(cleanup);
		return *this;
	}


	// ==================================================================
	//  CXShellPath::MakePretty
	// ------------------------------------------------------------------
	///
	/// Turns an all-uppercase path to all -lowercase. A path containing any lowercase 
	/// character is not modified.
	/// (This is Microsoft#s idea of prettyfying a path. I don't know what to say)
	/// 
	CXShellPath & CXShellPath::MakePretty()
	{
		CStringLock buffer(m_path);
		PathMakePretty(buffer);
		return *this;
	}


	// ==================================================================
	//  CXShellPath::Clean
	// ------------------------------------------------------------------
	///
	/// Applies multiple "path cleanup" operations
	/// 
	/// \param cleanup [DWORD]: a combination of zero or more quibase::EPathCleanup flags (see below)
	/// \returns [CXShellPath &]: a reference to the path object
	/// 
	/// The following cleanup operations are defined:
	///     - \c epcRemoveArgs: call PathRemoveArgs to remove arguments
	///     - \c epcRemoveIconLocation: call to PathParseIconLocation to remove icon location
	///     - \c \b epcTrim: trim leading and trailing whitespaces
	///     - \c \b epcUnquote: remove quotes 
	///     - \c \b epcTrimInQuote: trim whitespaces again after unqouting.
	///     - \c \b epcCanonicalize: collapse "\\.\\" and "\\..\\" segments
	///     - \c \b epcRemoveXXL: remove an "\\\\?\\" prefix for path lengths exceeding \c MAX_PATH
	///     - \c epcSlashToBackslash: (not implemented)  change forward slashes to back slashes (does not modify a "xyz://" protocol root)
	///     - \c epcMakePretty: applies PathMakePretty
	///     - \c epcRemoveArgs: remove command line arguments
	///     - \c epcRemoveIconLocation: remove icon location number
	///     - \c \b epcExpandEnvStrings: Expand environment strings
	/// 
	/// This function is called by most assignment constructors and assignment operators, using
	/// the \c epc_Default cleanup options (typically those set in bold above, but check the enum 
	/// documentation in case I forgot to update this one).
	/// 
	/// Constructors and Assignment operators that take a string (\c LPCTSTR, \c LPCTSTR, \c CString) call
	/// this function. Copy or assignment from another \c CXShellPath object does not call this function.
	/// 
	/// 
	//
	CXShellPath & CXShellPath::Clean(DWORD cleanup)
	{
		if (cleanup & epcRemoveArgs)
		{
			// remove leading spaces, otherwise PathRemoveArgs considers everything a space
			if (cleanup & epcTrim)
				m_path.TrimLeft();  

			PathRemoveArgs(CStringLock(m_path));
		}

		if (cleanup & epcRemoveIconLocation)
			PathParseIconLocation(CStringLock(m_path));


		if (cleanup & epcTrim)
			Trim();

		if (cleanup & epcUnquote)
		{
			Unquote();
			if (cleanup & epcTrimInQuote)
				Trim();
		}

		if (cleanup & epcExpandEnvStrings)
			ExpandEnvStrings();

		if (cleanup & epcCanonicalize)
			Canonicalize();

		if (cleanup & epcRemoveXXL)
			ShrinkXXLPath();

		if (cleanup & epcSlashToBackslash)
			m_path.Replace('/', '\\');

		if (cleanup & epcMakePretty)
			MakePretty();

		return *this;
	}


	// Extractors
	CStdString CXShellPath::GetStr(DWORD packing) const
	{
		CStdString str = m_path;

		//    _ASSERTE(!(packing & eppAutoXXL));   // TODO

		if (packing & eppAutoQuote)
			str = QuoteSpaces(str);

		if (packing & eppBackslashToSlash)
			str.Replace('\\', '/');  // TODO: suport server-share and protocol correctly

		return str;
	}


	_bstr_t CXShellPath::GetBStr(DWORD packing) const
	{
		return _bstr_t( GetStr(packing).operator LPCTSTR());
	}



	// ==================================================================
	//  Constructors 
	// ------------------------------------------------------------------
	// 
	CXShellPath::CXShellPath(LPCSTR path) : m_path(path)    
	{ 
		Clean();
	}

	CXShellPath::CXShellPath(LPCWSTR path) : m_path(path)
	{
		Clean();
	}

	CXShellPath::CXShellPath(CStdString const & path) : m_path(path)
	{
		Clean();
	}

	CXShellPath::CXShellPath(CXShellPath const & path) : m_path(path.m_path) {}  // we assume it is already cleaned


	CXShellPath::CXShellPath(CStdString const & path,DWORD cleanup) : m_path(path)
	{
		Clean(cleanup);
	}


	// ==================================================================
	//  Assignment
	// ------------------------------------------------------------------
	// 
	CXShellPath & CXShellPath::operator=(LPCSTR rhs)
	{
#ifndef _UNICODE    // avoidf self-assignment
		if (rhs == m_path.operator LPCTSTR())
			return *this;
#endif
		m_path = rhs;
		Clean();
		return *this;
	}

	CXShellPath & CXShellPath::operator=(LPCWSTR rhs)
	{
#ifdef _UNICODE // avoid self-assignment
		if (rhs == m_path.operator LPCTSTR())
			return *this;
#endif
		m_path = rhs;
		Clean();
		return *this;
	}

	CXShellPath & CXShellPath::operator=(CStdString const & rhs)
	{
		// our own test for self-assignment, so we can skip CClean in this case
		if (rhs.operator LPCTSTR() == m_path.operator LPCTSTR())
			return *this;
		m_path = rhs;
		Clean();
		return *this;
	}

	CXShellPath & CXShellPath::operator=(CXShellPath const & rhs)
	{
		if (rhs.m_path.operator LPCTSTR() == m_path.operator LPCTSTR())
			return *this;

		m_path = rhs;
		return *this;
	}




	// ==================================================================
	//  CXShellPath::operator &=
	// ------------------------------------------------------------------
	///
	/// AppendFormats a path segment, making sure it is separated by exactly one backslash
	/// \returns reference to the modified \c CXShellPath instance.
	// 
	CXShellPath & CXShellPath::operator &=(LPCTSTR rhs)
	{
		return CXShellPath::Append(rhs);
	}


	// ==================================================================
	//  CXShellPath::AddBackslash
	// ------------------------------------------------------------------
	///
	/// makes sure the contained path is terminated with a backslash
	/// \returns [CXShellPath &]: reference to the modified path
	/// see also: \c PathAddBackslash Shell Lightweight Utility API
	// 
	CXShellPath & CXShellPath::AddBackslash()
	{
		if (GetLastChar(m_path) != Backslash)
		{
			CStringLock buffer(m_path, m_path.GetLength()+1);
			PathAddBackslash(buffer);
		}
		return *this;
	}

	// ==================================================================
	//  CXShellPath::RemoveBackslash
	// ------------------------------------------------------------------
	///
	/// If the path ends with a backslash, it is removed.
	/// \returns [CXShellPath &]: a reference to the modified path.
	// 
	CXShellPath & CXShellPath::RemoveBackslash()
	{
		if (GetLastChar(m_path) == Backslash)
		{
			CStringLock buffer(m_path, m_path.GetLength()+1);
			PathRemoveBackslash(buffer);
		}
		return *this;
	}

	// ==================================================================
	//  CXShellPath::AppendFormat
	// ------------------------------------------------------------------
	///
	/// Concatenates two paths
	/// \par Differences to \c PathAddBackslash:
	/// Unlike \c PathAddBackslash, \c CXShellPath::AppendFormat AppendFormats a single backslash if rhs is empty (and
	/// the path does not already end with a backslash)
	/// 
	/// \param rhs [LPCTSTR]: the path component to AppendFormat
	/// \returns [CXShellPath &]: reference to \c *this
	// 
	CXShellPath & CXShellPath::Append(LPCTSTR rhs)
	{
		if (rhs == NULL || *rhs == _T('\0'))
		{
			AddBackslash();
		}
		else
		{
			int rhsLen = rhs ? _tcslen(rhs) : 0;
			CStringLock buffer(m_path, m_path.GetLength()+rhsLen+1);
			PathAppend(buffer, rhs);
		}
		return *this;
	}



	// ==================================================================
	//  CXShellPath::ShellGetRoot
	// ------------------------------------------------------------------
	///
	/// Retrieves the Root of the path, as returned by \c PathSkipRoot.
	/// 
	/// \note For a more detailed (but "hand-made") implementation see GetRoot and GetRootType.
	/// 
	/// The functionality of \c PathSkipRoot is pretty much limited:
	///     - Drives ("C:\\" but not "C:")
	///     - UNC Shares ("\\\\server\\share\\", but neither "\\\\server" nor "\\\\server\\share")
	///     - UNC long drive ("\\\\?\\C:\\")
	/// 
	/// \returns [CString]: the rot part of the string
	/// 
	CStdString CXShellPath::ShellGetRoot() const
	{
		LPCTSTR path = m_path;
		LPCTSTR rootEnd = PathSkipRoot(path);

		if (rootEnd == NULL)
			return CStdString();

		return m_path.Left(rootEnd - path);
	}


	// ==================================================================
	//  GetRootType
	// ------------------------------------------------------------------
	/// 
	/// returns the type of the root, and it's length.
	/// For supported tpyes, see \ref quibase::ERootType "ERootType".
	/// see also \ref quibase::GetRootType
	/// 
	ERootType CXShellPath::GetRootType(int * len, bool greedy) const
	{
		return quibase::GetRootType(m_path, len, greedy);
	}

	// ==================================================================
	//  GetRoot
	// ------------------------------------------------------------------
	/// 
	/// \param rt [ERootType * =NULL]: if given, receives the type of the root segment.
	/// \return [CString]: the root, as a string.
	/// 
	/// For details which root types are supported, and how the length is calculated, see
	/// \ref quibase::ERootType "ERootType" and \ref quibase::GetRootType
	/// 
	CStdString CXShellPath::GetRoot(ERootType * rt, bool greedy) const
	{
		int len = 0;
		ERootType rt_ = quibase::GetRootType(m_path, &len, greedy);
		if (rt)
			*rt = rt_;
		return m_path.Left(len);
	}


	// ==================================================================
	//  CXShellPath::SplitRoot
	// ------------------------------------------------------------------
	///
	/// removes and returns the root element from the contained path
	/// You can call SplitRoot repeatedly to retrieve the path segments in order
	/// 
	/// \param rt [ERootType * =NULL] if not NULL, receives the type of the root element
	///     note: the root element type can be recognized correctly only for the first segment
	/// \returns [CString]: the root element of the original path
	/// \par Side Effects: root element removed from contained path
	/// 
	CStdString CXShellPath::SplitRoot(ERootType * rt)
	{
		CStdString head;

		if (!m_path.GetLength())
			return head;

		int rootLen = 0;
		ERootType rt_ = quibase::GetRootType(m_path, &rootLen, false);
		if (rt)
			*rt = rt_;

		if (rt_ == rtNoRoot) // not a typical root element
		{
			int start = 0;
			if (GetFirstChar(m_path) == '\\') // skip leading backslash (double backslas handled before)
				++start;

			int ipos = m_path.Find('\\', start);
			if (ipos < 0)
			{
				head = start ? m_path.Mid(start) : m_path;
				m_path.Empty();
			}
			else
			{
				head = m_path.Mid(start, ipos-start);
				m_path = m_path.Mid(ipos+1);
			}
		}
		else
		{
			head = m_path.Left(rootLen);

			if (rootLen < m_path.GetLength() && m_path[rootLen] == '\\')
				++rootLen;
			m_path = m_path.Mid(rootLen);
		}

		return head;
	}



	// ==================================================================
	//  CXShellPath::GetPath
	// ------------------------------------------------------------------
	///
	/// returns the path (without file name and extension)
	/// \param includeRoot [bool=true]: if \c true (default), the root is included in the retuerned path.
	/// \returns [CXShellPath]: the path, excluding file name and 
	/// \par Implementation:
	///     Uses \c PathFindFileName, and \c PathSkipRoot
	/// \todo
	///     - in "c:\\temp\\", \c PathFindFileName considers "temp\\" to be a file name and returns
	///       "c:\\" only. This is clearly not my idea of a path
	///     - when Extending \c CXShellPath::GetRoot, this function should be adjusted as well
	/// 
	// 
	CXShellPath CXShellPath::GetPath(bool includeRoot ) const
	{
		LPCTSTR path = m_path;
		LPCTSTR fileName = PathFindFileName(path);
		if (fileName == NULL) // seems to find something in any way!
			fileName = path + m_path.GetLength();

		LPCTSTR rootEnd = includeRoot ? NULL : PathSkipRoot(path);

		CXShellPath ret;
		if (rootEnd == NULL)  // NULL if root should be included, or no root is found
			return m_path.Left(fileName-path);
		else
			return m_path.Mid(rootEnd-path, fileName-rootEnd);
	}

	// ==================================================================
	//  CXShellPath::GetName
	// ------------------------------------------------------------------
	///
	/// \returns [CString]: the file name of the path
	/// \par Differences to \c PathFindFileName:
	/// \c PathFindFileName, always treats the last path segment as file name, even if it ends with a backslash.
	/// \c GetName treats such a string as not containing a file name\n
	/// \b Example: for "c:\\temp\\", \c PathFindFileName finds "temp\\" as file name. \c GetName returns
	/// an empty string.
	CStdString CXShellPath::GetName() const
	{
		// fix treating final path segments as file name
		if (GetLastChar(m_path) == '\\')
			return CStdString();

		LPCTSTR path = m_path;
		LPCTSTR fileName = PathFindFileName(path);
		if (fileName == NULL)
			return CStdString();

		return m_path.Mid(fileName-path);
	}

	// ==================================================================
	//  CXShellPath::GetTitle
	// ------------------------------------------------------------------
	///
	/// \returns [CString]: the file title, without path or extension
	// 
	CStdString CXShellPath::GetTitle() const
	{
		LPCTSTR path = m_path;
		LPCTSTR fileName = PathFindFileName(path);
		LPCTSTR ext      = PathFindExtension(path);

		if (fileName == NULL)
			return CStdString();

		if (ext == NULL)
			return m_path.Mid(fileName-path);

		return m_path.Mid(fileName-path, ext-fileName);


	}

	// ==================================================================
	//  CXShellPath::GetExtension
	// ------------------------------------------------------------------
	///
	/// \returns [CString]: file extension
	/// \par Differences to \c PathFindExtension
	/// Unlike \c PathFindExtension, the period is not included in the extension string
	// 
	CStdString CXShellPath::GetExtension() const
	{
		LPCTSTR path = m_path;
		LPCTSTR ext      = PathFindExtension(path);

		if (ext == NULL)
			return CStdString();

		if (*ext == '.')        // skip "."
			++ext;

		return m_path.Mid(ext-path);
	}




	// ==================================================================
	//  CXShellPath::AddExtension
	// ------------------------------------------------------------------
	///
	/// AppendFormats the specified extension to the path. The path remains unmodified if it already contains
	/// an extension (that is, the part behind the last backslash contains a period)
	/// \par Difference to \c PathAddExtension
	/// Unlike CXShellPath::AddExtension adds a period, if \c extension does not start with one
	/// \param extension [LPCTSTR]: the extension to AppendFormat
	/// \param len [int, default=-1]: (optional) length of \c extension in characters, not counting the
	///     terminating zero. This argument is only for avoiding a call to _tcslen if the caller already
	///     knows the length of the string. The string still needs to be zero-terminated and contain exactly 
	///     \c len characters. 
	/// \returns [CXShellPath &]: reference to the modified Path object
	// 
	CXShellPath & CXShellPath::AddExtension(LPCTSTR extension, int len)
	{
		if (!extension)
			return AddExtension(_T(""), 0);

		if (*extension != _T('.'))
		{
			CStdString s = _T('.') + extension;
			return AddExtension( s, s.GetLength());
		}

		if (len < 0)
			return AddExtension(extension, _tcslen(extension));

		int totalLen = m_path.GetLength() + len;  // already counts the period

		PathAddExtension(CStringLock(m_path, totalLen), extension);
		return *this;
	}


	// ==================================================================
	//  CXShellPath::RemoveExtension
	// ------------------------------------------------------------------
	///
	/// Removes the extension of the path, if it has any.
	/// \returns [CXShellPath &]: reference to the modified path object
	// 
	CXShellPath& CXShellPath::RemoveExtension()
	{
		PathRemoveExtension(CStringLock(m_path));
		return *this;
	}


	// ==================================================================
	//  CXShellPath::RenameExtension
	// ------------------------------------------------------------------
	///
	/// Replaces an existing extension with the new given extension.
	/// If the path has no extension, it is AppendFormated.
	/// \par Difference  to \c PathRenameExtension:
	/// Unlike PathRenameExtension, \c newExt needs not start with a period.
	/// \param newExt [LPCTSTR ]: newextension
	/// \returns [CXShellPath &]: reference to the modified path string
	// 
	CXShellPath & CXShellPath::RenameExtension(LPCTSTR newExt)
	{
		if (newExt == NULL || *newExt != '.')
		{
			RemoveExtension();
			return AddExtension(newExt);
		}

		int maxLen = m_path.GetLength() + _tcslen(newExt) + 1;
		PathRenameExtension(CStringLock(m_path, maxLen), newExt);
		return *this;
	}


	// ==================================================================
	//  ::RemoveFileSpec
	// ------------------------------------------------------------------
	///
	/// Removes the file specification (amd extension) from the path.
	/// \returns [CXShellPath &]: a reference to the modified path object
	// 
	CXShellPath & CXShellPath::RemoveFileSpec()
	{
		PathRemoveFileSpec(CStringLock(m_path));
		return *this;
	}


	// ==================================================================
	//  CXShellPath::SplitArgs
	// ------------------------------------------------------------------
	///
	/// (static) Separates a path string from command line arguments
	/// 
	/// \param path_args [CString const &]: the path string with additional command line arguments
	/// \param args [CString *, out]: if not \c NULL, receives the arguments separated from the path
	/// \param cleanup [DWORD, = epc_Default]: the "cleanup" treatment to apply to the path, see \c CXShellPath::Clean
	/// \returns [CXShellPath]: a new path without the arguments
	// 
	CXShellPath SplitArgs(CStdString const & path_args, CStdString * args, DWORD cleanup)
	{
		CStdString pathWithArgs = path_args;

		// when "trim" is given, trim left spaces, so PathRemoveArgsworks correctly and returns 
		// the path part with the correct length
		if (cleanup & epcTrim)
			pathWithArgs.TrimLeft();

		// assign with only removing the arguments
		CXShellPath path(pathWithArgs, epcRemoveArgs);

		// cut non-argument part away from s
		if (args)
		{
			*args = pathWithArgs.Mid(path.GetLength());
			args->TrimLeft();
		}

		// now, clean the contained string (last since it might shorten the path string)
		path.Clean(cleanup &~ epcRemoveArgs);

		return path;
	}


	// ==================================================================
	//  CXShellPath::SplitIconLocation
	// ------------------------------------------------------------------
	/// 
	/// (static) Splits a path string containing an icon location into path and icon index
	///
	/// \param path_icon [CString const &]: the string containing an icon location
	/// \param pIcon [int *, NULL]: if not NULL, receives the icon index 
	/// \param cleanup [DWORD, epc_Default]: additional cleanup to apply to the returned path
	/// \returns [CXShellPath]: the path contained in \c path_icon (without the icon location)
	// 
	CXShellPath SplitIconLocation(CStdString const & path_icon, int * pIcon, DWORD cleanup)
	{
		CStdString strpath = path_icon;
		int icon = PathParseIconLocation( CStringLock(strpath) );
		if (pIcon)
			*pIcon = icon;

		return CXShellPath(strpath, cleanup & ~epcRemoveIconLocation);
	}


	// ==================================================================
	//  CXShellPath::BuildRoot
	// ------------------------------------------------------------------
	///
	/// (static) Creates a root path from a drive index (0..25)
	/// \param driveNumber [int]: Number of the drive, 0 == 'A', etc.
	/// \returns [CXShellPath]: a path consisitng only of a drive root
	// 
	CXShellPath BuildRoot(int driveNumber)
	{
		CStdString strDriveRoot;
		::PathBuildRoot(CStringLock(strDriveRoot, 3), driveNumber);
		return CXShellPath(strDriveRoot, 0);
	}



	// ==================================================================
	//  CXShellPath::GetModuleFileName
	// ------------------------------------------------------------------
	///
	/// Returns the path of the dspecified module handle
	/// Path is limited to \c MAX_PATH characters
	/// see Win32: GetModuleFileName for more information
	/// \param module [HMODULE =NULL ]: DLL module handle, or NULL for path to exe
	/// \returns [CXShellPath]: path to the specified module, or to the application exe if \c module==NULL.
	///         If an error occurs, the function returrns an empty string. 
	///         Call \c GetLastError() for more information.
	/// 
	CXShellPath GetModuleFileName(HMODULE module)
	{
		CStdString path;
		DWORD ok = ::GetModuleFileName(module, CStringLock(path, MAX_PATH), MAX_PATH+1);
		if (ok == 0)
			return CXShellPath();
		return CXShellPath(path);
	}



	// ==================================================================
	//  CXShellPath::GetCurrentDirectory
	// ------------------------------------------------------------------
	///
	/// \returns [CXShellPath]: the current directory, se Win32: \c GetCurrentDirectory.
	/// \remarks
	/// If an error occurs the function returns an empty string. More information is available
	/// through \c GetLastError.
	/// 
	CXShellPath GetCurrentDirectory()
	{
		CStdString path;
		CStringLock buffer(path, MAX_PATH);
		DWORD chars = ::GetCurrentDirectory(MAX_PATH+1, buffer);
		buffer.Release(chars);
		return CXShellPath(path);
	}


	// ==================================================================
	//  CXShellPath::GetCommonPrefix
	// ------------------------------------------------------------------
	///
	/// Returns the common prefix of this path and the given other path,
	/// e.g. for "c:\temp\foo\foo.txt" and "c:\temp\bar\bar.txt", it returns
	/// "c:\temp".
	/// \param secondPath [LPCTSTR]: the path to compare to
	/// \returns [CXShellPath]: a new path, containing the part that is identical
	// 
	CXShellPath CXShellPath::GetCommonPrefix(LPCTSTR secondPath)
	{
		CStdString prefix;
		PathCommonPrefix(m_path, secondPath, CStringLock(prefix, m_path.GetLength()));
		return CXShellPath(prefix, 0);


	}



	// ==================================================================
	//  CXShellPath::GetDriveNumber
	// ------------------------------------------------------------------
	///
	/// \returns [int]: the driver number (0..25 for A..Z), or -1 if the 
	///     path does not start with a drive letter
	// 
	int CXShellPath::GetDriveNumber()
	{
		return PathGetDriveNumber(m_path);
	}

	// ==================================================================
	//  CXShellPath::GetDriveLetter
	// ------------------------------------------------------------------
	///
	/// \returns [TCHAR]: the drive letter in uppercase, or 0
	// 
	TCHAR CXShellPath::GetDriveLetter()
	{
		int driveNum = GetDriveNumber();
		if (driveNum < 0)
			return 0;
		return (TCHAR)(driveNum + 'A');
	}


	// ==================================================================
	//  CXShellPath::RelativePathTo
	// ------------------------------------------------------------------
	///
	/// Determines a relative path from the contained path to the specified \c pathTo
	/// \par Difference to \c PathRelativeTo:
	///  - instead of a file attributes value, you specify a flag (this is a probelm only
	///     if the function supports other attribues than FILE_ATTRIBUTE_DIRECTORY in the future)
	///  - no flag / attribute is specified for the destination (it does not seem to make a difference)
	/// \param pathTo [LPCTSTR]: the target path or drive
	/// \param srcIsDir [bool =false]: determines whether the current path is as a directory or a file
	/// \returns [CXShellPath]: a relative path from this to \c pathTo
	// 
	CXShellPath CXShellPath::RelativePathTo(LPCTSTR pathTo,bool srcIsDir)
	{
		CStdString path;
		if (!pathTo)
			return CXShellPath();

		// maximum length estimate: 
		// going up to the root of a path like "c:\a\b\c\d\e", and then AppendFormat the entire to path
		int maxLen = 3*m_path.GetLength() / 2 +1  + _tcslen(pathTo); 

		PathRelativePathTo( CStringLock(path, maxLen), 
			m_path, 
			srcIsDir ? FILE_ATTRIBUTE_DIRECTORY : 0,
			pathTo, 0);

		return CXShellPath(path, 0);
	}


	// ==================================================================
	//  MakeRelative
	// ------------------------------------------------------------------
	/// 
	/// Of the path contained is below \c basePath, it is made relative.
	/// Otherwise, it remains unmodified. 
	/// 
	/// Unlike RelativePathTo, the path is made relative only if the base path
	/// matches completely, and does not generate ".." elements.
	/// 
	/// \return [bool] true if the path was modified, false otherwise.
	/// 
	bool CXShellPath::MakeRelative(CXShellPath const & basePath)
	{
		CXShellPath basePathBS = basePath;
		basePathBS.AddBackslash(); // add backslash so that "c:\a" is not a base path for "c:\alqueida\files"

		if (m_path.GetLength() > basePathBS.GetLength())
		{
			if (0 == _tcsnicmp(basePathBS, m_path, basePathBS.GetLength()))
			{
				m_path = m_path.Mid(basePathBS.GetLength());
				return true;
			}
		}
		return false;
	}

	// ==================================================================
	//  MakeAbsolute
	// ------------------------------------------------------------------
	/// 
	/// If the contained path is relative, it is prefixed by \c basePath.
	/// Otherwise it remains unmodified.
	/// 
	/// Use: as anti-MakeRelative.
	/// 
	bool CXShellPath::MakeAbsolute(CXShellPath const & basePath)
	{
		if (IsRelative())
		{
			m_path = basePath & m_path;
			return true;
		}
		return false;
	}




	// ==================================================================
	//  CXShellPath::MatchSpec
	// ------------------------------------------------------------------
	///
	/// Checks if the path matches a certain specification
	/// \param spec [LPCTSTR]: File specification (like "*.txt")
	/// \returns [bool]: true if the path matches the specification
	// 
	bool CXShellPath::MatchSpec(LPCTSTR spec)
	{
		return PathMatchSpec(m_path, spec) != 0;
	}



	// ==================================================================
	//  CXShellPath::ExpandEnvStrings
	// ------------------------------------------------------------------
	///
	/// replaces environment string references with their current value.
	/// See MSDN: \c ExpandEnvironmentStrings for more information
	/// \returns [CXShellPath &]: reference to the modified path
	// 
	CXShellPath & CXShellPath::ExpandEnvStrings()
	{
		CStdString target;

		DWORD len = m_path.GetLength();

		DWORD required = ExpandEnvironmentStrings(
			m_path,
			CStringLock(target, len), len+1);

		if (required > len+1)
			ExpandEnvironmentStrings(
			m_path,
			CStringLock(target, required), required+1);

		m_path = CXShellPath(target, 0);
		return *this;
	}


	// ==================================================================
	//  CXShellPath::GetCompactStr
	// ------------------------------------------------------------------
	///
	/// Inserts ellipses so the path fits into the specified number of pixels
	/// See also SMDN: \c PathCompactPath 
	/// \param dc [HDC]: device context where the path is displayed
	/// \param dx [UINT]: number of pixels where to display
	/// \param eppFlags [DWORD, =0]: combination of \c EPathPacking flags indicating how to prepare the path
	/// \returns [CString]: path string prepared for display
	// 
	CStdString CXShellPath::GetCompactStr(HDC dc,UINT dx, DWORD eppFlags)
	{
		CStdString ret = GetStr(eppFlags);
		PathCompactPath(dc, CStringLock(ret), dx);
		return ret;
	}

	// ==================================================================
	//  CXShellPath::GetCompactStr
	// ------------------------------------------------------------------
	///
	/// Inserts ellipses so the path does not exceed the given number of characters
	/// \param cchMax [UINT]: maximum number of characters
	/// \param eppFlags [DWORD, =0]: combination of \c EPathPacking flags indicating how to prepare the path
	/// \param flags [DWORD, =0]: reserved, must be 0
	/// \returns [CString]: path string prepared for display
	// 
	CStdString CXShellPath::GetCompactStr(UINT cchMax,DWORD flags, DWORD eppFlags )
	{
		CStdString cleanPath = GetStr(eppFlags);
		CStdString ret;

		PathCompactPathEx(CStringLock(ret, cleanPath.GetLength()), cleanPath, cchMax, flags);
		return ret;
	}

	// ==================================================================
	//  CXShellPath::SetDlgItem
	// ------------------------------------------------------------------
	///
	/// Sets the text of a child control of a window or dialog, 
	/// PathCompactPath to make it fit
	/// 
	/// \param dlg [HWND]: the window handle of the parent window
	/// \param dlgCtrlID [UINT]: ID of the child control
	/// \param eppFlags [DWORD, =0]: combination of \c EPathPacking flags indicating how to prepare the path
	/// \returns [void]:
	// 
	void CXShellPath::SetDlgItem(HWND dlg,UINT dlgCtrlID, DWORD eppFlags)
	{
		CStdString cleanPath = GetStr(eppFlags);
		PathSetDlgItemPath(dlg, dlgCtrlID, cleanPath);
	}


	// ==================================================================
	//  ::SearchAndQualify
	// ------------------------------------------------------------------
	///
	/// Searches for the given file on the search path. If it exists in the search path, 
	/// it is qualified with the full path of the first occurence found. Otherwise, it is 
	/// qualified with the current path. An absolute file paths remains unchanged. 
	/// 
	/// \note
	/// SearchAndQualify seems to be affected by the same problem
	/// as \ref quibase::CXShellPath::Canonicalize "Canonicalize" : a path like "..\\..\\x" 
	/// is changed to "\\x", which is clearly not correct (IMHO).
	/// \n
	/// compare also: \ref quibase::CXShellPath::FindOnPath "FindOnPath":
	/// FindOnPath allows to specify custom directories to be searched before the search path, and
	/// behaves differently in some cases.
	/// If the file is not found on the search path, \c FindOnPath leaves the file name unchanged.
	/// SearchAndQualify qualifies the path with the current directory in this case
	/// 
	// 
	CXShellPath & CXShellPath::SearchAndQualify()
	{
		if (!m_path.GetLength())
			return *this;

		CStdString qualified;
		DWORD len = m_path.GetLength();
		while (qualified.GetLength() == 0)
		{
			PathSearchAndQualify(m_path, CStringLock(qualified, len), len+1);
			len *= 2;
		}
		m_path = qualified;
		return *this;
	}



	// ==================================================================
	//  CXShellPath::FindOnPath
	// ------------------------------------------------------------------
	///
	/// Similar to SearchAndQualify, but 
	/// \note
	///  -# the return value of PathFindOnPath does \b not indicate whether the file
	///     exisits, that's why we don't return it either. If you want to check if the file
	///     really is there use \c FileExists
	///  -# PathFindOnPath does not check for string overflow. Documentation recommends to use a buffer
	///     of length MAX_PATH. I don't trust it to be fail safe in case a path plus the string
	///     exceeds this length (note that the file would not be found in this case - but the shell
	///     API might be tempted to build the string inside the buffer)\n
	///     If you don't need the "additional Dirs" functionality, it is recommended to use
	///     SearchAndQualify instead
	/// 
	/// \param additionalDirs [LPCTSTR *, = NULL]: Additional NULL-terminated array of directories 
	///                                            to search first
	/// \returns [CXShellPath &]: a reference to the fully qualified file path
	/// 
	/// \par error handling:
	///   If the function succeeds, \c GetLastError returns 0. Otherwise, \c GetLastError returns a Win32 error code.
	/// 
	CXShellPath & CXShellPath::FindOnPath(LPCTSTR * additionalDirs)
	{
		DWORD len = m_path.GetLength() + 1 + MAX_PATH;
		bool ok = PathFindOnPath(CStringLock(m_path, len), additionalDirs) != 0;
		if (ok)
			SetLastError(0);
		return *this;
	}




	// ==================================================================
	//  CXShellPath::Exists
	// ------------------------------------------------------------------
	///
	/// \returns [bool]: true if the file exists on the file system, false otherwise.
	// 
	bool CXShellPath::Exists() const
	{
		return PathFileExists(m_path) != 0;
	}

	// ==================================================================
	//  CXShellPath::IsDirectory
	// ------------------------------------------------------------------
	///
	/// \returns [bool]: true if the contained path specifies a directory 
	///                  that exists on the file system
	// 
	bool CXShellPath::IsDirectory() const
	{
		return PathIsDirectory(m_path) != 0;
	}


	// ==================================================================
	//  CXShellPath::IsSystemFolder
	// ------------------------------------------------------------------
	///
	/// \param  attrib [DWORD, default = FILE_ATTRIBUTE_SYSTEM]: the attributes that 
	///                 identify a system folder
	/// \returns [bool]: true if the specified path exists and is a system folder
	// 
	bool CXShellPath::IsSystemFolder(DWORD attrib) const
	{
		return PathIsSystemFolder(m_path, attrib) != 0;
	}

	// ==================================================================
	//  CXShellPath::MakeSystemFolder
	// ------------------------------------------------------------------
	///
	/// \param make [bool, default=true]: true to set the "System Folder" state, false to reset it
	/// \par error handling:
	///  If the function succeeds, \c GetLastError returns 0. Otherwise, \c GetLastError returns a Win32 error code.
	// 
	CXShellPath & CXShellPath::MakeSystemFolder(bool make)
	{
		bool ok = make ? PathMakeSystemFolder(m_path) != 0 : PathUnmakeSystemFolder(m_path) != 0;
		if (ok) 
			SetLastError(0);
		return *this;
	}


	// ==================================================================
	//  MakeFullPath
	// ------------------------------------------------------------------
	/// 
	/// Makes a absolute path from a relative path, using the current working directory.
	/// 
	/// If the path is already absolute, it is not changed.
	/// 
	CXShellPath & CXShellPath::MakeFullPath()
	{
		if (!IsRelative())
			return *this;

		LPTSTR dummy = NULL;
		DWORD chars = GetFullPathName(m_path, 0, NULL, &dummy);
		_ASSERTE(chars > 0);

		CStdString fullStr;
		chars = GetFullPathName(m_path, chars, CStringLock(fullStr, chars), &dummy);
		m_path = fullStr;
		return *this;
	}



	// ==================================================================
	//  CXShellPath::GetAttributes
	// ------------------------------------------------------------------
	///
	/// \returns [DWORD]: the file attributes of the specified path or file, or -1 if it 
	///                   does not exist.
	// 
	DWORD CXShellPath::GetAttributes()
	{
		return ::GetFileAttributes(m_path);
		// 
	}

	// ==================================================================
	//  CXShellPath::GetAttributes
	// ------------------------------------------------------------------
	///
	/// retrives the \c GetFileExInfoStandard File Attribute information
	/// 
	/// \param fad [WIN32_FILE_ATTRIBUTE_DATA &, out]: receives the extended file attribute
	///     information (like size, timestamps) for the specified file
	/// \returns [bool]: true if the file is found and the query was successful, false otherwise
	// 
	bool CXShellPath::GetAttributes(WIN32_FILE_ATTRIBUTE_DATA & fad)
	{
		ZeroMemory(&fad, sizeof(fad));
		return ::GetFileAttributesEx(m_path, GetFileExInfoStandard, &fad) != 0;
	}


	// ==================================================================
	//  CXShellPath::EnvUnexpandRoot
	// ------------------------------------------------------------------
	///
	/// replaces path start with matching environment variable
	/// If the path starts with the value of the environment variable %envVar%,
	/// The beginning of the path is replaced with the environment variable.
	/// 
	/// e.g. when specifying "WinDir" as \c envVar, "C:\\Windows\\foo.dll" is replaced by
	/// "%WINDIR%\foo.dll"
	/// 
	/// \param envVar [LPCTSTR]: environment variable to check 
	/// \returns \c true if the path was modified.
	/// 
	/// If the environment variable does not exist, or the value of the environment variable
	/// does not match the beginning of the path, the path is unmodified and the function returns 
	/// false.
	/// 
	bool CXShellPath::EnvUnexpandRoot(LPCTSTR envVar)
	{
		return nsDetail::EnvUnsubstRoot(m_path, envVar);
	}

	// ==================================================================
	//  CXShellPath::EnvUnexpandDefaultRoots
	// ------------------------------------------------------------------
	///
	/// Tries to replace the path root with a matching environment variable.
	/// 
	/// 
	/// Checks a set of default environment variables, if they match the beginning of the path.
	/// If one of them matches, the beginning of the path is replaced with the environment
	/// variable specification, and the function returns true.
	/// Otherwise, the path remains unmodified and the function returns false.
	/// 
	/// see EnvUnexpandRoot for details.
	/// 
	bool CXShellPath::EnvUnexpandDefaultRoots()
	{
		// note: Order is important
		return EnvUnexpandRoot(_T("APPDATA")) ||
			EnvUnexpandRoot(_T("USERPROFILE")) ||
			EnvUnexpandRoot(_T("ALLUSERSPROFILE")) ||
			EnvUnexpandRoot(_T("ProgramFiles")) ||
			EnvUnexpandRoot(_T("SystemRoot")) ||
			EnvUnexpandRoot(_T("WinDir")) ||
			EnvUnexpandRoot(_T("SystemDrive"));
	}


	// ==================================================================
	//  CXShellPath::FromRegistry
	// ------------------------------------------------------------------
	/// 
	/// Reads a path string from the registry.
	/// \param baseKey [HKEY]: base key for registry path
	/// \param subkey [LPCTSTR]: registry path
	/// \param name [LPCTSTR] name of the value
	/// \returns [CXShellPath]: a path string read from the specified location
	/// 
	/// If the path is stored as REG_EXPAND_SZ, environment strings are expanded.
	/// Otherwise, the string remains unmodified.
	/// 
	/// \par Error Handling:
	///   If an error occurs, the return value is an empty string, and GetLastError() returns the respective
	///   error code. In particular, if the registry value exists but does not contain a string, GetLastError()
	///   returns ERROR_INVALID_DATA
	///   \n\n
	///   If the function succeeds, GetLastError() returns zero.
	/// 
	/// See also quibase::ToRegistry
	///             
	CXShellPath FromRegistry(HKEY baseKey, LPCTSTR subkey, LPCTSTR name)
	{
		SetLastError(0);

		CAutoHKEY key;
		DWORD ok = RegOpenKeyEx(baseKey, subkey, 0, KEY_READ, key.OutArg());
		if (ok != ERROR_SUCCESS)
		{
			SetLastError(ok);
			return CXShellPath();
		}

		DWORD len = 256;
		DWORD type = 0;

		CStdString path;

		do 
		{
			CStringLock buffer(path, len);
			if (!buffer)
			{
				SetLastError(ERROR_OUTOFMEMORY);
				return CXShellPath();
			}

			DWORD size = (len+1) * sizeof(TCHAR); // size includes terminating zero
			ok = RegQueryValueEx(key, name, NULL, &type, 
				(LPBYTE) buffer.operator LPTSTR(), &size );

			// read successfully:
			if (ok == ERROR_SUCCESS)
			{
				if (type != REG_SZ && type != REG_EXPAND_SZ)
				{
					SetLastError(ERROR_INVALID_DATA);
					return CXShellPath();
				}
				break; // accept string
			}

			// buffer to small
			if (ok == ERROR_MORE_DATA)
			{
				len = (size + sizeof(TCHAR) - 1) / sizeof(TCHAR);
				continue;
			}

			// otherwise, an error occured
			SetLastError(ok);
			return CXShellPath();
		} while(1);

		DWORD cleanup = epc_Default;
		if (type == REG_SZ)
			cleanup &= ~epcExpandEnvStrings;
		else
			cleanup |= epcExpandEnvStrings; // on by default, but I might change my mind..

		return CXShellPath(path, cleanup);
	}





	// ==================================================================
	//  CXShellPath::ToRegistry
	// ------------------------------------------------------------------
	///
	/// Writes the path to the registry
	/// 
	/// \param baseKey: root of registry path
	/// \param subkey: registry path where to store
	/// \param name: name to store the key under
	/// \param replaceEnv [bool=true]: If true (default), environment strings will be replaced
	///     with environment variables, and the string is stored as REG_EXPAND_SZ. 
	///     Otherwise, the string is stored unmodified as REG_SZ.
	/// 
	/// See also quibase::FromRegistry
	/// 
	long CXShellPath::ToRegistry(HKEY baseKey,LPCTSTR subkey,LPCTSTR name,bool replaceEnv)
	{
		CAutoHKEY key;
		DWORD ok = RegCreateKeyEx(baseKey, subkey, NULL, NULL, 0, KEY_WRITE, NULL, key.OutArg(), NULL);
		if (ok != ERROR_SUCCESS)
			return ok;

		CStdString path;
		if (replaceEnv)
		{ 
			CXShellPath ptemp = path;
			ptemp.EnvUnexpandDefaultRoots();
			path = ptemp.GetStr();
		}
		else
			path = GetStr();

		ok = RegSetValueEx(key, name, 0, replaceEnv ? REG_EXPAND_SZ : REG_SZ,
			(BYTE *) path.operator LPCTSTR(), 
			(path.GetLength()+1) * sizeof(TCHAR) );
		return ok;
	}



	// ==================================================================
	//  IsDot, IsDotDot, IsDotty
	// ------------------------------------------------------------------
	///
	bool CXShellPath::IsDot() const 
	{ 
		return m_path.GetLength() == 1 && m_path[0] == '.';
	}

	bool CXShellPath::IsDotDot() const
	{
		return m_path.GetLength() == 2 && m_path[0] == '.' && m_path[1] == '.';
	}

	bool CXShellPath::IsDotty() const
	{
		return IsDot() || IsDotDot();
	}


	const LPCTSTR InvalidChars_Windows =
		L"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
		L"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
		L"\\/:*?\"<>|";



	// ==================================================================
	//  IsValid
	// ------------------------------------------------------------------
	/// 
	/// returns true if the path satisfies Windows naming conventions
	///
	bool CXShellPath::IsValid() const
	{
		if (!m_path.GetLength()) return false;
		if (m_path.FindOneOf(InvalidChars_Windows) >= 0) return false;
		if (GetLastChar(m_path) == '.') // may not end in '.', except "." and ".."
		{
			if (m_path.GetLength() > 2 || m_path[0] != '.')
				return false;
		}
		return true;
	}




	// ==================================================================
	//  ReplaceInvalid
	// ------------------------------------------------------------------
	/// 
	/// replaces all invalid file name characters  inc \c s with \c replaceChar
	/// This is helpful when generating names based on user input
	/// 
	CStdString ReplaceInvalid(CStdString const & str, TCHAR replaceChar)
	{
		if (!str.GetLength() || CXShellPath(str).IsDotty())
			return str;

		CStdString s = str;

		for(int i=0; i<s.GetLength(); ++i)
		{
			TCHAR ch = s.GetAt(i);
			if (_tcschr(InvalidChars_Windows, ch))
				s.SetAt(i, replaceChar);
		}

		// last one may not be a dot
		int len = s.GetLength();
		if (s[len-1] == '.')
			s.SetAt(len-1, replaceChar);
		return s;
	}



	// ==================================================================

} // namespace quibase

