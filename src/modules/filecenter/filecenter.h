#ifndef SLOONGNET_MODULE_FILECENTER_H
#define SLOONGNET_MODULE_FILECENTER_H

#include "core.h"
#include "export.h"
#include "filemanager.h"

extern "C"
{
    CResult RequestPackageProcesser(void *, CDataTransPackage *);
    CResult ResponsePackageProcesser(void *, CDataTransPackage *);
    CResult EventPackageProcesser(CDataTransPackage *);
    CResult NewConnectAcceptProcesser(CSockInfo *);
    CResult ModuleInitialization(GLOBAL_CONFIG *);
    CResult ModuleInitialized(SOCKET,IControl *);
    CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{
    class CFileCenter : IObject
    {
    public:

        CResult Initialization(GLOBAL_CONFIG *);
        CResult Initialized(IControl *);

        CResult CreateProcessEnvironmentHandler(void **);

        void EventPackageProcesser(CDataTransPackage *);

        void OnSocketClose(SharedEvent);

    protected:
        list<unique_ptr<FileManager>> m_listManage;
        GLOBAL_CONFIG *m_pConfig;

    public:
        static unique_ptr<CFileCenter> Instance;
    };
} // namespace Sloong

#endif //SLOONGNET_MODULE_FILECENTER_H