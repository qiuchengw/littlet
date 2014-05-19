#pragma once

#include "resource.h"

// xmsgbox
#ifndef QUI_USE_COLORBTN
#   define QUI_USE_COLORBTN
#endif

#include "ui/QApp.h"
//#include "LMainWnd.h"
#include "LittleTFrame.h"
#include "AppHelper.h"

#ifdef _DEBUG
#pragma comment(lib, "quil_d.lib")
#else
#pragma comment(lib, "quil.lib")
#endif

class LittleTApp : public QApp
{
public:
    LittleTApp();

    ~LittleTApp();

    virtual BOOL InitRun();

    virtual LPCWSTR GetAppVersion()const;

    BOOL CheckUpdate();

public:
    BOOL        m_bStartUpdater;
    CStdString     m_sUpdateFile;

protected:
    void StartUpdateExe();
    // 看看"更新程序“有没有更新
    void CheckUpdaterExe();

private:
    //    LMainWnd    m_MainFrm;
    LittleTFrame    m_frame;

    // 主窗口创建起来后，创建这个事件，此后可以用于和其他进程通信
    HANDLE      m_hEventMainWnd;
};


#include "ui/QConfig.h"

class LittleTConfig : public QUIConfig
{
public:
    // 休息一会儿的默认播放文件目录
    CStdString  GetPicFolder()
    {
        CStdString sRet = GetValue(L"setting",L"pic_folder");
        if (sRet.Find(L':') == -1)
        {   // 相对路径，转换为绝对路径
            return quibase::GetModulePath() + sRet;
        }
        return sRet;
    }
    // 休息一会儿的默认播放文件间隔时间
    int GetPicSec()
    {
        return GetIntValue(L"setting",L"pic_sec");
    }

    // 休息一会儿的默认显示时间
    int GetBreakSec()
    {
        return GetIntValue(L"setting",L"break_sec");
    }
    
    // 休息一会儿的默认透明度
    int GetPicAlpha()
    {
        return GetIntValue(L"setting",L"pic_alpha");
    }

    // 休息一会儿的默认背景色
    DWORD GetPicBkcolor()
    {
        return GetIntValue(L"setting",L"pic_bkcolor");
    }

    // 获取图标缩略图的目录
    CStdString GetIconsDir()
    {
        return quibase::GetModulePath() + L"icons/";
    }

public:
    void SetBreakFolder(LPCWSTR szFolder)
    {
        SetValue(L"setting", L"pic_folder", szFolder);
    }

    void SetBreakSec(int nSec)
    {
        SetValue(L"setting", L"break_sec", nSec);
    }

    void SetBreakSpan(int nSec)
    {
        SetValue(L"setting", L"pic_sec", nSec);
    }

    void SetBreakBkcolor(DWORD dwClr)
    {
        SetValue(L"setting", L"pic_bkcolor", dwClr);
    }

    void SetBreakAlpha(int nAlpha)
    {
        SetValue(L"setting", L"pic_alpha", nAlpha);
    }
};




