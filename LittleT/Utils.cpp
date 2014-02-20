/*@ Docs Online - A Google Docs Backup Applicaiotn                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* Written and Designed by Michael Haephrati                                */
/* COPYRIGHT ?008 by Michael Haephrati    haephrati@gmail.com              */
/* All rights reserved.                                                     */
/* -------------------------------------------------------------------------*/
#include "Utils.h"

//////////////////////////////////////////////////////////////////////////

const wstring &str::EmptyString()
{
  static wstring emptyString;
  return emptyString;
}

size_t SZBytes(size_t numberOfChars)
{
  return (numberOfChars + 1) * sizeof(wstring::value_type);
}

size_t str::SizeInBytes(const wchar_t *sz)
{
  return SZBytes(wcslen(sz));
}

size_t str::SizeInBytes(const wstring &str)
{
  return SZBytes(str.length());
}

//
//  FUNCTION: AssignChars(wstring &s, const wchar_t *buffer, size_t maxChars)
//
//  PURPOSE:  assigns a string a text from a buffer until '\0' is met or maxChars at most
//
void str::AssignSZ(wstring &s, const wchar_t *buffer, size_t maxChars)
{
  s.assign(buffer, wcsnlen(buffer, maxChars));
}

void str::AssignSZBytes(wstring &s, const void *buffer, size_t bufferBytes)
{
  AssignSZ(s, (wchar_t*)buffer, bufferBytes / sizeof(wstring::value_type));
}

wstring str::Format1K(LPCTSTR formatString, ...)
{
  wchar_t buf[1024];
  va_list marker;

  buf[0] = '\0';
  va_start( marker, formatString );
  _vsnwprintf_s(buf, ARRAYDIM(buf), ARRAYDIM(buf)-1, formatString, marker);
  va_end( marker );
  return wstring(buf);
}

wstring str::ToUpper(const wstring &str)
{
  vector<wchar_t> buffer;
  buffer.resize(str.length());
  memcpy(&buffer[0], str.c_str(), str.length() * sizeof(wchar_t));
  CharUpperBuff(&buffer[0], buffer.size());
  return wstring(&buffer[0], str.length());
}

void str::MB2Unicode(wstring &unicodeString, const char *mbString, int length, UINT codePage)
{
  size_t unicodeLen;
  wstring result;
  BOOL ok;

  ok = FALSE;
  if (length == -1)
    length = strlen(mbString);
  if (length > 0)
  {
    unicodeLen = MultiByteToWideChar(codePage, 0, mbString, length, NULL, 0);
    if (unicodeLen > 0)
    {
      CBuffer<wchar_t> buffer(unicodeLen);
      unicodeLen = MultiByteToWideChar(codePage, 0, mbString, length, buffer, buffer.Size());
      if (unicodeLen > 0)
      {
        ok = TRUE;
        str::AssignSZ(unicodeString, buffer, buffer.Size());
      }
    }
  }
  if (!ok)
    unicodeString.clear();
}

void str::Unicode2MB(vector<char> &mb, const wchar_t *unicodeString, UINT codePage)
{
  BOOL ok;
  int bufSize;
  size_t len;

  ok = FALSE;
  len = wcslen(unicodeString);
  if (len > 0)
  {
    bufSize = WideCharToMultiByte(codePage, 0, unicodeString, len, NULL, 0, NULL, 0);
    if (bufSize > 0)
    {
      mb.resize(bufSize);
      if (WideCharToMultiByte(codePage, 0, unicodeString, len, &mb[0], bufSize, NULL, 0) > 0)
        ok = TRUE;
    }
  }
  if (!ok)
    mb.clear();
}

wstring str::IntToStr(int n)
{
  return Format1K(L"%d", n);
}

wstring str::SizeToStr(size_t n)
{
  return Format1K(L"%Iu", n);
}

wstring str::Int64ToStr(__int64 n)
{
  return Format1K(L"%I64d", n);
}

size_t str::split(vector<wstring> &items, const wstring &s, const wchar_t delimiter)
{
  size_t prevPos = 0;
  size_t pos = 0;
  items.clear();
  while( (pos = s.find(delimiter, pos)) != wstring::npos )
  {
    if (pos > prevPos)
      items.push_back(s.substr(prevPos, pos-prevPos));
    prevPos = ++pos;
  }
  // handle the rest
  if (prevPos < s.length())
    items.push_back(s.substr(prevPos));
  return items.size();
}

int str::CompareI(const wstring &s1, const wstring &s2) // case insensitive compare
{
  return lstrcmpi(s1.c_str(), s2.c_str());
}

BOOL str::EqualI(const wstring &s1, const wstring &s2) // true if equal in case insensitive compare
{
  return (s1.length() == s2.length()) && (lstrcmpi(s1.c_str(), s2.c_str()) == 0);
}

BOOL str::StartingWithI(const wstring &s, const wstring &start)
{
  return (s.length() < start.length()) ? FALSE : 
    str::EqualI(s.length() == start.length() ? s : s.substr(0, start.length()), start);
}

BOOL str::StartingWith(const wstring &s, const wstring &start)
{
  return (s.length() < start.length()) ? FALSE : 
    (s.length() == start.length() ? s : s.substr(0, start.length())) == start;
}

//////////////////////////////////////////////////////////////////////////

void win::GetWindowText( wstring &text, HWND handle )
{
  size_t len = GetWindowTextLength(handle);
  CBuffer<wchar_t> buffer(len + 1);
  len = GetWindowText(handle, buffer, buffer.Size());
  if (len > 0)
    text = buffer;
  else
    text.clear();
}

void win::GetDlgItemText( wstring &text, HWND dialogHandle, int itemId )
{
  GetWindowText(text, GetDlgItem(dialogHandle, itemId));
}

void win::SetDlgItemText( HWND dialogHandle, int itemId, const wstring &text )
{
  ::SetDlgItemText(dialogHandle, itemId, text.c_str());
}

//////////////////////////////////////////////////////////////////////////

BOOL reg::GetValue( vector<BYTE> &buffer, HKEY key, const wchar_t *name )
{
  BOOL result;
  DWORD count;

  result = FALSE;
  count = 0;
  if (RegQueryValueEx(key, name, NULL, NULL, NULL, &count) == ERROR_SUCCESS)
  {
    buffer.resize(count);
    if (RegQueryValueEx(key, name, NULL, NULL, &buffer[0], &count) == ERROR_SUCCESS)
      result = TRUE;
  }
  if (!result)
    buffer.clear();
  return result;
}

BOOL reg::SetBinary(HKEY key, const wchar_t *name, const void *buffer, size_t bufferSize)
{
  return (RegSetValueEx(key, name, 0, REG_BINARY, (const BYTE *) buffer, bufferSize) == ERROR_SUCCESS);
}

BOOL reg::SetDword(HKEY key, const wchar_t *name, DWORD val)
{
  return (RegSetValueEx(key, name, 0, REG_DWORD, (const BYTE *) &val, sizeof(DWORD)) == ERROR_SUCCESS);
}

BOOL reg::GetDword( OUT DWORD &val, HKEY key, const wchar_t *name )
{
  BOOL result;
  vector<BYTE> buffer;

  result = reg::GetValue(buffer, key, name);
  if (result)
  {
    if (buffer.size() >= sizeof(DWORD))
      val = *((DWORD*)&buffer[0]);
    else
      result = FALSE;
  }
  return result;
}

BOOL reg::GetBool( OUT BOOL &val, HKEY key, const wchar_t *name )
{
  DWORD dw;
  BOOL result;
  result = reg::GetDword(dw, key, name);
  if (result)
    val = dw == 0 ? FALSE : TRUE;
  return result;
}

BOOL reg::GetString( OUT wstring &s, HKEY key, const wchar_t *name )
{
  BOOL result;
  vector<BYTE> buffer;

  result = reg::GetValue(buffer, key, name);
  if (result)
    str::AssignSZBytes(s, &buffer[0], buffer.size());
  return result;
}

BOOL reg::SetString(HKEY key, const wchar_t *name, const wchar_t *sz)
{
  return (RegSetValueEx(key, name, 0, REG_SZ, (const BYTE*)sz, str::SizeInBytes(sz)) == ERROR_SUCCESS);
}

BOOL reg::SetBool(HKEY key, const wchar_t *name, BOOL val)
{
  return SetDword(key, name, val ? 1 : 0);
}

//////////////////////////////////////////////////////////////////////////

void ExtractFilePath(wstring &pathOnly, const wstring &path) // returns path to file (without the last slash)
{
  size_t pos = path.rfind('\\');
  pathOnly = (pos == wstring::npos) ? str::EmptyString() : path.substr(0, pos);
}

void ExtractFileName(wstring &fileNameOnly, const wstring &path) // returns file name including extension
{
  size_t pos = path.rfind('\\');
  fileNameOnly = (pos == wstring::npos) ? path : path.substr(pos + 1);
}

// returns TRUE if file extension present, FALSE otherwise
// fileExtention receives the file extension (only, without a dot)
BOOL ExtractFileExt(OUT wstring &fileExtention, const wstring &path)
{
  size_t lastDotPos;
  BOOL result;

  result = FALSE;
  lastDotPos = path.rfind('.');
  if (lastDotPos != wstring::npos)
  {
    if (path.find('\\', lastDotPos) == wstring::npos) // no directory separator after the dot
    {
      result = TRUE;
      fileExtention = path.substr(lastDotPos + 1);
    }
  }
  if (!result)
    fileExtention.clear();
  return result;
}

void StripFileExt(OUT wstring &pathWithoutExtention, const wstring &path)
{
  size_t lastDotPos;
  BOOL isFileExtension;

  isFileExtension = FALSE;
  lastDotPos = path.rfind('.');
  if (lastDotPos != wstring::npos)
  {
    if (path.find('\\', lastDotPos) == wstring::npos) // no directory separator after the dot
    {
      isFileExtension = TRUE;
      pathWithoutExtention = path.substr(0, lastDotPos);
    }
  }
  if (!isFileExtension)
    pathWithoutExtention = path;
}

BOOL GetFileSizeByName( __int64 &fileSize, const wchar_t *path ) 
{
  HANDLE file;
  LARGE_INTEGER li;
  BOOL result;

  result = FALSE;
  fileSize = 0;
  file = CreateFile(path, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file != NULL)
  {
    if (GetFileSizeEx(file, &li))
    {
      fileSize = li.QuadPart;
      result = TRUE;
    }
    CloseHandle(file);
  }
  return result;
}

typedef struct 
{
  SCANPATH_CALLBACK userFunction;
  void *userParam;
  BOOL recursive;
} SCANPATH_PARAMS;

void PerformScanPath(const wstring &path, const SCANPATH_PARAMS &scanParams) 
{
  WIN32_FIND_DATA fd;
  wstring fileName;
  HANDLE hFindFile = FindFirstFile((path + L"\\*").c_str(), &fd );
  if (hFindFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      fileName = fd.cFileName;
      if (fileName != L"." && fileName != L"..") 
      {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
          if (scanParams.recursive)
            PerformScanPath(path + L'\\' + fileName, scanParams);
        }
        else 
        {
          if (!scanParams.userFunction(scanParams.userParam, path + L'\\' + fileName))
            break;
        }
      }
      if (!FindNextFile(hFindFile, &fd))
        break;
    } while (TRUE);
    FindClose(hFindFile);
  }
}

void ScanPath( const wchar_t *path, SCANPATH_CALLBACK userFunction, void *userParam )
{
  SCANPATH_PARAMS scanParams;
  scanParams.userFunction = userFunction;
  scanParams.userParam = userParam;
  scanParams.recursive = true;
  PerformScanPath(wstring(path), scanParams);
}

BOOL MyBrowseForFolder(wstring &strFolder, HWND hwndOwner, const wchar_t *szTitle, UINT ulFlags)
{
  BOOL result = FALSE;
  BROWSEINFO bi;
  wchar_t path[MAX_PATH];

  ZeroMemory(&bi, sizeof(BROWSEINFO));
  bi.hwndOwner = hwndOwner;
  bi.lpszTitle = szTitle;
  bi.ulFlags = ulFlags;
  LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
  if ( pidl != NULL )
  {
    if ( SHGetPathFromIDList ( pidl, path ) )
    {
      result = TRUE;
      strFolder = path;
    }
    IMalloc * imalloc = NULL;
    if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
    {
      imalloc->Free ( pidl );
      imalloc->Release ( );
    }
  }
  return result;
}

