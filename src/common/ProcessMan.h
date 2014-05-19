#pragma once

#include "BaseType.h"
#include "time/QTime.h"
#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <tlhelp32.h>

#pragma comment(lib,"Psapi")

// 系统进程枚举器
class QProcessMan
{
public:
	QProcessMan(void);
	~QProcessMan(void);

	// 枚举之
	BOOL SnapShot();
	// 进程数目
	inline DWORD GetPsNumber()const { return pids_.size(); }
	// 遍历进程ID
	DWORD NextID();
	
	static BOOL GetPsPath( __in DWORD nProcessID, 
		__out CStdString &sPath ,__out QTime* pStart=NULL);
	static BOOL GetPsPath( __in HANDLE hProcess ,
		__out CStdString &sPath,__out QTime* pStart=NULL);
	static BOOL GetPsPath( __in HWND hWnd,
		__out CStdString &sPath,__out QTime* pStart);
	static BOOL GetProcessStartupTime( __in HANDLE hProcess ,
		__out QTime* pStart);
	static QTime GetCurrentProcessStartupTime();
	static QTime GetSystemStartupTime() { return sm_tmSystemStart; }
	static BOOL DebugPrivilege(BOOL bEnable);
	static BOOL IsExeRun(const CStdString & sExePath,__out QTime &tmRun);

private:
	typedef std::vector<DWORD> PIDs;
	PIDs	pids_;
	int		idx_;
	static const QTime		sm_tmSystemStart; // 系统启动时间
	static QTime		sm_tmAppStart;	// 本应用程序启动时间
};

