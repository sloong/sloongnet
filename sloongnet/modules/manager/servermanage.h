/*
 * @Author: WCB
 * @Date: 2020-04-21 11:17:32
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-21 16:16:08
 * @Description: file content
 */
#ifndef SERVERMANAGE_H
#define SERVERMANAGE_H

#include "main.h"
#include "configuation.h"
#include "DataTransPackage.h"
#include "IData.h"
namespace Sloong
{

    struct ServerItem
    {
        void Active() { ActiveTime = time(NULL); }
        string Address;
        int Port;
        int ActiveTime;
        string UUID;
        string TemplateName;
        int TemplateID;
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
        int ID;
        string Name;
        string Note;
        int Replicas;
        string Configuation;
        list_ex<string> Created;
    };

    typedef std::function<bool(const Json::Value&,CDataTransPackage*)> FunctionHandler;
    class CServerManage
    {
    public:
        CResult Initialize( int template_id );

        bool ProcessHandler(Functions func, string sender, CDataTransPackage* pack);

        bool RegisterServerHandler(Functions func,string sender, CDataTransPackage* pack);

        /* 
        Request(JSON):
                    {
                        "Function":"AddTemplate",
                        "Template":{
                            "Name":"",
                            "Note":"",
                            "Replicas":"",
                            "Configuation":""//base64
                        }
                    }
        Response: Content -> ID
        */
        bool AddTemplateHandler(const Json::Value&,CDataTransPackage*);
        /* 
        Request(JSON):
                    {
                        "Function":"DeleteTemplate",
                        "TemplateID":""
                    }
        Response: Result
        */
        bool DeleteTemplateHandler(const Json::Value&,CDataTransPackage*);
        /* 
        Request(JSON):
                    {
                        "Function":"SetTemplate",
                        "Template":{
                            "ID":"",
                            "Name":"",
                            "Note":"",
                            "Replicas":"",
                            "Configuation":"" //base64
                        }
                    }
        Response: Result
        */
        bool SetTemplateHandler(const Json::Value&,CDataTransPackage*);
        /* 
        Request(JSON):
                    {
                        "Function":"QueryTemplate",
                        "TemplateID":"" // Empty for query all.
                    }
        Response(JSON):
                    {
                        "TemplateList": [
                        {
                          "ID": "",
                          "Name":"",
                          "Note":"",
                          "Replicas":"",
                          "Configuation":""
                        }
                      ]
                    }
        */
        bool QueryTemplateHandler(const Json::Value&,CDataTransPackage*);
        
        /* Flow:  ControlUI -> Control
       Response: Content(JSON) - Config template  list.
                    Format: 
                    {
                      "ServerList": [
                        {
                          "UUID": "",
                          "Type":"",
                          "TemplateID": ""
                        }
                      ]
                    }
    */
        bool GetServerListHandler(const Json::Value&,CDataTransPackage*);

        /// 由于初始化太早，无法在initialize时获取。只能由control_server手动设置
        void SetLog(CLog* log) { m_pLog = log; }

        void ResetManagerTemplate(const TemplateInfo&);

    private:
        int SearchNeedCreateTemplate();

    private:
        unique_ptr<CConfiguation>	m_pAllConfig = make_unique<CConfiguation>();
        map_ex<string, FunctionHandler> m_listFuncHandler;
        CLog* m_pLog = nullptr;
        map_ex<string, ServerItem>	m_oServerList;
        map_ex<int, TemplateItem>	m_oTemplateList;
    };
}
#endif
