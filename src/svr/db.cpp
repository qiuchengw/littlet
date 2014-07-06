#include "db.h"

db::db(void)
{
}

bool db::UserLogin( const CStdString& mac )
{
    return _UserActivity(mac, USER_ACTIVITY_LOGIN);
}

bool db::UserLogout( const CStdString& mac )
{
    return _UserActivity(mac, USER_ACTIVITY_LOGOUT);
}

bool db::UserFeedback( const CStdString& mac, const CStdString& content, const CStdString& contact )
{
    // ¼ÇÂ¼
    CStdString sQ;
    sQ.Format(L"INSERT INTO tbl_feed(mac, content, contact, crtime) "
        L" VALUES('%s', '%s', '%s', %lf)",
        SFHSQ(mac), SFHSQ(content), SFHSQ(contact), QTime::SQLDateTimeCurrent());

    _ExecSQL_RetBOOL(sQ);

    return _UserActivity(mac, USER_ACTIVITY_FEEDBACK);
}

bool db::_UserActivity( const CStdString& mac, EnumUserActivity active )
{
    CStdString sQ;
    sQ.Format(L"INSERT INTO tbl_activity(mac, activity, crtime) "
        L" VALUES('%s', %d, %lf)",
        SFHSQ(mac), active, QTime::SQLDateTimeCurrent());

    return _ExecSQL_RetBOOL(sQ);
}

