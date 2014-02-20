#pragma once

#include "ui/QWindow.h"
#include "QPictureLoader.h"

enum
{
    QSCREENWND_MIN_LIFESEC = 30,    // 默认最短显示时间是30s
    QSCREENWND_LIFETIMER_ID = 0x101,    //
    QSCREENWND_ANIMATIONTIMER_ID = 0x102,    //
    QSCREENWND_LIFETIMER_PERIOD = 1000,    // 1000MS
    QSCREENWND_ANIMATIONTIMER_PERIOD = 1000,    // 500MS
};

class QScreenWnd : public QFrame
{
    BEGIN_MSG_MAP_EX(QScreenWnd)
        BEGIN_QUI_MSG
            MSG_QUI_CODE(PICTRUELOADER_PLAYTIMER_FIRED,OnPlayerTimerFired)
        END_QUI_MSG
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_CLOSE(OnClose)
        CHAIN_MSG_MAP(QFrame)
    END_MSG_MAP()

    SINGLETON_ON_DESTRUCTOR(QScreenWnd)
    {

    }

    QScreenWnd();

public:
	static BOOL Show(LPCWSTR pszPicFolder, int nAlpha = 200,
        COLORREF crBk = RGB(88,93,245),int nSeconds = QSCREENWND_MIN_LIFESEC,
        int nPicPeriod = 3);

protected:
    void OnPlayerTimerFired(LPARAM lp);
    void OnTimer(UINT_PTR nTimerID);
    void OnKeyDown(TCHAR ch, UINT ,UINT);
    void OnClose();
    BOOL _Show(LPCWSTR pszPicFolder, int nAlpha = 200,
        COLORREF crBk = RGB(88,93,245),
        int nSeconds = QSCREENWND_MIN_LIFESEC, int nPicPeriod = 3);

    inline QString _ImgPath()
    {
        return qcwbase::GetModulePath() + L"__ILoveUButIamSorry.png";
    }
private:
    QPictureLoader      m_pic;
    int                 m_nLifeSec;     // 显示多少秒
    int                 m_nPeriodSec;   // 图片播放频率
    int                 m_iAnimation;   
    ECtrl               m_ctlPic;
};

