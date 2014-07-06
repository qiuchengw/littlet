#pragma once

#include "file/db/QDBMgr.h"


enum EnumUserActivity
{
    USER_ACTIVITY_LOGIN,
    USER_ACTIVITY_LOGOUT,
    USER_ACTIVITY_FEEDBACK,
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

protected:
    bool _UserActivity(const CStdString& mac, EnumUserActivity active);
};

