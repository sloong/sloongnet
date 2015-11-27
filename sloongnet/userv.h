#ifndef SLOONGWALLUS_H
#define SLOONGWALLUS_H

class CEpollEx;
class CMsgProc;
class SloongWallUS
{
public:
    SloongWallUS();
    ~SloongWallUS();

    void Initialize(int nPort);
    void Run();

protected:
    int m_sockServ;
    int* m_sockClient;
    CEpollEx* m_pEpoll;
    CMsgProc* m_pMsgProc;
};



#endif //SLOONGWALLUS_H
