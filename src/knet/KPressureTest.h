#pragma once

/*
 *	 用于压力测试的代码
 */

#include <QThread>

class KPressureTest : public QThread
{
public:
    KPressureTest();
    ~KPressureTest();

protected:
    virtual void run();

};

