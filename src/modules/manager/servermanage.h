/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-21 11:17:32
 * @LastEditTime: 2021-01-05 16:29:35
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/manager/servermanage.h
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

#include "configuation.h"

#include "protocol/manager.pb.h"
using namespace Manager;

#include "IData.h"
#include "IObject.h"
namespace Sloong
{
    struct NodeItem
    {
        void Active() { ActiveTime = time(NULL); }
        string Address;
        int Port;
        time_t ActiveTime;
        uint64_t UUID;
        string TemplateName;
        int TemplateID;
        uint64_t ConnectionHashCode;
        Json::Value ToJson()
        {
            Json::Value item;
            item["Address"] = this->Address;
            item["Port"] = this->Port;
            char buffer[80];
            strftime(buffer, 80, "%c", std::localtime(&ActiveTime));
            item["ActiveTime"] = string(buffer);
            item["UUID"] = (Json::UInt64)this->UUID;
            item["TemplateName"] = this->TemplateName;
            item["TemplateID"] = this->TemplateID;
            return item;
        }
        void ToProtobuf(Manager::NodeItem *item)
        {
            item->set_address(this->Address);
            item->set_port(this->Port);
            char buffer[80];
            strftime(buffer, 80, "%c", std::localtime(&ActiveTime));
            item->set_activetime(string(buffer));
            item->set_uuid(this->UUID);
            item->set_templatename(this->TemplateName);
            item->set_templateid(this->TemplateID);
        }
    };

    struct TemplateItem
    {
        TemplateItem() {}
        TemplateItem(TemplateInfo info)
        {
            ID = info.id;
            Name = info.name;
            Note = info.note;
            Configuation = string(info.configuation.begin(), info.configuation.end());
            Replicas = info.replicas;
            BuildCache();
        }
        TemplateInfo ToTemplateInfo()
        {
            TemplateInfo info;
            info.configuation = vector<char>(this->Configuation.begin(), this->Configuation.end());
            info.id = this->ID;
            info.name = this->Name;
            info.note = this->Note;
            info.replicas = this->Replicas;
            return info;
        }
        Json::Value ToJson()
        {
            Json::Value item;
            item["ID"] = this->ID;
            item["Name"] = this->Name;
            item["Replicas"] = this->Replicas;
            item["Created"] = (int)this->Created.size();
            item["Note"] = this->Note;
            item["Configuation"] = this->Configuation;
            return item;
        }
        void ToProtobuf(Manager::TemplateItem *item)
        {
            item->set_id(this->ID);
            item->set_name(this->Name);
            item->set_replicas(this->Replicas);
            item->set_created(this->Created.size());
            item->set_note(this->Note);
            item->set_configuation(this->Configuation);
        }
        bool IsValid()
        {
            if (ID < 0 || Name.length() == 0 || Replicas < 0 || Configuation.length() == 0)
                return false;
            return true;
        }
        void BuildCache()
        {
            ConfiguationObj = ConvertStrToObj<GLOBAL_CONFIG>(this->Configuation);
        }
        int ID;
        string Name;
        string Note;
        int Replicas;
        string Configuation;
        shared_ptr<GLOBAL_CONFIG> ConfiguationObj;
        list_ex<int> Reference;
        list_ex<uint64_t> Created;
    };

    class CServerManage : public IObject
    {
    public:
        CResult Initialize(IControl *ic, const string &);

        void OnSocketClosed(uint64_t);

        PackageResult ProcessHandler(Package *);

        CResult EventRecorderHandler(const string &, Package *);
        CResult RegisteWorkerHandler(const string &, Package *);
        CResult RegisteNodeHandler(const string &, Package *);
        CResult AddTemplateHandler(const string &, Package *);
        CResult DeleteTemplateHandler(const string &, Package *);
        CResult SetTemplateHandler(const string &, Package *);
        CResult QueryTemplateHandler(const string &, Package *);
        CResult QueryNodeHandler(const string &, Package *);
        CResult StopNodeHandler(const string &, Package *);
        CResult RestartNodeHandler(const string &, Package *);
        CResult QueryReferenceInfoHandler(const string &, Package *);
        CResult ReportLoadStatusHandler(const string &, Package *);

    public:
        static CResult LoadManagerConfig(const string &);
        static CResult ResetManagerTemplate(GLOBAL_CONFIG *config);

    private:
        int SearchNeedCreateTemplate(  );
        int SearchNeedCreateWithIDs( const vector<int>& );
        int SearchNeedCreateWithType( bool, const vector<int>&  );
        void RefreshModuleReference(int id);
        void SendEvent(const list<uint64_t> &, int, ::google::protobuf::Message *);

    protected:
        map_ex<Manager::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<uint64_t, NodeItem> m_mapUUIDToNodeItem;
        strict_map<int, TemplateItem> m_mapIDToTemplateItem;
        map_ex<uint64_t, uint64_t> m_mapConnectionToUUID;
    };
} // namespace Sloong
