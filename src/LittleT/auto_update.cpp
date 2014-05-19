#include "auto_update.h"
#include "ui/QUIGlobal.h"
#include "ui/QConfig.h"
#include "ui/QApp.h"
#include "inet/WinHttpClient.h"
#include "crypt/QDataEnDe.h"

//////////////////////////////////////////////////////////////////////////
QAutoUpdater::QAutoUpdater(void)
{
    //m_bEnd = FALSE;
    m_hTimer = NULL;
}

VOID CALLBACK QAutoUpdater::CheckUpdateCallback(
    __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired)
{
    QAutoUpdater* pThis = reinterpret_cast<QAutoUpdater*>(lpParameter);
    if (NULL == pThis)
    {
        ASSERT(FALSE);
        return ;
    }
    pThis->CheckUpdate();
//    pThis->CheckBaseData();
}

BOOL QAutoUpdater::CheckUpdate( )
{
    _Download_Param *p = new _Download_Param(TASK_TYPE_UPDATION);
    p->sURL = url_updation_.url_;
    p->sRefer = url_updation_.domain_;
    p->pData = this;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL,0, 
        &QAutoUpdater::thread_download,p,0,NULL);
    if (hThread > 0)
    {
        CloseHandle(hThread);
        return TRUE;
    }
    delete p;
    return FALSE;
}

// BOOL QAutoUpdater::CheckBaseData( )
// {
//     _Download_Param *p = new _Download_Param(TASK_TYPE_UPDATION);
//     p->sURL = url_basedata_.url_;
//     p->sRefer = url_basedata_.domain_;
//     p->pData = this;
//     HANDLE hThread = (HANDLE)_beginthreadex(NULL,0, 
//         &QAutoUpdater::thread_download,p,0,NULL);
//     if (hThread > 0)
//     {
//         CloseHandle(hThread);
//         return TRUE;
//     }
//     delete p;
//     return FALSE;
// }

BOOL IsHigherVersion(CStdString& sVer, BOOL IsUpdation)
{
    // 版本号应该只带一个小数点
    if (IsUpdation) 
    {
        return _wtof(sVer.Trim()) > _wtof(QUIGetAppVersion());
    }

    // 从配置文件中读取
    CStdString str = QUIGetConfig()->GetValue(L"APP",L"base_data_ver").Trim();

    // 如果本地文件不存在，那么也认为有更新的
    return (_wtof(sVer.Trim()) > _wtof(str)) 
        || !quibase::IsFileExist(__BasedataCacheFile());
}

// J记录版本号
void SaveBaseDataVersion(const CStdString& sVer)
{
    QUIGetConfig()->SetValue(L"APP",L"base_data_ver", sVer);
}

BOOL updation_ExtractVerAndUrl(__in CStdString& sHtml, __out CStdString& sVersion,
    __out CStdString& sFileUrl)
{
    // 这段代码真恶心啊，不会正则表达式啊
    // version
    int ib = sHtml.Find(L"[[[ver:");
    if (-1 == ib)
        return FALSE;
    ib += 7;
    int ie = sHtml.Find(L"]]]", ib);
    if (-1 == ie)
        return FALSE;
    sVersion = sHtml.Mid(ib, ie-ib);

    // file
    sHtml = sHtml.Mid(ie);
    ib = sHtml.Find(L"--begin--");
    if (-1 == ib)
        return FALSE;
    ib = sHtml.Find(L"real_src",ib);
    if (-1 == ib)
        return FALSE;
    ib += 8;
    ie = sHtml.Find(L"&amp;", ib);
    if (-1 == ie)
        return FALSE;
    sFileUrl = sHtml.Mid(ib, ie-ib);
    ib = sFileUrl.Find(L"http://");
    if (-1 == ib)
        return FALSE;
    sFileUrl = sFileUrl.Mid(ib);
    return TRUE;
}

BOOL basedata_ExtractVerAndUrl(__in CStdString& sHtml, __out CStdString& sVersion,
    __out CStdString& sFileUrl)
{
    // 这段代码真恶心啊，不会正则表达式啊
    // version
    int ib = sHtml.Find(L"[[[ver:");
    if (-1 == ib)
        return FALSE;
    ib += 7;
    int ie = sHtml.Find(L"]]]", ib);
    if (-1 == ie)
        return FALSE;
    sVersion = sHtml.Mid(ib, ie-ib);

    // file
    sHtml = sHtml.Mid(ie);
    ib = sHtml.Find(L"--begin--");
    if (-1 == ib)
        return FALSE;
    ib = sHtml.Find(L"real_src",ib);
    if (-1 == ib)
        return FALSE;
    ib += 8;
    ie = sHtml.Find(L"&amp;", ib);
    if (-1 == ie)
        return FALSE;
    sFileUrl = sHtml.Mid(ib, ie-ib);
    ib = sFileUrl.Find(L"http://");
    if (-1 == ib)
        return FALSE;
    sFileUrl = sFileUrl.Mid(ib);
    return TRUE;
}

BOOL DecryptPictureData(__in QBuffer& bufEncrypt, __out QBuffer& bufData)
{
    DWORD dwPicSize = bufEncrypt.GetBufferLen();
    if (dwPicSize < 2048)
    {
        return FALSE;
    }

    // 最后写入20个字节，顺序如下
    // 密钥在图像数据中的偏移
    DWORD dwOffset;
    bufEncrypt.ReadLast((BYTE*)&dwOffset,sizeof(DWORD));
    // 密钥长度
    DWORD dwKeyLen;
    bufEncrypt.ReadLast((BYTE*)&dwKeyLen,sizeof(DWORD));
    // 图像大小
    bufEncrypt.ReadLast((BYTE*)&dwPicSize,sizeof(DWORD));
    // 加密后的数据长度
    DWORD dwEncrypDataLen;
    bufEncrypt.ReadLast((BYTE*)&dwEncrypDataLen,sizeof(DWORD));
    // 原始数据长度
    DWORD dwOriginLen;
    bufEncrypt.ReadLast((BYTE*)&dwOriginLen,sizeof(DWORD));

    if (!bufData.AllocBuffer(dwEncrypDataLen))
    {
        return FALSE;
    }
    if (!bufData.Write(bufEncrypt.GetBuffer(dwPicSize),dwEncrypDataLen))
    {
        // eInfo.SetText(L"读取数据错误");
        return FALSE;
    }

    QDataEnDe ende;
    if (!ende.SetCodeData(bufEncrypt.GetBuffer(dwOffset),dwKeyLen))
    {
        return FALSE;
        // eInfo.SetText(L"设置密码数据失败");
    }
    if (!ende.DecryptData(bufData))
    {
        // eInfo.SetText(L"解密数据失败!");
        return FALSE;
    }

    // eInfo.SetText(L"全部搞定啦！已经保存为：" + sFileName);
    return TRUE;
}

BOOL DecryptUpdateFile(__in QBuffer& bufEncrypt, __in const CStdString& sSavePath)
{
    QBuffer bufData;
    if (DecryptPictureData(bufEncrypt, bufData))
    {
        return (bufData.FileWrite(sSavePath));
    }
    return FALSE;
}

UINT_PTR __stdcall QAutoUpdater::thread_download( void* pparam )
{
    _Download_Param *p = (_Download_Param*)pparam;
    if (NULL == p)
    {
        ASSERT(FALSE);
        return 0;
    }

    BOOL bOK = FALSE;
    WinHttpClient cl(p->sURL);
    //WinHttpClient cl(L"http://blog.sina.com.cn/s/blog_936e65db0101n2y7.html");
    cl.SetUserAgent(L"Mozilla/5.0 (Windows NT 6.1; WOW64) "
        L"AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.75 Safari/535.7");
    cl.SetReferrer(p->sRefer);
    //cl.SetReferrer(L"http://blog.sina.com.cn");

    // 先检查是否有更新
    CStdString sVer = L"0", sFileUrl;
    for (int iTry = 0; iTry < 3; iTry++)
    {
        if (!cl.SendHttpRequest())
            continue;

        // 下载数据了
        CStdString sHtml = cl.GetResponseContent().c_str();
        if (p->IsUpdationTask())
        {
            // 自动更新
            bOK = updation_ExtractVerAndUrl(sHtml, sVer, sFileUrl);
        }
        else
        {
            // 基础数据
            bOK = basedata_ExtractVerAndUrl(sHtml, sVer, sFileUrl);
        }
        break;
    }

    CStdString sSavePath;
    BOOL bHasUpdate = IsHigherVersion(sVer, p->IsUpdationTask());
    if (!bOK || !bHasUpdate)
    {
        delete p;
        return 0;
    }

    bOK = FALSE;
    // 试三次去下载数据
    for (int iTry = 0; iTry < 3; iTry++)
    {
        cl.UpdateUrl(sFileUrl);
        if (!cl.SendHttpRequest())
            continue;

        // 下载数据了
        p->bufDown.ClearBuffer();
        p->bufDown.Write(cl.GetRawResponseContent(),
            cl.GetRawResponseReceivedContentLength());
        bOK = TRUE;
        break;
    }

    // 下载成功
    if (bOK)
    {
        // 程序更新的话，将数据写到临时文件中
        if (p->IsUpdationTask())
        {
            sSavePath = __UpdationTempFile();
            bOK = DecryptUpdateFile(p->bufDown, sSavePath);
        }
        // 基础数据就直接写下来就好了，不要解密，嘿嘿，这是秘密啊
        else
        {
            sSavePath = __BasedataCacheFile();
            bOK = p->bufDown.FileWrite(sSavePath);
            if (bOK)
            {
                SaveBaseDataVersion(sVer);
            }
        }
    }

    if (bHasUpdate)
    {
        LPQUI_USERMSGPARAM pMsg = QUIGetUserMsgParam(
            (bOK) ? 1 : 0,   // 下载更新是否完成
            p->IsUpdationTask() ? 0 : 1, 
            NULL,
            sSavePath, // 更新文件的地址
            TRUE);  // 接收者应该释放内存
        // 通知程序更新
        ::PostMessage(QUIGetMainWnd(), QSOFT_MSG_UPDATEAPP, 
            1, // 有更新
            (LPARAM)pMsg);
    }
    else
    {
        // 通知程序更新
        ::PostMessage(QUIGetMainWnd(), QSOFT_MSG_UPDATEAPP, 
            0, // 没有更新
            0);
    }
    delete p;

    return 0;
}

BOOL QAutoUpdater::Startup( __in _Url& urlUpdation, 
    __in _Url& urlBaseData, __in DWORD dwCheckPeriod /*= 30*/ )
{
    if (NULL != m_hTimer)
    {
        ASSERT(FALSE);  
        return TRUE;
    }

    url_updation_ = urlUpdation;
    //url_basedata_ = urlBaseData;

    dwCheckPeriod = max(10, dwCheckPeriod);

    // 启动15秒的时候先检查一次更新
    return CreateTimerQueueTimer(&m_hTimer, NULL, CheckUpdateCallback, this,
        15 * 1000, dwCheckPeriod * 60 * 1000, WT_EXECUTEDEFAULT);
}


// BOOL QAutoUpdater::Startup( __in DWORD dwCheckPeriod /*= 30*/ )
// {
//     if (NULL != m_hTimer)
//     {
//         ASSERT(FALSE);  
//         return TRUE;
//     }
// 
// //     QUIConnectCenter*pCC = QUIConnectCenter::GetInstance();
// // 
// //     if (pCC->GetUpdationUrl(url_updation_) 
// //         && pCC->GetUserUrl(url_basedata_))
//     {
//         dwCheckPeriod = max(10, dwCheckPeriod);
//         
//         // 启动15秒的时候先检查一次更新
//         return CreateTimerQueueTimer(&m_hTimer, NULL, CheckUpdateCallback, this,
//             15 * 1000, dwCheckPeriod * 60 * 1000, WT_EXECUTEDEFAULT);
//     }
//     return FALSE;
// }


