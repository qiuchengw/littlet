#include "ProcessMan.h"
#include "file/FileOper.h"

QTime QProcessMan::sm_tmAppStart;
const QTime QProcessMan::sm_tmSystemStart =  (QTime::GetCurrentTime() 
	- QTimeSpan( (double)GetTickCount() / 86400000.0f )).IgnoreMillSecond();

QProcessMan::QProcessMan(void)
{
	m_idx = 0;
}

QProcessMan::~QProcessMan(void)
{
}

BOOL QProcessMan::SnapShot()
{
	m_vPsID.clear();
	m_idx = 0;
	// Enum all process
	DWORD arrPS[1024], cbNeeded;
	if ( !EnumProcesses( arrPS, sizeof(arrPS), &cbNeeded ) )
		return FALSE;
	// Calculate how many process identifiers were returned.
	for (int i = 0; i < (cbNeeded / sizeof(DWORD)); ++i)
	{
		m_vPsID.push_back(arrPS[i]);
	}
	return TRUE;
}

BOOL QProcessMan::GetProcessStartupTime( __in HANDLE hProcess ,__out QTime* pStart)
{
	if ((nullptr != pStart) && (NULL != hProcess))
	{
		FILETIME ftStart,ftExit,ftKenal,ftUser;
		if (GetProcessTimes(hProcess,&ftStart,&ftExit,&ftKenal,&ftUser))
		{
			*pStart =ftStart;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL QProcessMan::GetPsPath( __in HANDLE hProcess ,
	__out QString &sPath,__out QTime* pStart)
{
	BOOL bOK = FALSE;
	// Get the process name.
	if (NULL != hProcess )
	{
		TCHAR szProcessName[MAX_PATH * 2] = TEXT("<unknown>");
		if (GetModuleFileNameEx(hProcess,NULL,szProcessName,MAX_PATH * 2))
		{
			sPath = szProcessName;
			bOK = TRUE;
		}
		if (nullptr != pStart)
		{
			bOK = GetProcessStartupTime(hProcess,pStart);
		}
	}
	return bOK;
}

BOOL QProcessMan::GetPsPath( __in DWORD nProcessID, 
		__out QString &sPath,__out QTime* pStart)
{
	// Get a handle to the process.
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, nProcessID);
	if (NULL != hProcess)
	{
		BOOL bOK = GetPsPath(hProcess,sPath,pStart);
		CloseHandle( hProcess );
		return bOK;
	}
	return FALSE;
}

BOOL QProcessMan::GetPsPath( __in HWND hWnd, 
	__out QString &sPath,__out QTime* pStart)
{
	// Get a handle to the process.
	DWORD dwPID;
	GetWindowThreadProcessId(hWnd,&dwPID);
	return GetPsPath(dwPID,sPath,pStart);
}

DWORD QProcessMan::NextID()
{
	ASSERT(m_idx >= 0);
	if ( (GetPsNumber() <= 0) || (GetPsNumber() <= m_idx) )
		return INVALID_PROCESS_ID;
	return m_vPsID.at(m_idx++);
}

BOOL  QProcessMan::DebugPrivilege(BOOL   bEnable) 
{ 
	BOOL   bResult   =   TRUE; 
	HANDLE  hToken; 
	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,&hToken)) 
	{ 
		bResult   =   FALSE; 
	} 
	TOKEN_PRIVILEGES     tp; 
	tp.PrivilegeCount   =   1; 
	tp.Privileges[0].Attributes   =   bEnable? SE_PRIVILEGE_ENABLED:0; 
	LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid); 
	AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL); 
	if(GetLastError()   !=   ERROR_SUCCESS) 
	{ 
		bResult   =   FALSE; 
	} 
	CloseHandle(hToken); 
	return   bResult; 
} 

BOOL QProcessMan::IsExeRun(const QString & sExePath,__out QTime &tmRun) 
{ 
	BOOL  bFind   =   FALSE; 
	//提升进程权限 
	DebugPrivilege(TRUE); 
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); 
	if(hProcessSnap   ==   INVALID_HANDLE_VALUE) 
	{ 
		return   FALSE;
	} 

	QString sPath,sName = quibase::CPath(sExePath).GetFileName();
	PROCESSENTRY32     pe32; 
	pe32.dwSize   =   sizeof(PROCESSENTRY32); 
	if(Process32First(hProcessSnap,&pe32)) 
	{ 
		do 
		{ 
			if(0 == pe32.th32ProcessID)   //进程ID 
				continue;
			if ((sName == pe32.szExeFile) 
				&& GetPsPath(pe32.th32ProcessID,sPath,&tmRun)
				&& (sPath == sExePath))
			{ 
				bFind   =   TRUE; 
				break; 
			}
		} while(Process32Next(hProcessSnap,&pe32)); 
	} 
	CloseHandle(hProcessSnap); 
	//恢复进程权限 
	DebugPrivilege(FALSE); 
	return   bFind; 
} 

QTime QProcessMan::GetCurrentProcessStartupTime()
{
	if (sm_tmAppStart.m_status != QTime::invalid)
	{
		QProcessMan::GetProcessStartupTime(GetCurrentProcess(),&sm_tmAppStart);
		sm_tmAppStart.IgnoreMillSecond();
	}
	return sm_tmAppStart;
}

