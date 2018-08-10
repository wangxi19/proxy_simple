#ifndef PROXYSERVER_H
#define PROXYSERVER_H
#define UNUSED(arg) (void)arg;

#include <sys/types.h>
#include <string>
#include <map>
#include <sstream>
#include <istream>
#include <vector>
#include <regex>

class ProxyServer
{
    struct httpHeader
    {
        explicit httpHeader(const std::string &headerRawStr)
        {
            std::istringstream ss(headerRawStr);
            std::string line;
            std::getline(ss, line);
            std::istringstream ss2(line);
            ss2 >> method;
            ss2 >> uri;

            //parse url and uri, parameters will be merged to uri
            std::regex rgx("((\\w+\\://)?[^/]+)(/.*)");
            std::smatch match;
            //            if (std::regex_search(s.begin(), s.end(), match, rgx))
            if (std::regex_search(uri, match, rgx)) {
                url = match[1];
                std::vector<std::string> v;
                ProxyServer::split(url, v, ":");
                if (v.size() > 1 && v[v.size() - 1].find("//") == std::string::npos){
                    port = std::stoi(v[v.size() -1]);
                }
                else {
                    port = std::stoi("80");
                }
                uri = match[3];
            }

            ss2 >> version;

            while (std::getline(ss, line)) {
                if (line.length() == 0) {
                    //TODO error occured
                    continue;
                }
                if ('\r' != *(--line.end())) {
                    //TODO error occured
                    continue;
                }
                //trim the '\r'
                line = line.substr(0, line.length() - 1);
                std::vector<std::string> v;
                ProxyServer::split(line, v, ": ");
                if (v.size() != 2) {
                    //TODO error
                    continue;
                }
                headerMap.insert(headerMap.end(), std::pair<std::string, std::string>(v[0], v[1]));
            }
        }
        ~httpHeader()
        {
        }

        void clear()
        {
            headerMap.clear();
        }

        std::string compose() const
        {
            std::string bufferStr = method + " " + url + uri + " " + version + "\r\n";
            for (auto const &itor: headerMap) {
                bufferStr += itor.first + ": " + itor.second + "\r\n";
            }
            bufferStr += "\r\n";
            return bufferStr;
        }

        std::string getHeader(const std::string &iHeaderNm)
        const
        {
            for (auto const &itor: headerMap) {
                if (itor.first == iHeaderNm) {
                    return itor.second;
                }
            }
            return std::string();
        }
        std::map<std::string, std::string> headerMap;
        std::string method;
        std::string uri;
        std::string url;
        unsigned int port;
        std::string version;
    };


public:
    explicit ProxyServer(ushort portNumber);
    ~ProxyServer();

    //http only
    std::string get(const httpHeader& iHttpHeader);
    std::string post(const httpHeader& iHttpHeader, const char* const pData, int size, int __sidx = 0);

    std::string GetStdoutFromCommand(std::string cmd);
    void doWork(int fd);
    int extraHeader(char *pBuffer, int size);
    int listening();
    static void split(const std::string &s, std::vector<std::string> &v, const std::string &c);
private:
    inline int dns(struct addrinfo **result, const char* domain);

    ushort mPortNumber = 0;
    int mServerFd = 0;
};

#endif //PROXYSERVER_H
