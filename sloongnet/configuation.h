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
#include "SQLite_ORM.hpp"

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
		CResult Initialize( const string& dbPath );

        CResult GetConfig( const string& uuid );
        CResult SetConfig(const string& uuid, string config );
        CResult SetConfigToTemplate(const string& uuid, int tpl_id);
        
        CResult GetTemplate(int id);
        map<int, string> GetTemplateList();
        CResult AddTemplate(string config, int* out_id);
		CResult SetTemplate( int id, string config);

    protected:
        TResult<int> AddConfig(string config);
        CResult GetStringConfig(string table_name, string key, string& outValue);
		CResult AddOrUpdateRecord(const string& table_name, const map<string, string>& list, string where_str);

    protected:
        unique_ptr<Storage> m_oStorage;
    };


}

#endif