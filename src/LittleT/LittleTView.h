#pragma once

#include "ui/QView.h"
#include "ui/QUIGlobal.h"
#include "ui/ECtrl.h"

class LittleTView : public QView
{
    BEGIN_MSG_MAP_EX(LittleTView)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        CHAIN_MSG_MAP(QView)
    END_MSG_MAP()

public:
    void OnMouseHover( UINT , CPoint )
    {
        SetFocus();

        SetMsgHandled(FALSE);
    }

    void OnMouseMove( UINT , CPoint )
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER;
        tme.dwHoverTime = 10;
        tme.hwndTrack = GetSafeHwnd();

        _TrackMouseEvent(&tme);

        SetMsgHandled(FALSE);
    }

};