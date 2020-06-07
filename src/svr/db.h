#pragma once

#include "file/db/QDBMgr.h"


enum EnumUserActivity
{
    USER_ACTIVITY_LOGIN,
    USER_ACTIVITY_LOGOUT,
    USER_ACTIVITY_FEEDBACK,
};

struct UserFeed 
{
    CStdString      mac_;
    CStdString      content_;
    CStdString      contact_;
    QTime           tm_;
};

class db : public DBMan
{
    SINGLETON_ON_DESTRUCTOR(db)
    {

    }

public:
    db(void);

public:
    bool UserLogin(const CStdString& mac);
    bool UserLogout(const CStdString& mac);
    bool UserFeedback(const CStdString& mac, const CStdString& content, const CStdString& contact);

    bool RequireFeedbackMsg(int i_page, int l_count, 
        __out int &total, __out std::vector<UserFeed>& feeds);

protected:
    bool _UserActivity(const CStdString& mac, EnumUserActivity active);
};

