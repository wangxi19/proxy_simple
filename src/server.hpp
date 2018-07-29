#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <sys/types.h>
#include <string>
#include <map>

class ProxyServer
{
    struct header
    {
        explicit header(const std::string &headerRawStr)
        {
        }
        ~header()
        {
        }

        void clear()
        {
            headerMap.clear();
        }
        std::map<std::string, std::string> headerMap;
        std::string method;
        std::string uri;
        std::string version;
    };

public:
    explicit ProxyServer(ushort portNumber);
    ~ProxyServer();

    int listening();
private:
    ushort mPortNumber = 0;
};

#endif //PROXYSERVER_H
