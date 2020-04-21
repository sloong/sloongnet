/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-21 12:06:15
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

        TResult<TemplateInfo> GetTemplate(int id);
        vector<TemplateInfo> GetTemplateList();
        CResult AddTemplate( const TemplateInfo& config, int* out_id);
		CResult SetTemplate( int id, const TemplateInfo& config);
        CResult DeleteTemplate( int id );

    protected:
        unique_ptr<Storage> m_oStorage;
    };


}

#endif