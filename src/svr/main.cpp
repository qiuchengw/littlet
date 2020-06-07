#include <iostream>

#include "server.h"

#pragma comment(lib, "qui.lib")

int main(int _Argc, char ** _Argv)
{

    return CMsgServer().Startup();
}
