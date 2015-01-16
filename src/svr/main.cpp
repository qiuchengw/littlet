#include <iostream>

#include "server.h"

#ifdef _DEBUG
#pragma comment(lib, "quil_mtd.lib")
#else
#pragma comment(lib, "quil_mt.lib")
#endif

int main(int _Argc, char ** _Argv)
{

    return CMsgServer().Startup();
}
