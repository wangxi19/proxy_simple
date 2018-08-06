#include <iostream>
#include "server.h"

int main()
{
    std::cout << "Initialization...\n";
    ProxyServer proxyServer(8080);
    return proxyServer.listening();
}
