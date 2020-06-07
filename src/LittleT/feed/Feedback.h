#pragma once

#include "ui/QFrame.h"

#pragma comment(lib, "libzmq.lib")

// 用户意见反馈系统
//  1> 使用http发送数据到web服务器
//  /// 2> 使用smtp+SSL发送数据到email服务器
class QUserFeedbackWnd : public QFrame
{
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
    static BOOL Show(const CStdString& sSubject = L"USER FEEDBACK:",
        const CStdString& sSenderName = APP_NAME);

protected:
    LRESULT OnMSG_Feedback(UINT uMsg, WPARAM w, LPARAM l);

    LRESULT OnMSG_FeedbackFinished(UINT uMsg, WPARAM w, LPARAM l);

    virtual LRESULT OnDocumentComplete();

    void OnclkSubmit(HELEMENT he);
 
    void OnclkWebsite(HELEMENT he);

    void OnMouseupTxtFeed(MOUSE_PARAMS &);

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

    CStdString FormatSendResult(SENDFEEDBACK_RESULT s);

    /**
     *	发送反馈
     *
     *	@param
     *		-[in]
     *          sFeedback       意见
    **/
    SENDFEEDBACK_RESULT SendFeedback( __in const CStdString& sFeedback);
    
    BOOL CheckMaxFeedbackReached();

    // 加一发送记录
    void IncreaseFeedback();

    // 备份未发送的数据
    BOOL BackupFeedback();

    BOOL RestoreFeedback();
   
    CStdString _FeedbackBackpath();

    EEdit _TxtFeed()
    {
        return GetCtrl("#txt_feed");
    }

    EEdit _TxtContact()
    {
        return GetCtrl("#txt_mail");
    }

    BOOL SaveContactInfo();

    CStdString MakeContent(const CStdString& sTxt, const CStdString& sMail);

private:    
    int         max_feed_;  // 每天最多能发送多少反馈
    CStdString     feed_content_;
    CStdString     sender_name_;
};

