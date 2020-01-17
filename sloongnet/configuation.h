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

        string GetConfig(string uuid);
        CResult SetConfig( string uuid, string config );
        CResult SetConfigToTemplate(string uuid, int tpl_id);
        
        string GetTemplate(int id);
        map<int, string> GetTemplateList();
        CResult AddTemplate(string config, int* out_id);
		CResult SetTemplate( int id, string config);

    protected:
		CResult LoadDB();
        CResult AddConfig(string config, int* out_id);
        template<typename K, typename V>
        CResult LoadKeyValueList(string tbName, map<K, V>& out_list);
        CResult GetStringConfig(string table_name, string key, string& outValue);
		CResult AddOrUpdateRecord(const string& table_name, const map<string, string>& list, string where_str);

    protected:
        map_ex<string,string> m_oServerList;
        map_ex<string, string> m_oTemplateList;
        map_ex<string, string> m_oConfigList;

    protected:
        unique_ptr<CSQLiteEx> m_pDB;
    };


}

#endif