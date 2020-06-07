#include "LMainWnd.h"

QFrame* LMainWnd::CreateRealWnd(HWND hWndParent)
{
    if ( !m_frame.IsWindow() )
    {
        m_frame.Create(hWndParent,WS_POPUP|WS_VISIBLE,0,0);
    }

    if ( m_frame.IsWindow() )
    {
        return &m_frame;
    }

    ASSERT(FALSE);
    return NULL;
}

void LMainWnd::OnInitChildrenWnds()
{

}

QFrame* LMainWnd::GetFrame()
{
    return &m_frame;
}

