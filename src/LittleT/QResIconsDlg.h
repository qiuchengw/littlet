#pragma once

#include "ui/QDialog.h"

class QResIconsDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP

public:
    QResIconsDlg(void);
    ~QResIconsDlg(void);

protected:
    virtual LRESULT OnDocumentComplete();

    void OnIconSelChanged(HELEMENT);
    void OnAddNewIcon(HELEMENT he);

    // nIconID  为图标在数据库中的ID
    // szIconPath 为已经转换好的小图标的路径
    BOOL AddIcon(int nIconID, LPCWSTR szIconPath );

private:
    ECtrl _IconBox()
    {
        return GetCtrl("#icon-xbox");
    }

public:
    CStdString     m_sIconPath;
    int         m_nIconID;
};

