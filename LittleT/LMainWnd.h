//#include "ui/transwnd/transdlg.h"
#include "LittleTFrame.h"

class LMainWnd : public CTransDlg
{
public:
    virtual QFrame* CreateRealWnd(HWND hWndParent);
    virtual void OnInitChildrenWnds();

    QFrame* GetFrame();

private:
    LittleTFrame  m_frame;    
};