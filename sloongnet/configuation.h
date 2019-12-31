/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:12:59
 * @Description: file content
 */

#ifndef SLOONGNET_CONFIGUATION_H
#define SLOONGNET_CONFIGUATION_H

#include "main.h"
#include "SQLiteEx.h"

namespace Sloong
{
    class CConfiguation
    {
    public:
        CConfiguation();

        /**
         * @Remarks: Init db and load all configuation template.
         * @Params: dbPath : the path of sqlite file.
         *          uuid : uuid string of the owner.
         * @Return: 
         */
		CResult Initialize( string dbPath, string uuid );

		CResult LoadConfig( string uuid );
        CResult SaveConfig( string uuid, string config );
		CResult SaveTemplate( string id, string config);
		CResult ReloadTemplate( string id );

        string GetConfig(string uuid);
		map<string, string> GetTemplateList() { return m_oTemplateList; }


    protected:
		CResult LoadConfigTemplate(string tbName);
        CResult GetStringConfig(string table_name, string key, string& outValue);
		CResult AddOrInsertRecord(const string& table_name, const map<string, string>& list, string where_str);

    protected:
        map<string,string> m_oServerConfigList;
        map<string,string> m_oTemplateList;

    protected:
        unique_ptr<CSQLiteEx> m_pDB;
    };


}

#endif