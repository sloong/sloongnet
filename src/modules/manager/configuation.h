/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-26 12:09:48
 * @Description: file content
 */

#ifndef SLOONGNET_CONFIGUATION_H
#define SLOONGNET_CONFIGUATION_H

#include "SQLiteEx.h"
#include "SQLite_ORM.hpp"

namespace Sloong
{
    
    class CConfiguation
    {
    public:
        CConfiguation();

        /**
         * @Description: Init db and load all configuation template.
         * @Params: dbPath : the path of sqlite file.
         *          uuid : uuid string of the owner.
         * @Return: 
         */
		CResult Initialize( const string&);
        bool IsInituialized(){return m_bInitialized;}

        TResult<TemplateInfo> GetTemplate(int );
        bool CheckTemplateExist(int );
        vector<TemplateInfo> GetTemplateList();
        CResult AddTemplate( const TemplateInfo& , int* );
		CResult SetTemplate( int , const TemplateInfo& );
        CResult DeleteTemplate( int  );

    protected:
        mutex m_oMutex;
        unique_ptr<Storage> m_oStorage;
        bool    m_bInitialized = false;

    public:
        static unique_ptr<CConfiguation> Instance;
    };


}

#endif