/*
 * @Author: WCB
 * @Date: 2020-04-21 11:17:32
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-28 12:09:02
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
        time_t ActiveTime;
        string UUID;
        string TemplateName;
        int TemplateID;
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
        int ID;
        string Name;
        string Note;
        int Replicas;
        string Configuation;
        list_ex<string> Created;
    };

    typedef std::function<CResult(const Json::Value&,CDataTransPackage*)> FunctionHandler;
    class CServerManage
    {
    public:
        CResult Initialize();

        CResult ProcessHandler(CDataTransPackage*);

        /* When worker start, send this request to registe a worker, and wait manage assigning template.
        Request(JSON):
                    {
                        "Function":"RegisteWorker",
                    }
        Response: Result    -> Succeed      Content(JSON)
                                            {
                                                "TemplateID":"",
                                                "Configuation":""//base64
                                            }
                            -> Retry        Wait assign, wait some time and retry again.
        */
        CResult RegisteWorkerHandler(const Json::Value&,CDataTransPackage*);

        /* When assigning tamplate, and node is ready to work, send this requst.
        Request(JSON):
                    {
                        "Function":"RegisteNode",
                        "TemplateID":""
                    }
        Response: Result
        */
        CResult RegisteNodeHandler(const Json::Value&,CDataTransPackage*);

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
        Response: Result
        */
        CResult AddTemplateHandler(const Json::Value&,CDataTransPackage*);
        /* 
        Request(JSON):
                    {
                        "Function":"DeleteTemplate",
                        "TemplateID":""
                    }
        Response: Result
        */
        CResult DeleteTemplateHandler(const Json::Value&,CDataTransPackage*);
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
        CResult SetTemplateHandler(const Json::Value&,CDataTransPackage*);
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
        CResult QueryTemplateHandler(const Json::Value&,CDataTransPackage*);
        
        /* 
        Request(JSON):
                    {
                        "Function":"QueryNode",
                    }
        Response(JSON):
                    {
                      "NodeList": [
                        {
                          "UUID": "",
                          "TemplateName":"",
                          "TemplateID": "",
                          "Address":"",
                          "Port":"",
                          "ActiveTime":"",
                        }
                      ]
                    }
    */
        CResult QueryNodeHandler(const Json::Value&,CDataTransPackage*);

        CResult Regise

        /// 由于初始化太早，无法在initialize时获取。只能由control_server手动设置
        void SetLog(CLog* log) { m_pLog = log; }

        CResult ResetManagerTemplate(GLOBAL_CONFIG* config);

    private:
        int SearchNeedCreateTemplate();

    private:
        map_ex<string, FunctionHandler> m_listFuncHandler;
        CLog* m_pLog = nullptr;
        map_ex<string, ServerItem>	m_oWorkerList;
        map_ex<int, TemplateItem>	m_oTemplateList;
    };
}
#endif
