/********************************************************************************
** Form generated from reading UI file 'feed.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FEED_H
#define UI_FEED_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_FeedClass
{
public:
    QVBoxLayout *verticalLayout;
    QTableWidget *tbl_;
    QHBoxLayout *horizontalLayout;
    QPushButton *btn_first_;
    QPushButton *btn_prev_;
    QPushButton *btn_next_;
    QPushButton *btn_last_;

    void setupUi(QDialog *FeedClass)
    {
        if (FeedClass->objectName().isEmpty())
            FeedClass->setObjectName(QStringLiteral("FeedClass"));
        FeedClass->resize(694, 467);
        verticalLayout = new QVBoxLayout(FeedClass);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tbl_ = new QTableWidget(FeedClass);
        tbl_->setObjectName(QStringLiteral("tbl_"));

        verticalLayout->addWidget(tbl_);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        btn_first_ = new QPushButton(FeedClass);
        btn_first_->setObjectName(QStringLiteral("btn_first_"));

        horizontalLayout->addWidget(btn_first_);

        btn_prev_ = new QPushButton(FeedClass);
        btn_prev_->setObjectName(QStringLiteral("btn_prev_"));

        horizontalLayout->addWidget(btn_prev_);

        btn_next_ = new QPushButton(FeedClass);
        btn_next_->setObjectName(QStringLiteral("btn_next_"));

        horizontalLayout->addWidget(btn_next_);

        btn_last_ = new QPushButton(FeedClass);
        btn_last_->setObjectName(QStringLiteral("btn_last_"));

        horizontalLayout->addWidget(btn_last_);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(FeedClass);

        QMetaObject::connectSlotsByName(FeedClass);
    } // setupUi

    void retranslateUi(QDialog *FeedClass)
    {
        FeedClass->setWindowTitle(QApplication::translate("FeedClass", "Feed", 0));
        btn_first_->setText(QApplication::translate("FeedClass", "\351\246\226\351\241\265", 0));
        btn_prev_->setText(QApplication::translate("FeedClass", "\344\270\212\344\270\200\351\241\265", 0));
        btn_next_->setText(QApplication::translate("FeedClass", "\344\270\213\344\270\200\351\241\265", 0));
        btn_last_->setText(QApplication::translate("FeedClass", "\346\234\200\345\220\216\351\241\265", 0));
    } // retranslateUi

};

namespace Ui {
    class FeedClass: public Ui_FeedClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FEED_H
