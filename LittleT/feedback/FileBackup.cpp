#include "FileBackup.h"
#include "ui/QConfig.h"
#include "ui/QUIGlobal.h"
//#include "../clock/DATaskMan.h"

namespace littlet
{
    LRESULT SMTP_SendMail_SSL( 
        __in _CLIENT_MAIL& cmFrom, __in const QString& sSenderName,
        __in const QString& sTo, __in const QString& sToName, 
        __in const QString& sSubject, __in const QString& sContent,
        __in const QString& sAttachFile)
    {
        ATL::CW2AEX<128> sbj(sSubject);
        CSmtp smtp;
        smtp.SetSubject(sbj);

        ATL::CW2AEX<128> srv(sTo);
        ATL::CW2AEX<128> srvName(sToName);
        smtp.AddRecipient(srv, srvName);

        // 连接到服务器
        do 
        {
            ATL::CW2AEX<128> host(cmFrom.host_);
            ATL::CW2AEX<128> user(cmFrom.user_);
            ATL::CW2AEX<128> pswd(cmFrom.pswd_);
            ATL::CW2AEX<128> mail(cmFrom.mail_);
            ATL::CW2AEX<128> sender_n(sSenderName);
            smtp.SetSenderMail(mail);
            smtp.SetSenderName(sender_n);

            if (smtp.ConnectRemoteServer(host, cmFrom.port_, USE_SSL, true, user, pswd))
                break;
        } while (true);

        // 发送
        ATL::CW2AEX<128> msg(sContent);
        smtp.AddMsgLine(msg);
        if (!sAttachFile.IsEmpty())
        {
            QString sTemp = sAttachFile;
            sTemp.Replace(L'/', L'\\');
            ATL::CW2AEX<128> fname(sTemp);
            smtp.AddAttachment(fname);
        }

        try
        {
            smtp.Send();
        }
        catch(ECSmtp& e)
        {
            return e.GetErrorNum();
        }

        return ECSmtp::CSMTP_NO_ERROR;
    }


    _USER_INFO* YuluGetUserInfo()
    {
        return YuluGetUserData()->GetUserInfo();
    }

    _UserData* YuluGetUserData()
    {
        static _UserData _S_UD_;

        return &_S_UD_;
    }

//     DATaskItem* FindAutoTask( __in int nDoWhat )
//     {
//         auto *p = DATaskMan::GetInstance()->GetAll();
//         for (auto i = p->begin(); i != p->end(); ++i)
//         {
//             if ((*i)->DoWhat() == nDoWhat)
//             {
//                 return *i;
//             }
//         }
//         return nullptr;
//     }

}

CFileBackup::CFileBackup(void)
{
    QUIConfig* pCfg = QUIGetConfig();
    auto_backup_ = pCfg->GetIntValue(L"APP", L"auto_backup");
    period_ = pCfg->GetIntValue(L"APP", L"auto_backup_period");
}

CFileBackup::~CFileBackup(void)
{
}

void CFileBackup::SetAutoBackup( __in BOOL bAuto /*= TRUE*/, 
    __in DWORD dwPeriodMinute /*= 30*/ )
{
    dwPeriodMinute = min(max(dwPeriodMinute, 30), 24 * 30 * 2);
    
    QUIConfig* pCfg = QUIGetConfig();
    auto_backup_ = bAuto;
    pCfg->SetValue(L"APP", L"auto_backup", bAuto ? 1 : 0);
    period_ = dwPeriodMinute;
    pCfg->SetValue(L"APP", L"auto_backup_period", period_);

    ASSERT(FALSE);

//     DATaskItem* pt = yulu::FindAutoTask_AUTOBACKUP();  // 自动备份的任务指针
//     if (bAuto)
//     {
//         if (nullptr != pt)
//         {
//             DATaskMan::GetInstance()->DeleteItem(pt);
//         }
// 
//     }
}

BOOL CFileBackup::BackupToUserMail(__in const QString& sFile)
{
    _USER_INFO *p = littlet::YuluGetUserInfo();
#ifdef _DEBUG
    p->cm_.mail_ = L"ziyue_yulu@163.com";
    p->cm_.pswd_ = L"ZaiHangzhou88";
    p->cm_.host_ = L"smtp.163.com";
    p->cm_.port_ = 465;
    p->cm_.user_ = L"ziyue_yulu";
#endif

    return littlet::SMTP_SendMail_SSL(p->cm_, APP_NAME, p->cm_.mail_, 
        L"Myself", APP_NAME + L"_backup_", L"File Backup", sFile);
}

