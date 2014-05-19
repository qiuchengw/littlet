#pragma once

#include "ui/QWindow.h"
#include "ui/QWindow.h"
#include "inet/QWebAD.h"
#include "inet/WinHttpClient.h"

#define MAX_DOWNLOADTHREAD_NUM 5

enum ENUM_PANE_WND
{
	PANE_WND_PAPER,
	PANE_WND_BOOK,
};

class MainWnd : public QFrame
{
	QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(MainWnd)
        MSG_WM_CLOSE(OnClose)
        CHAIN_MSG_MAP(QFrame)
    END_MSG_MAP()
public:
	MainWnd(void);
	~MainWnd(void);
	BOOL Startup();
	virtual BOOL OnClose();

protected:
	virtual BOOL on_key(HELEMENT he, HELEMENT target,
		UINT event_type, UINT code, UINT keyboardStates );

	void OnBtnEncrypt(HELEMENT);
	void OnBtnDecrypt( HELEMENT );
	BOOL ZipData(__in const CStdString&sDataPath,__out QBuffer &bufData);
};

