#ifndef GDIpInitializer_h__
#define GDIpInitializer_h__

#pragma once

#ifdef GDIPVER 
#   if GDIPVER < 0x0110
#       undef GDIPVER
#       define GDIPVER 0x0110
#   endif
#else
#   define GDIPVER 0x0110
#endif

#include <GdiPlus.h>

#pragma comment(lib,"GdiPlus")

class CGDIpInitializer 
{
	//! Constructor offers the ability to initialize on construction, or delay until needed.
	CGDIpInitializer(bool bInitCtorDtor = false): m_bInitCtorDtor(bInitCtorDtor), 
		m_bInited(false), m_hMap(NULL), m_gdiplusToken(NULL), 
		m_gdiplusStartupInput(NULL)
	{
		if (m_bInitCtorDtor) 
			Initialize();
	}

	//! If GDI+ has not explicitly been Deinitialized, do it in the destructor
	virtual ~CGDIpInitializer()
	{
		if (m_bInitCtorDtor) 
			Deinitialize();
	}

public:
    // 非线程安全的函数
    static CGDIpInitializer* Get()
    {
        static CGDIpInitializer _inst;

        return &_inst;
    }

	/*! \brief Initialize function, makes sure only one GDI+ is called for each process.

	This function creates a file mapping based on the current process id.
	If the mapping already exists, it knows that another instance of this class
	elsewhere in the process has already taken care of starting GDI+.
	*/
	void Initialize()
	{
		if (!m_bInited) 
		{
			TCHAR buffer[1024];
			_stprintf_s(buffer,1024, _T("GDIPlusInitID=%x"), GetCurrentProcessId());
			m_hMap = CreateFileMapping((HANDLE) INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, sizeof(long), buffer);
			if (m_hMap != NULL) 
			{
				// We might have a winner
				if (GetLastError() == ERROR_ALREADY_EXISTS) 
				{ 
					CloseHandle(m_hMap); 
				} 
				else 
				{
					// Yes, we have a winner
					m_bInited = true;
					Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
				}
			}
		}
		m_initcount++;
	}

	/*! \brief Deinitialize or decrease initialize count.

	No tricks to this function.  If this was the class that originally started GDI+,
	and its initialization count has reached zero, it shuts down GDI+.
	*/
	void Deinitialize()
	{
		using namespace Gdiplus;

		m_initcount--;
		if (m_bInited && m_initcount == 0) 
		{
			Gdiplus::GdiplusShutdown(m_gdiplusToken);
			CloseHandle(m_hMap);
			m_bInited = false;
		}
	}

private:
	HANDLE							m_hMap;
	bool							m_bInited, m_bInitCtorDtor;
	ULONG_PTR						m_gdiplusToken;
	Gdiplus::GdiplusStartupInput				m_gdiplusStartupInput;
	long						m_initcount;
};

// __declspec(selectany) long CGDIpInitializer::m_initcount;
 
#endif // GDIpInitializer_h__
