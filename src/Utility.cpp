/* 
 * File:   Utility.cpp
 * Author: daniel
 * 
 * Created on December 5, 2015, 4:14 PM
 */

#include "Utility.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#else
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>
#endif

const unsigned int MAX_NAME_SIZE{ 255 };

std::string Utility::getDomainName()
{
    char hostName[MAX_NAME_SIZE];
    int status = gethostname(hostName, MAX_NAME_SIZE);
    if (status == 0)
    {
        return std::string(hostName);
    }
    return std::string("unknown");
}

std::string Utility::getUserName()
{
#ifdef _WIN32
    char userName[MAX_NAME_SIZE];
    DWORD nameSize = sizeof(userName);
    int status = GetUserName((LPTSTR)userName, &nameSize);
    if (status == 0)
    {
        return std::string(userName);
    }
#else
    uid_t userID = getuid();
    struct passwd *userInfo;
    userInfo = getpwuid(userID);
    if (userInfo != nullptr)
    {
        return std::string(userInfo->pw_name);
    }
#endif
	return std::string("unknown");
}

std::string Utility::getLocalIPAddress(const AddressType addressType)
{
    switch(addressType)
    {
        case AddressType::ADDRESS_LOOPBACK:
            return "127.0.0.1";
        case AddressType::ADDRESS_LOCAL_NETWORK:
            return getExternalLocalIPAddress();
        case AddressType::ADDRESS_INTERNET:
            return getExternalNetworkIPAddress();
    }
    throw std::invalid_argument("No AddressType constant");
}

Utility::AddressType Utility::getNetworkType(const std::string& remoteAddress)
{
    if(remoteAddress.compare("127.0.0.1") == 0 || remoteAddress.compare("::1") == 0 
       || remoteAddress.compare("0:0:0:0:0:0:0:1") == 0 || remoteAddress.compare("localhost") == 0)
    {
        return AddressType::ADDRESS_LOOPBACK;
    }
    //Check for private network address
    if(Utility::isLocalNetwork(remoteAddress))
    {
        return AddressType::ADDRESS_LOCAL_NETWORK;
    }
    //all other addresses are external
    return AddressType::ADDRESS_INTERNET;
}



std::string Utility::trim(const std::string& in)
{
    //https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
    auto wsfront=std::find_if_not(in.begin(),in.end(),[](int c){return std::isspace(c);});
    return std::string(wsfront,std::find_if_not(in.rbegin(),std::string::const_reverse_iterator(wsfront),[](int c){return std::isspace(c);}).base());
}

bool Utility::equalsIgnoreCase(const std::string& s1, const std::string s2)
{
    if(s1.size() != s2.size())
    {
        return false;
    }
#ifdef _WIN32
    return lstrcmpi(s1.data(), s2.data()) == 0;
#else
    return strcasecmp(s1.data(), s2.data()) == 0;
#endif
}

std::string Utility::getExternalLocalIPAddress()
{
    //find external IP of local device
    std::string address("");
#ifdef _WIN32
    //See: http://tangentsoft.net/wskfaq/examples/ipaddr.html
    const hostent* phe = gethostbyname(getDomainName().c_str());
    char buffer[65] = {0};
    for (int i = 0; phe->h_addr_list[i] != nullptr; ++i)
    {
        inet_ntop(phe->h_addrtype, phe->h_addr_list[i], buffer, 64);
        break;
    }
    address = buffer;
#else
    //See: http://linux.die.net/man/3/getifaddrs
    ifaddrs* start = nullptr;
    ifaddrs* current = nullptr;
    
    if(getifaddrs(&start) == -1)
    {
        //error
        return "";
    }
    for(current = start; current != nullptr; current = current->ifa_next)
    {
        //skip loopback
        if((current->ifa_flags & IFF_LOOPBACK) == IFF_LOOPBACK)
        {
            continue;
        }
        //skip disabled
        if((current->ifa_flags & (IFF_UP|IFF_RUNNING)) == 0)
        {
            continue;
        }
        //skip nullptr on socket-address
        if(current->ifa_addr == nullptr)
        {
            continue;
        }
        //skip non IP
        if(current->ifa_addr->sa_family != AF_INET && current->ifa_addr->sa_family != AF_INET6)
        {
            continue;
        }
        //otherwise select first
        break;
    }
    if(current != nullptr)
    {
        char buffer[65] = {0};
        const sockaddr* socketAddress = current->ifa_addr;
        getnameinfo(socketAddress, (socketAddress->sa_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
                    buffer, 64, nullptr, 0, NI_NUMERICHOST );
        address = buffer;
    }
    //free allocated structures
    freeifaddrs(start);
#endif
    return address;
}

std::string Utility::getExternalNetworkIPAddress()
{
    //let some server give us our external IP
    const std::string remoteServer = "http://checkip.dyndns.org/";
    
    //TODO
    //1. open TCP connection
    //2. send something - anything
    //3. read response
    //4. extract IP address
    return "";
}

bool Utility::isLocalNetwork(const std::string& ipAddress)
{
    //See: https://en.wikipedia.org/wiki/Private_network
    if(ipAddress.find('.') != std::string::npos)
    {
        //we have IPv4
        in_addr address;
        inet_pton(AF_INET, ipAddress.c_str(), &address);
        const uint32_t asNumber = ntohl(address.s_addr);
        //first block: 10.0.0.0 - 10.255.255.255 - check values 167772160 to 184549375
        if(asNumber >= 167772160 && asNumber <= 184549375)
        {
            return true;
        }
        //second block: 172.16.0.0 - 172.31.255.255 - check values 2886729728 to 2887778303
        if(asNumber >= 2886729728 && asNumber <= 2887778303)
        {
            return true;
        }
        //third block: 192.168.0.0 - 192.168.255.255 - check values 3232235520 to 3232301055
        if(asNumber >= 3232235520 && asNumber <= 3232301055)
        {
            return true;
        }
        return false;
    }
    else if(ipAddress.find(':') != std::string::npos)
    {
        //we have IPv6
        in6_addr address;
        inet_pton(AF_INET6, ipAddress.c_str(), &address);
        //local block: fd00::/8
        const uint8_t* firstByte = (const uint8_t*) &address.__in6_u;
        return *firstByte == 0xFD;
    }
    throw std::invalid_argument("No IPv4 or IPv6 address supplied");
}
