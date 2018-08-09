#include <iostream>
#include "server.h"

#define PORT 8080

int main()
{
    std::cout << "Initialization...\n";
    ProxyServer proxyServer(PORT);
    return proxyServer.listening();
}
