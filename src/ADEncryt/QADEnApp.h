#pragma once

#include "ui/QApp.h"
#include "ui/QConfig.h"
#include "MainWnd.h"

#ifdef _DEBUG
#pragma comment(lib, "quil_d")
#else
#pragma comment(lib, "quil")
#endif

class QADEnApp : public QApp
{
public:
	QADEnApp(void);
	~QADEnApp(void);

	virtual BOOL InitRun();
	virtual CStdString GetConfigPath();

// 	BOOL SendCommandToMainWnd(WPARAM wParam,LPARAM lParam);
// 	BOOL SendNotifyToMainWnd(WPARAM wParam,LPARAM lParam);
// 	BOOL PostCommandToMainWnd(WPARAM wParam,LPARAM lParam);
// 	BOOL PostNotifyToMainWnd(WPARAM wParam,LPARAM lParam);
	
private:
	MainWnd		m_MainWnd;
};

class QADEnCfg : public QConfig
{
public:
	QADEnCfg(void);
	~QADEnCfg(void);

	virtual CStdString GetMainQSS();
};

