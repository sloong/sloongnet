#ifndef SOCKINFO_H
#define SOCKINFO_H

#include<queue>
using namespace std;
#include <string>
using std::string;
class CSockInfo
{
public:
    CSockInfo();

    queue<string> m_ReadList;
    queue<string> m_WriteList;
    string m_Address;
    int m_nPort;
    time_t m_ConnectTime;
    int m_sock;
};

#endif // SOCKINFO_H
