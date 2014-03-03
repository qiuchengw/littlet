#pragma once

#include "UserInfo.h"
#include "inet/CSmtp.h"

// enum ENUM_AUTOTASK_DOWHAT_CUSTOM
// {
//     // 文件备份
//     AUTOTASK_DOWHAT_CUSTOM_AUTOBACKUP = AUTOTASK_DO_USERCUSTOM + 1,
// };

// class YuluATaskEventHandler : public DATaskEventHandlerBase
// {
// public:
//     virtual BOOL OnTaskFiredRequest(DATaskItem *pTask)
//     {
//         if (!pTask->IsUserCustomTask())
//             return TRUE;
// 
//         switch (pTask->DoWhat())
//         {
//         case AUTOTASK_DOWHAT_CUSTOM_AUTOBACKUP:
//             {
//                 break;
//             }
//         }
// 
//         // 我们自己处理
//         return FALSE;
//     }
// };

namespace littlet
{
    /*
     *	使用SMTP协议 + SSL 发送邮件
     *      
     *      返回值见：
     *          ECSmtp::CSmtpError
     */
    LRESULT SMTP_SendMail_SSL( 
        __in _CLIENT_MAIL& cmFrom, __in const QString& sSenderName,
        __in const QString& sTo, __in const QString& sToName, 
        __in const QString& sSubject, __in const QString& sContent,
        __in const QString& sAttachFile = L"");

    /**
     *  查找特定类型的自动任务 
     *      ENUM_AUTOTASK_DOWHAT/ ENUM_AUTOTASK_DOWHAT_CUSTOM
    **/
/*    DATaskItem* FindAutoTask(__in int nDoWhat);*/

//     inline DATaskItem* FindAutoTask_AUTOBACKUP()
//     {
//         return FindAutoTask(AUTOTASK_DOWHAT_CUSTOM_AUTOBACKUP);
//     }

}

// 使用邮箱做数据备份
class CFileBackup
{
public:
    CFileBackup(void);
    ~CFileBackup(void);

    /*
     *	立即备份
     */
    BOOL BackupNow();

    /*
     *  设置自动备份周期
     *	    dwPeriodMinute  最小30
     */
    void SetAutoBackup(__in BOOL bAuto = TRUE, __in DWORD dwPeriodMinute = 30);

protected:
public:
    /*
     *	备份文件到邮箱
     *      sFile   文件全路径
     */
    BOOL BackupToUserMail(__in const QString& sFile);
private:
    DWORD       period_;    
    BOOL        auto_backup_;
};

