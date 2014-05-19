#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

#define ARRAYDIM(x) (sizeof(x)/sizeof(x[0]))

class out_of_resources :
  public exception
{
public:
  out_of_resources() : exception("Out of resources") { ; }
};

namespace str
{
  const wstring &EmptyString();
  size_t SizeInBytes(const wstring &str);
  size_t SizeInBytes(const wchar_t *sz);
  wstring Format1K(LPCTSTR formatString, ...);
  wstring ToUpper(const wstring &str);
  void AssignSZ(wstring &s, const wchar_t *buffer, size_t maxChars);
  void AssignSZBytes(wstring &s, const void *buffer, size_t maxBytes);
  void MB2Unicode(wstring &unicodeString, const char *mbString, int length = -1, UINT codePage = CP_ACP);
  void Unicode2MB(vector<char> &mb, const wchar_t *unicodeString, UINT codePage);
  wstring IntToStr(int n);
  wstring SizeToStr(size_t n);
  wstring Int64ToStr(__int64 n);
  size_t split(vector<wstring> &items, const wstring &s, const wchar_t delimiter);
  int CompareI(const wstring &s1, const wstring &s2) /* case insensitive compare */;
  BOOL EqualI(const wstring &s1, const wstring &s2) /* true if equal in case insensitive compare */;
  BOOL StartingWithI(const wstring &s, const wstring &start);
  BOOL StartingWith(const wstring &s, const wstring &start);
}

namespace win
{
  void GetWindowText(wstring &text, HWND handle);
  void GetDlgItemText(wstring &text, HWND dialogHandle, int itemId);
  void SetDlgItemText(HWND dialogHandle, int itemId, const wstring &text);
}

namespace reg
{
  BOOL GetValue( vector<BYTE> &buffer, HKEY key, const wchar_t *name );
  BOOL SetBinary(HKEY key, const wchar_t *name, const void *buffer, size_t bufferSize);
  BOOL GetDword( OUT DWORD &val, HKEY key, const wchar_t *name );
  BOOL SetDword(HKEY key, const wchar_t *name, DWORD val);
  BOOL GetString(OUT wstring &str, HKEY key, const wchar_t *name);
  BOOL SetString(HKEY key, const wchar_t *name, const wchar_t *str);
  BOOL GetBool( OUT BOOL &val, HKEY key, const wchar_t *name );
  BOOL SetBool(HKEY key, const wchar_t *name, BOOL val);
}

template<class K, class V>
class MapInitializer
{
  map<K,V> m;
public:
  operator map<K,V>() const 
  { 
    return m; 
  }

  MapInitializer& Add( const K& k, const V& v )
  {
    m[ k ] = v;
    return *this;
  }
};

class CSafeAccess 
{
public:
  CSafeAccess(LPCRITICAL_SECTION cs)
  {
    m_cs = cs;
    EnterCriticalSection(m_cs);
  }
  ~CSafeAccess()
  {
    Leave(); 
  }
  void Leave()
  {
    if (m_cs != NULL)
    {
      LeaveCriticalSection(m_cs);
      m_cs = NULL;
    }
  }

private:
  LPCRITICAL_SECTION m_cs;
};

void ExtractFilePath(wstring &pathOnly, const wstring &path) /* returns path to file (without the last slash) */;
void ExtractFileName(wstring &fileNameOnly, const wstring &path); /* returns file name including extension */
BOOL ExtractFileExt(OUT wstring &fileExtention, const wstring &path); /* returns file extension */
void StripFileExt(OUT wstring &pathWithoutExtention, const wstring &path);
BOOL GetFileSizeByName( __int64 &fileSize, const wchar_t *path );

template<typename Type> Type *vector_ptr(vector<Type> &v)
{
  return &v[0];
}
template<typename Type> const Type *const_vector_ptr(const vector<Type> &v)
{
  return &v[0];
}
template<typename Type> size_t vector_bytes(const vector<Type> &v)
{
  return v.size() * sizeof(Type);
}
template<typename Type> void vector_append(vector<Type> &v, const Type *ptr, size_t count)
{
  size_t oldSize;

  oldSize = v.size();
  v.resize(v.size() + count);
  memcpy(&v[oldSize], ptr, count * sizeof(Type));
}

typedef BOOL (*SCANPATH_CALLBACK)(void *userParam, const wstring &fileName);
void ScanPath(const wchar_t *path, SCANPATH_CALLBACK userFunction, void *userParam);
BOOL MyBrowseForFolder(wstring &strFolder, HWND hwndOwner, const wchar_t *szTitle, UINT ulFlags);
