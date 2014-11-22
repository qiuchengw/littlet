#pragma once

#include "ui/QDialog.h"
//#include "sys/Regkey.h"

// about dialog
class LAboutDlg : public QDialog
{
    SINGLETON_ON_DESTRUCTOR(LAboutDlg)
    {

    }

    QUI_DECLARE_EVENT_MAP

public:
    LAboutDlg(void);

    static void ShowModal();

protected:
    void OnClkLink(HELEMENT he);
};

// update dialog
class LUpdateInfoDlg : public QDialog
{
public:
    LUpdateInfoDlg(LPCWSTR szUpdateFile);

protected:
    virtual LRESULT OnDocumentComplete();

private:
    CStdString     m_sZipFile; // updater file
};

// setting dialog
class  LSettingDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP;

    SINGLETON_ON_DESTRUCTOR(LSettingDlg)
    {

    }
public:
    LSettingDlg();
    static void ShowModal();

protected:
    virtual LRESULT OnDocumentComplete();
    virtual BOOL OnDefaultButton(INT_PTR nBtn);
    void OnCmnChkAutoRun(HELEMENT);
    void OnCmnChkPinToTaskbar(HELEMENT);
    BOOL IsStartupShortcutExist();

protected:
    ENumber _BreakSec()
    {
        return GetCtrl("#input_break_sec");
    }

    ENumber _BreakSpan()
    {
        return GetCtrl("#input_break_span");
    }

    ENumber _BreakAlpha()
    {
        return GetCtrl("#input_break_alpha");
    }

    EFolderPath _BreakFolder()
    {
        return GetCtrl("#path_break_pic");
    }

    EColorPicker _BreakColor()
    {
        return GetCtrl("#clrp_break_bk");
    }

    ECheck _AutoRun()
    {
        return GetCtrl("#chk_cmn_autorun");
    }

    // 本程序的注册表路径
    // CStdStringW _KeyPath();
    CStdStringW _ProgPath();

private:
//    CXRegKey    m_reg;
};