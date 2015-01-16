#include "feed.h"
#include <QtWidgets/QApplication>

#pragma comment(lib, "knet.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Feed w;
    w.show();
    return a.exec();
}
