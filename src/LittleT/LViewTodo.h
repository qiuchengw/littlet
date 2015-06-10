#include "ui/QForm.h"
#include "ui/QFrame.h"
#include "LittleTView.h"
#include "../common/QDBHelper.h"
#include "ui/WndHelper.h"

class LFormTodo : public QForm
{
    QUI_DECLARE_EVENT_MAP;

public:
    ~LFormTodo();

public:
    BOOL ShowTask( ENUM_TODO_STATUS eStatus );
    void OnClkNewTask( HELEMENT hBtn );

protected:
    //virtual LRESULT OnDocumentComplete();
    virtual void OnAttach();
    
    BOOL RefreshTasks();
    void InsertTask(ECtrl& eGroup, TTodoTask* pTask);
    void FreshTaskItem(ECtrl& eGroup, ECtrl &eItem, TTodoTask* pTask);

    void OnClkDeleteTask(HELEMENT hBtn);
    void OnClkStickyNote(HELEMENT hBtn);
    void OnClkTaskChk(HELEMENT hBtn);
    void OnClkPriority(HELEMENT );
    void OnClkTodoItem(HELEMENT );
    void OnClkDoit(HELEMENT );
    void OnClkPlus5Minutes(HELEMENT );
    void OnClkMinus5Minutes(HELEMENT );

    void OnClkIdbarOK(HELEMENT );
    void OnClkIdbarCancel(HELEMENT );
    void OnTodoShow(HELEMENT he);
    void RefreshTaskNum();

    ECtrl _PopupBar() { return GetCtrl("#id_popup_todoitem"); };
    ECtrl _TodoList() { return GetCtrl("#todolist"); }
    void ShowPopupBar(TTodoTask &t,BOOL bEdit,HELEMENT he=NULL);

private:
    ECtrl					m_eItemEdit;
};

class LViewTodo : public LittleTView
{
    BEGIN_MSG_MAP_EX(LViewTodo)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_DESTROY(OnDestroy)
        CHAIN_MSG_MAP(LittleTView)
    END_MSG_MAP()

    QUI_DECLARE_EVENT_MAP;

public:
    LViewTodo();

    void OnClkNewTask( HELEMENT hBtn )
    {
        m_formTodo.OnClkNewTask(hBtn);
    }

protected:
    void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);
    void OnDestroy();

    virtual LRESULT OnDocumentComplete() override;

private:
    LFormTodo       m_formTodo;
};

class QDesktopEmbedFrame : public QFrame
{
    BEGIN_MSG_MAP_EX(LViewTodo)
        CHAIN_MSG_MAP(QFrame)
    END_MSG_MAP()

public:
    QDesktopEmbedFrame(LPCWSTR szResName=NULL)
        :QFrame(szResName)
    {

    }

    BOOL Create(__in int nAlpha = 200)
    {
        HWND hWndProgram = FindWindow(_T("Progman"), _T("Program Manager"));
        if(NULL != hWndProgram)
        {
            HWND hWndShellDLL = FindWindowEx(hWndProgram, NULL,
                _T("SHELLDLL_DefView"), NULL);
            if(NULL != hWndShellDLL)
            {
                if (QFrame::Create(hWndShellDLL, WS_POPUP | WS_VISIBLE, WS_EX_LAYERED, 0))
                {
                    // 设置Z-order，最下层显示
                    SetWindowPos(HWND_BOTTOM, 0, 0, 0, 0,
                        SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

                    quibase::TransparentWindow(GetSafeHwnd(), nAlpha);

                    return TRUE;
                }
            }
        }
        return FALSE;
    }

    BOOL CheckParent()
    {
        if (!IsWindow())
        {
            ASSERT(FALSE);
            return FALSE;
        }

        HWND hWndProgram = FindWindow(_T("Progman"), _T("Program Manager"));
        if(hWndProgram != NULL)
        {
            HWND hWndShellDLL = FindWindowEx(hWndProgram, NULL,
                _T("SHELLDLL_DefView"), NULL);
            if((hWndShellDLL != NULL) && (hWndShellDLL != GetParent()))
            {
                SetWindowLong(GWL_HWNDPARENT, (LONG)hWndShellDLL);

                // 设置Z-order，最下层显示
                SetWindowPos(HWND_BOTTOM, 0, 0, 0, 0,
                    SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

                return TRUE;
            }
        }
        return FALSE;
    }

};

class LDesktopTodo : public QDesktopEmbedFrame
{
    SINGLETON_ON_DESTRUCTOR(LDesktopTodo)
    {
        
    }

    BEGIN_MSG_MAP_EX(LViewTodo)
        MSG_WM_KEYDOWN(OnKeyDown)
        CHAIN_MSG_MAP(QDesktopEmbedFrame)
    END_MSG_MAP()

   QUI_DECLARE_EVENT_MAP;

public:
    LDesktopTodo()
        :QDesktopEmbedFrame(L"qabs:main/todo/index.htm")
    {

    }

protected:
    void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);

    virtual LRESULT OnDocumentComplete();

private:
    LFormTodo       m_formTodo;
};

