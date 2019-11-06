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
        bool Initialize( string dbPath, string uuid );

        bool LoadConfig( string uuid );
        bool SaveConfig( string uuid );
        bool SaveTemplate( string id );
        bool ReloadTemplate( string id );

        string GetConfig(string uuid);

    protected:
        bool LoadConfigTemplate(string tbName);
        string GetStringConfig(string table_name, string key, string def);

    protected:
        map<string,string> m_oServerConfigList;
        map<string,string> m_oTemplateList;

    protected:
        unique_ptr<CSQLiteEx> m_pDB;
    };


}

#endif