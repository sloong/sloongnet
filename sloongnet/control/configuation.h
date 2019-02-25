
#ifndef SLOONGNET_CONFIGUATION_H
#define SLOONGNET_CONFIGUATION_H

#include "main.h"

#include "MessageTypeDef.h"
using namespace ProtobufMessage;

#include "SQLiteEx.h"
namespace Sloong
{
    class CConfiguation
    {
    public:
        CConfiguation();


        bool Initialize( string tableName );

        bool LoadAll();
        bool SaveAll();

        void LoadControlConfig( string serverIp, ProtobufMessage::GLOBAL_CONFIG& config );
        void LoadProxyConfig( string serverIp, ProtobufMessage::PROXY_CONFIG& config );
        void LoadProcessConfig( string serverIp, ProtobufMessage::PROCESS_CONFIG& config );
        void LoadDataConfig( string serverIp, ProtobufMessage::DATA_CONFIG& config );
        void LoadDBConfig( string serverIp, ProtobufMessage::DB_CONFIG& config );

        void SaveControlConfig(){}
        void SaveProxyConfig(){}
        void SaveProcessConfig(){}
        void SaveDataConfig(){}
        void SaveDBConfig(){}

        string GetStringConfig(string table_name, string domain, string key, string def);

    protected:
        void LoadGlobalConfig(string domain,string ip,  ProtobufMessage::GLOBAL_CONFIG* config);
        bool GetBoolen( string domain, string ip, string key, bool def );
        string GetString( string domain, string ip,  string key, string def );
        int GetInt( string domain, string ip, string key, int def );

    public:
        ProtobufMessage::GLOBAL_CONFIG   m_oControlConfig;
        ProtobufMessage::PROXY_CONFIG    m_oProxyConfig;
        ProtobufMessage::PROCESS_CONFIG  m_oProcessConfig;
        ProtobufMessage::DATA_CONFIG     m_oDataConfig;
        ProtobufMessage::DB_CONFIG       m_oDBConfig;

    protected:
        unique_ptr<CSQLiteEx> m_pDB;
        string      m_strTableName;

    };


}

#endif