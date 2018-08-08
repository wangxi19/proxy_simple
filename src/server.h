#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <sys/types.h>
#include <string>
#include <map>
#include <sstream>
#include <istream>
#include <vector>

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
                split(line, v, ": ");
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

        std::string compose()
        {
            std::string bufferStr = method + " " + uri + " " + version + "\r\n";
            for (std::map<std::string, std::string>::iterator itor = headerMap.begin(); itor != headerMap.end(); ++itor) {
                bufferStr += itor->first + ": " + itor->second + "\r\n";
            }
            bufferStr += "\r\n";
            return bufferStr;
        }
        std::map<std::string, std::string> headerMap;
        std::string method;
        std::string uri;
        std::string version;
    };

static void split(const std::string& s, std::vector<std::string>& v, const std::string& c)
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

public:
    explicit ProxyServer(ushort portNumber);
    ~ProxyServer();

    //http only
    std::string get(const httpHeader& iHttpHeader);
    std::string post(const httpHeader& iHttpHeader);

    std::string GetStdoutFromCommand(std::string cmd);
    void doWork(int fd);
    int extraHeader(char *pBuffer, int size);
    int listening();
private:
    ushort mPortNumber = 0;
    int mServerFd = 0;
};

#endif //PROXYSERVER_H
