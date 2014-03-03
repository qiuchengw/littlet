#pragma once

#include "ui/QConfig.h"
#include "file/rjson/inc.h"
#include "ui/QWindow.h"
#include "QUIConnectCenter.h"

// 用户意见反馈系统
//  1> 使用http发送数据到web服务器
//  2> 使用smtp+SSL发送数据到email服务器

class QUserFeedbackWnd : public QFrame
{
    typedef QUIConnectCenter::_QiuInfo _QiuInfo;

    QUI_DECLARE_EVENT_MAP;

    SINGLETON_ON_DESTRUCTOR(QUserFeedbackWnd)
    {

    }

    BEGIN_MSG_MAP_EX(QUserFeedbackWnd)
        CHAIN_MSG_MAP(QFrame)
    END_MSG_MAP()

public:
    QUserFeedbackWnd(void);
    
    /*
     *	sSubject    主题
     *  sSenderName 发送方名字
     *  
     */
    static BOOL Show(const QString& sSubject = L"USER FEEDBACK:",
        const QString& sSenderName = APP_NAME);

protected:
    LRESULT OnMSG_Feedback(UINT uMsg, WPARAM w, LPARAM l);

    LRESULT OnMSG_FeedbackFinished(UINT uMsg, WPARAM w, LPARAM l);

    virtual LRESULT OnDocumentComplete();

    void OnclkSubmit(HELEMENT he);
 
    void OnclkWebsite(HELEMENT he);

    void OnMouseupTxtFeed(MOUSE_PARAMS &);

    // 异步方式发送邮件
    LRESULT AsyncSendMail(LPVOID);
    // 异步方式Post到网络
    LRESULT AsyncWebPost(LPVOID);
    // 假装是在发送中
    LRESULT AsyncFakeSend(LPVOID);

    enum SENDFEEDBACK_RESULT
    {
        SENDFEEDBACK_RESULT_MAXSENDREACHED = -2,    // 达到每天最多能发送的反抗数目
        SENDFEEDBACK_RESULT_FAIL = -1,
        SENDFEEDBACK_RESULT_OK = 0,
        SENDFEEDBACK_RESULT_NOCLIENTMAIL,
        SENDFEEDBACK_RESULT_NOSERVERMAIL,
    };

    QString FormatSendResult(SENDFEEDBACK_RESULT s);

    /**
     *	发送反馈
     *
     *	@param
     *		-[in]
     *          sFeedback       意见
     *          bUseMail        是否使用邮件，否的话就post到web上
    **/
    SENDFEEDBACK_RESULT SendFeedback(__in const QString& sFeedback, __in BOOL bUseMail = TRUE);

    BOOL CheckMaxFeedbackReached();

    // 加一发送记录
    void IncreaseFeedback();

    // 备份未发送的数据
    BOOL BackupFeedback();

    BOOL RestoreFeedback();
   
    QString _FeedbackBackpath()
    {
        return quibase::GetModulePath() + L"feedback.dat";
    }

    EEdit _TxtFeed()
    {
        return GetCtrl("#txt_feed");
    }

    EEdit _TxtContact()
    {
        return GetCtrl("#txt_mail");
    }

    BOOL SaveContactInfo();

    QString MakeContent(const QString& sTxt, const QString& sMail);

private:    
    int         max_feed_;  // 每天最多能发送多少反馈
    QUIConnectCenter::_QiuInfo*   addrs_;    // 地址
    QString     subject_;   // 邮件主题、意见
    QString     feed_content_;
    QString     sender_name_;
};

