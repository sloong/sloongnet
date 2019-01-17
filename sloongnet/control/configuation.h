
#ifndef SLOONGNET_CONFIGUATION_H
#define SLOONGNET_CONFIGUATION_H

#include "main.h"
namespace Sloong
{
    struct GLOBAL_CONFIG{
        int     ListenPort;
        string  LogPath;
        LOGLEVEL LogLevel;
        bool    DebugMode;
        int     MQThreadQuantity;
        bool    EnableSSL;
        string  CertFilePath;
        string  KeyFilePath;
        string  CertPasswd;
        int     ConnectTime;
        int     ReceiveTime;
        int     EPollThreadQuantity;
    };

    struct PROXY_CONFIG{
        int     ClientCheckTime;
        string  ClientCheckKey;
        int     TimeoutCheckInterval;
        GLOBAL_CONFIG   ServerConfig;
    };

    struct PROCESS_CONFIG{
        string  LuaContextQuantity;
        string  LuaScriptFolder;
        string  LuaEntryFile;
        string  LuaEntryFunction;
        string  LuaProcessFunction;
        string  LuaSocketCloseFunction;
        GLOBAL_CONFIG   ServerConfig;
    };

    struct DATA_CONFIG{
        int     DataReceivePort;
        int     WaitConnectTime;
        int     DataRecvTime;
        GLOBAL_CONFIG   ServerConfig;
    };

    struct DB_CONFIG{
        string  ServerAddress;
        int     ServerPort;
        string  User;
        string  Passwd;
        string  Database;
        GLOBAL_CONFIG   ServerConfig;
    };

    class CSQLiteEx;
    class CConfiguation
    {
        public:
        bool Initialize();

        bool LoadAll();
        bool SaveAll();

        void LoadControlConfig();
        void LoadProxyConfig( string serverIp, PROXY_CONFIG& config );
        void LoadProcessConfig();
        void LoadDataConfig();
        void LoadDBConfig();

        void SaveControlConfig();
        void SaveProxyConfig();
        void SaveProcessConfig();
        void SaveDataConfig();
        void SaveDBConfig();

    protected:
        void LoadGlobalConfig(string domain,string ip,  GLOBAL_CONFIG& config);
        bool GetBoolen( string domain, string ip, string key, bool def );
        string GetString( string domain, string ip,  string key, string def );
        int GetInt( string domain, string ip, string key, int def );

    public:
        GLOBAL_CONFIG   m_oControlConfig;
        PROXY_CONFIG    m_oProxyConfig;
        PROCESS_CONFIG  m_oProcessConfig;
        DATA_CONFIG     m_oDataConfig;
        DB_CONFIG       m_oDBConfig;

    protected:
        unique_ptr<CSQLiteEx> m_pDB;


    };


}

#endif