#include "QADEnApp.h"
#include "apphelper.h"
#include "ui/QUIGlobal.h"
#include "../common/LittleTUIcmn.h"

QADEnApp theApp;
QADEnApp::QADEnApp(void)
{
	
}

QADEnApp::~QADEnApp(void)
{
}

BOOL QADEnApp::InitRun()
{
	SetMainWnd(&m_MainWnd);

	if (!m_MainWnd.Startup())
	{
		XMsgBox::ErrorMsgBox(L"´°Ìå´´½¨Ê§°Ü£¡");
		return FALSE;
	}

	m_MainWnd.ShowWindow(SW_SHOW);
	return TRUE;
}

QString QADEnApp::GetConfigPath()
{
	return qcwbase::GetModulePath() + L"Config.ini";
}


//////////////////////////////////////////////////////////////////////////
QADEnCfg theConfig;
QADEnCfg::QADEnCfg(void)
{
}

QADEnCfg::~QADEnCfg(void)
{
}

QString QADEnCfg::GetMainQSS()
{
	return L"ADEn.zip";
	
}
