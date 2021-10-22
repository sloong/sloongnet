/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2019-01-15 15:57:36
 * @LastEditTime: 2021-09-23 17:05:09
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/gateway.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description:
 */
/***
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#pragma once

#include "EasyConnect.h"
#include "IObject.h"
#include "core.h"
#include "export.h"
#include "transpond.h"

#include "protocol/manager.pb.h"
using namespace Manager;

extern "C"
{
    PackageResult RequestPackageProcesser(void *, Package *);
    PackageResult ResponsePackageProcesser(void *, Package *);
    CResult EventPackageProcesser(Package *);
    CResult NewConnectAcceptProcesser(SOCKET);
    CResult ModuleInitialization(IControl *);
    CResult ModuleInitialized();
    CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{
typedef struct _FORWARD_RANE_INFO
{
    bool InRange(int f)
    {
        return f >= begin && f <= end;
    }
    int begin, end;
    int forward_to;
    std::function<int(int)> converter;
} _FORWARD_RANE_INFO;

typedef struct _FORWARD_MAP_INFO
{
    int from, to;
    int forward_to;
} _FORWARD_MAP_INFO;

typedef struct _FORWARD_INFO
{
    SessionID connection_id;
    int function_id;
} _FORWARD_INFO;

using FWResult = TResult<_FORWARD_INFO>;

class SloongNetGateway : public IObject
{
  public:
    CResult Initialization(IControl *);
    CResult Initialized();

    PackageResult ResponsePackageProcesser(Package *);

    void QueryReferenceInfo();
    void QueryReferenceInfoResponseHandler(IEvent *, Package *);

    inline CResult CreateProcessEnvironmentHandler(void **);
    void EventPackageProcesser(Package *);

    // Event handler
    void OnStart(SharedEvent);

    void OnReferenceModuleOnlineEvent(const string &, Package *);
    void OnReferenceModuleOfflineEvent(const string &, Package *);

  private:
    inline int ParseFunctionValue(const string &);
    list<int> ProcessProviedFunction(const string &);

    void AddConnection(uint64_t, const string &, int);

  public:
    FWResult GetForwardInfo(int function);
    list_ex<_FORWARD_RANE_INFO> m_mapforwardRangeInfo;
    map_ex<int, _FORWARD_MAP_INFO> m_mapForwardMapInfo;
    map_ex<int, list_ex<uint64_t>> m_mapTempteIDToUUIDs;
    map_ex<uint64_t, NodeItem> m_mapUUIDToNode;
    map_ex<uint64_t, SessionID> m_mapUUIDToConnectionID;
    map_ex<uint64_t, UniquePackage> m_mapSerialToRequest;

  protected:
    list<unique_ptr<GatewayTranspond>> m_listTranspond;

    GLOBAL_CONFIG *m_pConfig;
    Json::Value *m_pModuleConfig;

    int _DefaultTemplateId;

  public:
    static unique_ptr<SloongNetGateway> Instance;
};

} // namespace Sloong
