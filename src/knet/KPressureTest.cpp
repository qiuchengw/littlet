#include "KPressureTest.h"
#include "deps\nzmqt\nzmqt.hpp"


KPressureTest::KPressureTest()
{
}


KPressureTest::~KPressureTest()
{
}

void KPressureTest::run()
{
    nzmqt::SocketNotifierZMQContext ctx;
    auto *sock = ctx.createSocket(nzmqt::ZMQSocket::TYP_DEALER, this);

    for (int i = 0; i < 100000; ++i)
    {
        msleep(1);

        // ·¢Êý¾Ý
        
    }
}
