#pragma once

#include "BaseType.h"
#include "QBuffer.h"
#include "ui/QWindow.h"
#include "inet/http/http_request_manager.h"
#include "AppHelper.h"
#include "ui/QUIMgr.h"
#include "file/rjson/inc.h"
#include "UserInfo.h"

#define UPDATION_TEMP_FILE L"__temp_update.zip"

inline QString __UpdationTempFile()
{
    return quibase::GetModulePath() + UPDATION_TEMP_FILE;
}

#define BASEDATA_CACHE_FILE L"__basedata.dat"

inline QString __BasedataCacheFile()
{
    return quibase::GetModulePath() + BASEDATA_CACHE_FILE;
}


/**
*	数据保存在图片中	
*
*	@param
*		-[in]
*          bufEncrypt      图片数据，已加密
*		-[out]
*          bufData         解密后的数据
**/
BOOL DecryptPictureData(__in QBuffer& bufEncrypt, __out QBuffer& bufData);

struct _Url 
{
    QString     url_;
    QString     domain_;
};

#define UIBASE_MAILPSWD_PATH L"_qiuinfo.json"

// 网络连接中心 + 消息中心（隐藏的窗口）
class QUIConnectCenter : 
    public FCHttpRequestManager,
    public CWindowImpl<QUIConnectCenter>
{
    friend class QUserFeedbackWnd;

    enum ENUM_DOWNLOAD_TYPE
    {
        DOWNLOAD_TYPE_QUI = 0,  // QUIBase 需要   
        DOWNLOAD_TYPE_APP,  // 应用程序需要
        DOWNLOAD_TYPE_USER,     // 用户需求
    };

    BEGIN_MSG_MAP_EX(QUIConnectCenter)
        MSG_WM_CLOSE(OnClose)
    END_MSG_MAP()

    SINGLETON_ON_DESTRUCTOR(QUIConnectCenter)
    {
        if (IsWindow())
        {
            DestroyWindow();

            m_hWnd = NULL;
        }
    }

protected:
    // 从UI.zip 的 url.x 文件提取网络地址数据
    //  1，自动更新地址
    //  2，用户数据（BaseData）地址
    //  3，用户统计地址
    class _QUrlExtract
    {
        typedef std::vector<_Url>   LstAddr;

    public:
        // 提取
        BOOL Init();

        BOOL GetUpdationUrl(__inout _Url& adr)
        {
            return GetAddr(update_url_, adr);
        }

        BOOL GetUserUrl(__inout _Url& adr)
        {
            return GetAddr(user_url_, adr);
        }

        // 用户统计页面
        QString GetStaticsUrl()const
        {
            return stat_url_;
        }

    protected:
        BOOL GetAddr(__in LstAddr& lst, __inout _Url& adr);

    private:
        LstAddr     update_url_;    // 自动更新的地址
        LstAddr     user_url_;    // 用户需要的数据
        QString     stat_url_;  // 用户统计
    }ui_url_;

protected:
    /**
    *  从网络下载下来的BaseData数据包
    *
    *	此zip中包含的数据应该有	
    *      1，用户反馈、bug提交所需要的client、web数据
    *      2，广告数据（可选）
    **/
    class _QBaseData : public QUIResource
    {
    public:
        // 设置原始的加密数据
        BOOL SetBaseData(__in QBuffer& bufEncrypt)
        {
            ReleaseAll(NULL);

            QBuffer buf;
            if (!DecryptPictureData(bufEncrypt, buf))
                return FALSE;

            return QUIResource::LoadFromZip(buf);
        }

        virtual void OnReleaseResource(__in const QString& szResName, 
            __in const QString& szResIndicator)
        {

        }

        BOOL LoadDataFile(__in const QString& sFile = __BasedataCacheFile())
        {
            QBuffer buf;
            if (buf.FileRead(sFile))
            {
                return SetBaseData(buf);
            }
            return FALSE;
        }

    }basedata_;

protected:
    // 用于保存 意见反馈/bug提交 所需要的邮件、密码，网络地址等
    class _QiuInfo
    {
    public:
        typedef std::vector<QString> LstAddr;
        typedef std::vector<_CLIENT_MAIL> LstCM;

    public:
        /**
        *	设置地址文件，解析之	
        *
        *	@return
        *	    -1      	解析错误
        *      0           解析的数据为空
        *      >0          解析的数据
        *	@param
        *		-[in]
        *          sFile       json 数据
        *
        **/
        int SetAddrData(__in const QString& sJsonData) throw()
        {
            // 文件格式
            // {
            //     "server":
            //     {
            //         "web":
            //         [
            //             "http://www.baidu.com",
            //             "http://www.qiuchengw.com"
            //         ]
            //         ,
            //             "email":
            //         [
            //             "qiuchengw@qq.com",
            //             "qiuchengw@163.com"
            //         ]
            //     },
            //         "client":
            //     {
            //         "ziyue_yulu@163.com":
            //         {
            //             "pswd":"ZaiHangzhou88",
            //                 "host":"smtp.163.com",
            //                 "port":465,
            //                 "user":"ziyue_yulu"
            //         }
            //     }
            // }
            RJsonDocumentW d;
            d.Parse<0>(sJsonData);
            if (d.HasParseError())
                return FALSE;

            // servers
            RJsonValueW& srv = d[L"server"];
            if (srv.IsObject())
            {
                // web
                RJsonValueW& web = srv[L"web"];
                if (web.IsArray())
                {
                    webs_.clear();
                    for (auto i = web.Begin(); i != web.End(); ++i)
                    {
                        webs_.push_back((*i).GetString());
                    }
                }

                // mails
                RJsonValueW& mail = srv[L"email"];
                if (mail.IsArray())
                {
                    mails_.clear();
                    for (auto i = mail.Begin(); i != mail.End(); ++i)
                    {
                        mails_.push_back(i->GetString());
                    }
                }
            }

            // client
            RJsonValueW& cmail = d[L"client"];
            if (cmail.IsObject())
            {
                client_mail_.clear();
                for (auto i = cmail.MemberBegin(); i != cmail.MemberEnd(); ++i)
                {
                    RJsonValueW& v = i->value;
                    _CLIENT_MAIL cm;
                    cm.mail_ = i->name.GetString();
                    cm.host_ = v[L"host"].GetString();
                    cm.user_ = v[L"user"].GetString();
                    cm.pswd_ = v[L"pswd"].GetString();
                    cm.port_ = v[L"port"].GetInt();
                    client_mail_.push_back(cm);
                }
            }

            return mails_.size() + webs_.size() + client_mail_.size();
        }

        /**
        *	获取最合适的mail地址	
        *
        *	@param
        *		-[inout]
        *          adr     如果不为空，先在地址中查找它，然后返回排在它下面的那个那个地址
        *                  如果为空，获取mail列表中最前面的那个地址
        **/
        inline BOOL GetMailAddr(__inout QString& adr)
        {
            return GetAddr(mails_, adr);
        }

        inline BOOL GetWebAddr(__inout QString& adr)
        {
            return GetAddr(webs_, adr);
        }

        /**
        *	获取用于发送邮件的服务器
        *
        *	@param
        *		-[inout]
        *          cm
        **/
        BOOL GetClientMail(__inout _CLIENT_MAIL& cm)
        {
            if (client_mail_.empty())
                return FALSE;

            if (cm.mail_.IsEmpty())
            {
                cm = client_mail_.front();
                return TRUE;
            }

            auto iEnd = client_mail_.end();
            for (auto i = client_mail_.begin(); i != iEnd; ++i)
            {
                if (cm.mail_.CompareNoCase((*i).mail_) == 0)
                {
                    if (++i != iEnd)
                    {
                        cm = *i;
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }

    protected:
        BOOL GetAddr(__in LstAddr& lst, __inout QString& adr)
        {
            if (lst.empty())
                return FALSE;

            if (adr.IsEmpty())
            {
                adr = lst.front();
                return TRUE;
            }

            auto iEnd = lst.end();
            for (auto i = lst.begin(); i != iEnd; ++i)
            {
                if (adr.CompareNoCase(*i) == 0)
                {
                    if (++i != iEnd)
                    {
                        adr = *i;
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }

    protected:
        LstAddr     mails_;
        LstAddr     webs_;
        LstCM       client_mail_;
    }qiu_info_;

public:
    // 必须初始化后才能使用
    BOOL Init();

    _QiuInfo* GetQiuInfo()
    {
        return &qiu_info_;
    }

    inline BOOL IsInited()const
    {
        return IsWindow();
    }

    BOOL GetUpdationUrl(__inout _Url& adr)
    {
        return ui_url_.GetUpdationUrl(adr);
    }

    BOOL GetUserUrl(__inout _Url& adr)
    {
        return ui_url_.GetUserUrl(adr);
    }

    /*
    *	用户意见反馈、bug提交所需要的网络数据
    */
    BOOL LoadBaseData(__in const QString& sBaseDataFile);

    /*
    *	在应用程序启动后访问统计页面一次
    */
    BOOL VisitStatisticPage();

protected:
    // msgs
    void OnClose();

protected:
    //@{
    /// callback after connected to server and sent HTTP request.
    virtual void OnAfterRequestSend (FCHttpRequest& rTask) ;

    /**
    callback after request finish\n
    after this callback, request will be deleted.
    */
    virtual void OnAfterRequestFinish (FCHttpRequest& rTask) ;

    /**
    callback when request over its lifetime that user set in HTTP_REQUEST_HEADER::m_lifetime_limit\n
    after this callback, request will be deleted.
    */
    virtual void OnOverRequestLifetime (FCHttpRequest& rTask) ;
};


/** 更新应用程序文件
*	return:
*      无返回
*	params:
*		-[in]
*          wParam      1，有更新， 0，当前无更新
*          lParam      当wParam==0，此域也为0 
*                        wParam==1，此域为LPQUI_USERMSGPARAM，
*  -----------------------------------------
*          LPQUI_USERMSGPARAM 域的值为：
*                      wParam  1：更新成功
*                              0：更新失败
*                      lParam  1：baseData 更新
*                              0：应用程序更新
*                      sParam  已下载的更新文件路径
*                      bFreeIt TRUE    接收者应该删除这个数据
*		-[out]
*      无输出
**/
DECLARE_USER_MESSAGE(QSOFT_MSG_UPDATEAPP);

class QAutoUpdater
{
    static VOID CALLBACK CheckUpdateCallback(
        __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);

    SINGLETON_ON_DESTRUCTOR(QAutoUpdater)
    {
        if (NULL != m_hTimer)
        {
            DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
        }
    }

public:
    QAutoUpdater(void);

    /** 启动自动更新程序
    *	return:
    *      TRUE    启动成功
    *	params:
    *		-[in]
    *          dwCheckPeriod   检查更新的频率，单位时间（分）
    *                          最小检查频率为10分钟
    *          sURL            网络地址
    *          szRefer         破解防盗链 如设置：http://www.sina.com，
    *
    **/
    //BOOL Startup(__in _Url& urlUpdation, __in _Url& urlBaseData, __in DWORD dwCheckPeriod = 30);
    BOOL Startup(__in DWORD dwCheckPeriod = 30);

protected:
    /** 异步方式检查程序更新.
    *      如果启动检查成功，检查结束后将会向程序的主窗口发送一个QSOFT_MSG_UPDATEAPP消息
    *	return:
    *      TRUE    启动成功
    **/
    BOOL CheckUpdate( );
    BOOL CheckBaseData( );

    enum ENUM_TASK_TYPE 
    {
        TASK_TYPE_UPDATION = 1,
        TASK_TYPE_BASEDATA = 2,
    };

    // 下载线程回调函数
    struct _Download_Param 
    {
        _Download_Param (ENUM_TASK_TYPE t)
            :pData(nullptr), type_(t)
        {

        }

        QString sURL;	// in
        QString sRefer;	// in
        LPVOID	pData; // in
        QBuffer	bufDown;	// out 下载的数据

        inline BOOL IsUpdationTask()const
        {
            return TASK_TYPE_UPDATION == type_; 
        }

    private:
        ENUM_TASK_TYPE type_;   // 任务类型
    };
    /** 下载数据的线程
    *	params:
    *		-[in]
    *          param            _Download_Param* 
    **/
    static uint __stdcall thread_download(void* param);
private:
    //BOOL				m_bEnd;
    HANDLE              m_hTimer;   // 检查更新的时间定时器
    //     QString             m_sUrl;
    //     QString             m_sRefer;   // 可以破解防盗链
    _Url        url_updation_;
    _Url        url_basedata_;
};

