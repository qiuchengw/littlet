#ifndef FEED_H
#define FEED_H

#include <QtWidgets/QDialog>
#include "ui_feed.h"

class Feed : public QDialog
{
    Q_OBJECT

public:
    Feed(QWidget *parent = 0);
    ~Feed();

protected:
    void RefreshPage(int ipage);

private slots:
    void on_btn_first__clicked();

    void on_btn_prev__clicked();

    void on_btn_next__clicked();

    void on_btn_last__clicked();

    void on_ReciveData(qint64, QByteArray);

private:
    Ui::FeedClass ui;

    int cur_page_ = 0;
};

#endif // FEED_H
