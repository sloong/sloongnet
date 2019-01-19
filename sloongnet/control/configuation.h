
#ifndef SLOONGNET_CONFIGUATION_H
#define SLOONGNET_CONFIGUATION_H

#include "main.h"

#include "config.pb.h"
using namespace MessageConfig;

#include "SQLiteEx.h"
namespace Sloong
{
    class CConfiguation
    {
        public:
        bool Initialize( string tableName );

        bool LoadAll();
        bool SaveAll();

        void LoadControlConfig( string serverIp, MessageConfig::GLOBAL_CONFIG& config );
        void LoadProxyConfig( string serverIp, MessageConfig::PROXY_CONFIG& config );
        void LoadProcessConfig( string serverIp, MessageConfig::PROCESS_CONFIG& config );
        void LoadDataConfig( string serverIp, MessageConfig::DATA_CONFIG& config );
        void LoadDBConfig( string serverIp, MessageConfig::DB_CONFIG& config );

        void SaveControlConfig(){}
        void SaveProxyConfig(){}
        void SaveProcessConfig(){}
        void SaveDataConfig(){}
        void SaveDBConfig(){}

        string GetStringConfig(string domain, string key, string def);

    protected:
        void LoadGlobalConfig(string domain,string ip,  MessageConfig::GLOBAL_CONFIG* config);
        bool GetBoolen( string domain, string ip, string key, bool def );
        string GetString( string domain, string ip,  string key, string def );
        int GetInt( string domain, string ip, string key, int def );

    public:
        MessageConfig::GLOBAL_CONFIG   m_oControlConfig;
        MessageConfig::PROXY_CONFIG    m_oProxyConfig;
        MessageConfig::PROCESS_CONFIG  m_oProcessConfig;
        MessageConfig::DATA_CONFIG     m_oDataConfig;
        MessageConfig::DB_CONFIG       m_oDBConfig;

    protected:
        unique_ptr<CSQLiteEx> m_pDB;
        string      m_strTableName;

    };


}

#endif