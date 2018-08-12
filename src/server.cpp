// Server side C/C++ program to demonstrate Socket programming
#include "server.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ProxyServer::ProxyServer(ushort portNumber)
    :mPortNumber(portNumber)
{

}

ProxyServer::~ProxyServer()
{

}

std::string ProxyServer::get(const httpHeader &iHttpHeader)
{
    struct addrinfo *result, *rp;
    int s = dns(&result, iHttpHeader.getHeader("Host").c_str());
    if (s != 0) {
        //TODO [error]
        std::cout << "[dnsError]: " << gai_strerror(s) << std::endl;
        freeaddrinfo(result);
        return std::string("");
    }

    int sfd = 0;
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        //set the port number
        uint16_t portNb = htons(iHttpHeader.port);
        memcpy(rp->ai_addr->sa_data, &portNb, 2);
        if (connect(sfd, rp->ai_addr, sizeof(struct sockaddr)) != -1)
            break;

        sfd = -1;
    }
    freeaddrinfo(result);

    if (-1 == sfd) {
        //TODO [error]
        std::cout << "[dnsError]: " << "connot connect to server" << std::endl;
        return std::string();
    }

    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        //TODO
        perror("setsockopt");
        return std::string();
    }

    std::string bufStr = iHttpHeader.compose();
    write(sfd, bufStr.c_str(), bufStr.length());
    bufStr.clear();
    char *pBuffer = (char*)calloc(10240, 1);
    int rvSize = read(sfd, pBuffer, 10240);
    if (rvSize == 0) {
        //TODO [error]
    }
    if (rvSize < 0) {
        //TODO [error]
    }

    bufStr = pBuffer;
    free(pBuffer);
    close(sfd);
    return bufStr;
}

std::string ProxyServer::post(const httpHeader &iHttpHeader, const char * const pData, int size, int __sidx)
{
    UNUSED(iHttpHeader);
    UNUSED(pData);

    struct addrinfo *result, *rp;
    int s = dns(&result, iHttpHeader.getHeader("Host").c_str());
    if (s != 0) {
        //TODO [error]
        std::cout << "[dnsError]: " << gai_strerror(s) << std::endl;
        return std::string("");
    }

    int sfd = 0;
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        //set the port number
        uint16_t portNb = htons(iHttpHeader.port);
        memcpy(rp->ai_addr->sa_data, &portNb, 2);
        if (connect(sfd, rp->ai_addr, sizeof(struct sockaddr)) != -1)
            break;

        sfd = -1;
    }
    freeaddrinfo(result);

    if (-1 == sfd) {
        //TODO [error]
        std::cout << "[dnsError]: " << "connot connect to server" << std::endl;
        return std::string();
    }

    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        //TODO
        perror("setsockopt");
        return std::string();
    }

    std::string bufStr = iHttpHeader.compose();
    write(sfd, bufStr.c_str(), bufStr.length());
    bufStr.clear();
    if (__sidx < 0) __sidx = 0;
    if (size > 0)
        write(sfd, pData + __sidx, size);

    char *pBuffer = (char*)calloc(10240, 1);
    int rvSize = read(sfd, pBuffer, 10240);
    if (rvSize == 0) {
        //TODO [error]
    }
    if (rvSize < 0) {
        //TODO [error]
    }

    bufStr = pBuffer;
    free(pBuffer);
    close(sfd);
    return bufStr;

    return std::string();
}

std::string ProxyServer::GetStdoutFromCommand(std::string cmd)
{
    std::string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
    return data;
}

void ProxyServer::doWork(int fd)
{
    char* buffer = (char *)calloc(10240, 1);
    int datLength = read(fd, buffer, 10240);
    if (datLength < 0) {
        //TODO peer close the connection
    } else if (datLength == 0) {
        //TODO no data has been sended
    }
    //check whether http entire header has been readed
    int headerLength = extraHeader(buffer, datLength);
    httpHeader header(std::string(buffer, headerLength));
    //just version1, so don't thinking a huge data condition
    std::string bodyLenStr;
    for (const auto &itor: header.headerMap) {
        if (itor.first == std::string("Content-Length")) {
            bodyLenStr = itor.second;
            break;
        }
    }

    //TODO [bug] will be crashed when bodyLenStr is not a valid numeric
    int bodyLen = bodyLenStr.length() == 0 ? 0 : std::stoi(bodyLenStr);
    if (headerLength + bodyLen > 10240) {
        //TODO [work] will be handle later
    }

    if (headerLength + bodyLen != datLength) {
        //TODO [error] http request error
    }

    //TODO will to get ip addr by domain name
    std::string bufStr;
    if (header.method == "POST") {
        bufStr = post(header, buffer, bodyLen, headerLength);
    } else if (header.method == "GET") {
        bufStr = get(header);
    }
    write(fd, bufStr.c_str(), bufStr.length());

    free(buffer);
}

int ProxyServer::extraHeader(char *pBuffer, int size)
{
    int headerLength = 0;
    for (int i = 0; i < size; ++i) {
        if ('\r' == pBuffer[i] && i < size - 3 && '\n' == pBuffer[i+1]
                && '\r' == pBuffer[i+2]
                && '\n' == pBuffer[i+3])
        {
            break;
        }
        ++headerLength;
    }

    return headerLength + 4;
}



int ProxyServer::listening()
{
    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((mServerFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(mServerFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( mPortNumber );

    // Forcefully attaching socket to the port 8080
    if (bind(mServerFd, (struct sockaddr *)&address,
             sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(mServerFd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

Listening:
    std::cout << "*********************Listening on " << mPortNumber << "*********************\n";
    if ((new_socket = accept(mServerFd, (struct sockaddr *)&address,
                             (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    doWork(new_socket);
    close(new_socket);
    goto Listening;
    return 0;
}

void ProxyServer::split(const std::string &s, std::vector<std::string> &v, const std::string &c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

inline int ProxyServer::dns(addrinfo **result, const std::string &domain)
{
    std::regex rgx("^\\s*(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})(\\:\\d+)?\\s*$");
    std::smatch match;
    if (std::regex_search(domain, match, rgx)) {
        if (match[1].length() > 0) {
            memcpy((*result)->ai_addr->sa_data + 2, inet_addr(match[1].c_str()), 4);
        }

        if (match[2].length() > 0) {
            //cp port number
        }

        return 1;
    }
//    (*result)->ai_addr->sa_data
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = /*AF_UNSPEC*/AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = /*SOCK_DGRAM*/SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = /*AI_PASSIVE*/AI_CANONNAME;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    return getaddrinfo(domain.c_str(), NULL, &hints, result);
}
