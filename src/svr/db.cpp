#include "db.h"

db::db(void)
{
}

bool db::UserLogin( const CStdString& mac )
{
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(id) FROM tbl_user WHERE (mac='%s')", SFHSQ(mac));
    if (0 >= _ExecSQL_RetInt(sQ))
    {
        // 插入一个mac
        sQ.Format(L"INSERT INTO tbl_user(mac, crtime) VALUES('%s', %lf);",
            SFHSQ(mac), QTime::SQLDateTimeCurrent());
        _ExecSQL_RetBOOL(sQ);
    }

    return _UserActivity(mac, USER_ACTIVITY_LOGIN);
}

bool db::UserLogout( const CStdString& mac )
{
    return _UserActivity(mac, USER_ACTIVITY_LOGOUT);
}

bool db::UserFeedback( const CStdString& mac, const CStdString& content, const CStdString& contact )
{
    // 记录
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

bool db::RequireFeedbackMsg(int i_page, int l_count, 
    __out int &total, __out std::vector<UserFeed>& feeds)
{
    total = _ExecSQL_RetInt(L"SELECT COUNT(id) FROM tbl_feed");

    CStdString sQ;
    sQ.Format(L"SELECT * FROM tbl_feed LIMIT %2 OFFSET %3",
        l_count, l_count * i_page);
    SqlQuery q = ExecQuery(sQ);
    while (!q.Eof())
    {
        UserFeed uf;
        uf.mac_ = q.StrValue(L"mac");
        uf.content_ = q.StrValue(L"content");
        uf.contact_ = q.StrValue(L"contact");
        uf.tm_ = q.DateTimeValue(L"crtime");
        feeds.push_back(uf);

        q.nextRow();
    }
    return true;
}

