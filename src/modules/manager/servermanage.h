/*
 * @Author: WCB
 * @Date: 2020-04-21 11:17:32
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 10:27:12
 * @Description: file content
 */
#ifndef SLOONGNET_MANAGER_SERVERMANAGE_h
#define SLOONGNET_MANAGER_SERVERMANAGE_h

#include "configuation.h"
#include "DataTransPackage.h"
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
        SOCKET ConnectionID;
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
            //Configuation = CBase64::Decode(this->Configuation);// string(info.configuation.begin(), info.configuation.end());
            Configuation = string(info.configuation.begin(), info.configuation.end());
            Replicas = info.replicas;
            BuildCache();
        }
        TemplateInfo ToTemplateInfo()
        {
            TemplateInfo info;
            //info.configuation = CBase64::Encode(this->Configuation);//   vector<char>(this->Configuation.begin(), this->Configuation.end());
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
        CResult Initialize(IControl *ic, const string&);
        
        void OnSocketClosed(SOCKET);

        CResult ProcessHandler(CDataTransPackage *);

        CResult EventRecorderHandler(const string &, CDataTransPackage *);
        CResult RegisteWorkerHandler(const string &, CDataTransPackage *);
        CResult RegisteNodeHandler(const string &, CDataTransPackage *);
        CResult AddTemplateHandler(const string &, CDataTransPackage *);
        CResult DeleteTemplateHandler(const string &, CDataTransPackage *);
        CResult SetTemplateHandler(const string &, CDataTransPackage *);
        CResult QueryTemplateHandler(const string &, CDataTransPackage *);
        CResult QueryNodeHandler(const string &, CDataTransPackage *);
        CResult StopNodeHandler(const string &, CDataTransPackage *);
        CResult RestartNodeHandler(const string &, CDataTransPackage *);
        CResult QueryReferenceInfoHandler(const string &, CDataTransPackage *);
        CResult ReportLoadStatusHandler(const string &, CDataTransPackage *);

    public:
        static CResult LoadManagerConfig(const string&);
        static CResult ResetManagerTemplate(GLOBAL_CONFIG *config);


    private:
        int SearchNeedCreateTemplate();
        void RefreshModuleReference(int id);
        void SendEvent(const list<uint64_t> &, int, ::google::protobuf::Message *);

    protected:
        map_ex<Manager::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<int64_t, NodeItem> m_mapUUIDToNodeItem;
        map_ex<int, TemplateItem> m_mapIDToTemplateItem;
        map_ex<SOCKET, int64_t> m_mapSocketToUUID;
    };
} // namespace Sloong
#endif
