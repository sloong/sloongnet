#ifndef UTILITY_H
#define UTILITY_H

class CUtility
{
public:
    CUtility();

    static int GetCpuUsed(int nWaitTime = 10);
    static int GetMemory(int& total, int& free);
};

#endif // UTILITY_H
