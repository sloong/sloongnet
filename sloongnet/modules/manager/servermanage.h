/*
 * @Author: WCB
 * @Date: 2020-04-21 11:17:32
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-12 11:44:45
 * @Description: file content
 */
#ifndef SERVERMANAGE_H
#define SERVERMANAGE_H

#include "main.h"
#include "configuation.h"
#include "DataTransPackage.h"
#include "protocol/manager.pb.h"
using namespace Manager;

#include "IData.h"
namespace Sloong
{

    struct ServerItem
    {
        void Active() { ActiveTime = time(NULL); }
        string Address;
        int Port;
        time_t ActiveTime;
        string UUID;
        string TemplateName;
        int TemplateID;
        SmartConnect Connection;
        Json::Value ToJson(){
            Json::Value item;
            item["Address"] = this->Address;
            item["Port"] = this->Port;
            char buffer [80];
            strftime (buffer,80,"%c", std::localtime(&ActiveTime));
            item["ActiveTime"] = string(buffer);
            item["UUID"] = this->UUID;
            item["TemplateName"] = this->TemplateName;
            item["TemplateID"] = this->TemplateID;
            return item;
        }
    };

    struct TemplateItem
    {
        TemplateItem() {}
        TemplateItem(TemplateInfo info) {
            ID = info.id;
            Name = info.name;
            Note = info.note;
            Configuation = info.configuation;
            Replicas = info.replicas;
        }
        TemplateInfo ToTemplateInfo() {
            TemplateInfo info;
            info.configuation = this->Configuation;
            info.id = this->ID;
            info.name = this->Name;
            info.note = this->Note;
            info.replicas = this->Replicas;
            return info;
        }
        Json::Value ToJson(){
            Json::Value item;
            item["ID"] = this->ID;
            item["Name"] = this->Name;
            item["Replicas"] = this->Replicas;
            item["Created"] = (int)this->Created.size();
            item["Note"] = this->Note;
            item["Configuation"] = this->Configuation;
            return item;
        }
        void ToProtobuf(Manager::TemplateItem* item){
            item->set_id(this->ID);
            item->set_name( this->Name);
            item->set_replicas(this->Replicas);
            item->set_created(this->Created.size());
            item->set_note(this->Note);
            item->set_configuation(this->Configuation);
        }
        int ID;
        string Name;
        string Note;
        int Replicas;
        string Configuation;
        list_ex<int> Reference;
        list_ex<string> Created;
    };

    typedef std::function<CResult(const string&,CDataTransPackage*)> FunctionHandler;
    class CServerManage
    {
    public:
        CResult Initialize( IControl* ic );

        CResult ProcessHandler(CDataTransPackage*);

        /*
        Request:PostLogMessageRequest
        Response:Result
        */
        CResult EventRecorderHandler(const string&,CDataTransPackage*);

        /* When worker start, send this request to registe a worker, and wait manage assigning template.
        Request: Function
        Response: Result    -> Succeed      RegisteWorkerMessageResponse
                            -> Retry        Wait assign, wait some time and retry again.
        */
        CResult RegisteWorkerHandler(const string&,CDataTransPackage*);

        /* When assigning tamplate, and node is ready to work, send this requst.
        Request : RegisteNodeRequest
        Response: Result
        */
        CResult RegisteNodeHandler(const string&,CDataTransPackage*);

        /* 
        Request:AddTemplateRequest
        Response: Result
        */
        CResult AddTemplateHandler(const string&,CDataTransPackage*);
        /* 
        Request:DeleteTemplateRequest
        Response: Result
        */
        CResult DeleteTemplateHandler(const string&,CDataTransPackage*);
        /* 
        Request :SetTemplateRequest
        Response: Result
        */
        CResult SetTemplateHandler(const string&,CDataTransPackage*);
        /* 
        Request:QueryTemplateRequest
        Response:QueryTemplateResponse
        */
        CResult QueryTemplateHandler(const string&,CDataTransPackage*);
        
        /* 
        Request:QueryNodeRequest
        Response:QueryNodeResponse
        */
        CResult QueryNodeHandler(const string&,CDataTransPackage*);

        CResult ResetManagerTemplate(GLOBAL_CONFIG* config);

        void OnSocketClosed(SOCKET);

    private:
        int SearchNeedCreateTemplate();
        void RefreshModuleReference(int id);
        void SendEvent(list<string>, int, ::google::protobuf::Message*);

    protected:
        int m_nSerialNumber = 0;
        map_ex< Manager::Functions , FunctionHandler> m_listFuncHandler;
        map_ex<string, ServerItem>	m_oWorkerList;
        map_ex<int, TemplateItem>	m_oTemplateList;
        map_ex<SOCKET, string>      m_oSocketList;
        
    private:
        CLog* m_pLog = nullptr;
        IControl*   m_pControl = nullptr;
    };
}
#endif
