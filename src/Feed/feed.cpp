#include "feed.h"
#include "../../include/kconfig.h"
#include "../knet/ksyncrequest.h"

#define SVR_CONN "little_qcw"

Feed::Feed(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    KSubscribeHead(, this, SLOT(on_ReciveData(qint64, QByteArray)));
}

Feed::~Feed()
{

}

void Feed::on_btn_first__clicked()
{
    cur_page_ = 0;
    RefreshPage(cur_page_);
}

void Feed::on_btn_prev__clicked()
{
    cur_page_--;

    if (cur_page_ <= 0)
        cur_page_ = 0;

    RefreshPage(cur_page_);
}

void Feed::on_btn_next__clicked()
{
    cur_page_++;

    RefreshPage(cur_page_);
}

void Feed::on_btn_last__clicked()
{
    
}

void Feed::RefreshPage(int ipage)
{
    KConfig cfg;
    cfg.AddMember("page", ipage).AddMember("count", 10);

    auto d = cfg.GetConfigString().toUtf8();
    KAsyncPostRequest2(SVR_CONN, "require_feed_qcw", d);
}

void Feed::on_ReciveData(qint64, QByteArray)
{

}
