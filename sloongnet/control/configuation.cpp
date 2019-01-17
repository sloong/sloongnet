#include "configuation.h"

#include "SQLiteEx.h"

bool Sloong::CConfiguation::Initialize()
{
    m_pDB->Initialize("configuation.db");
}


bool Sloong::CConfiguation::LoadAll()
{
    LoadControlConfig();
    LoadDataConfig();
    LoadDBConfig();
    LoadProcessConfig();
    LoadProxyConfig("",m_oProxyConfig);
}

bool Sloong::CConfiguation::SaveAll()
{
    SaveControlConfig();
    SaveDataConfig();
    SaveDBConfig();
    SaveProcessConfig();
    SaveProxyConfig();
}

void Sloong::CConfiguation::LoadGlobalConfig(string domain, string ip, GLOBAL_CONFIG& config)
{
    config.LogPath = GetString(domain, ip, "LogPath", "/var/log/sloong/");
    config.LogLevel = LOGLEVEL(GetInt(domain, ip, "LogLevel", LOGLEVEL::Info));
    config.ListenPort = GetInt(domain, ip, "ListenPort", 0);
    config.CertFilePath = GetString(domain, ip, "CertFilePath", "");
    config.CertPasswd = GetString(domain, ip, "CertPasswd","");
    config.ConnectTime = GetInt(domain, ip, "ConnectTime", 2);
    config.DebugMode = GetBoolen(domain, ip, "DebugMode", false);
    config.EnableSSL = GetBoolen(domain, ip, "EnableSSL", false);
    config.EPollThreadQuantity = GetInt(domain, ip, "EPollThreadQuantity", 3);
    config.KeyFilePath = GetString(domain, ip, "KeyFilePath", "");
    config.MQThreadQuantity = GetInt(domain, ip, "MQThreadQuantity", 3);
    config.ReceiveTime = GetInt(domain, ip, "ReceiveTime", 5);
}

void Sloong::CConfiguation::LoadProxyConfig(string serverIp, PROXY_CONFIG& config)
{
    config.ClientCheckKey = GetString("proxy", serverIp, "ClientCheckKey", "sloong.com");
    config.ClientCheckTime = GetInt("proxy", serverIp, "ClientCheckTime", 2);
    config.TimeoutCheckInterval = GetInt("proxy", serverIp, "TimeoutCheckInterval", 5);
    LoadGlobalConfig("proxy", serverIp, config.ServerConfig);
}

bool Sloong::CConfiguation::GetBoolen( string domain, string ip,  string key, bool def )
{
    auto res = GetString(domain,ip, key,CUniversal::ntos(def));
    if(res.compare("true") == 0)
        return true;
    else if( res.compare("false") == 0)
        return false;
    else
        throw normal_except(CUniversal::Format("The config value in DB cannot convert to bool. doamin[%s],key[%s],value[%s]",domain,key,res));
}


string Sloong::CConfiguation::GetString( string domain, string ip, string key, string def )
{
    try
    {
        string value = m_pDB->QueryEx("configuation", "control", "LogPath");
        return value;
    }
    catch(normal_except e){
        return def;
    }
}


int Sloong::CConfiguation::GetInt( string domain, string ip, string key, int def )
{
    auto res = GetString(domain,ip, key,CUniversal::ntos(def));
    return atoi(res.c_str());
}


bool Sloong::CConfiguation::SaveAll()
{

}