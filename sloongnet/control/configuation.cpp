#include "configuation.h"


bool Sloong::CConfiguation::Initialize(string tableName)
{
    m_pDB->Initialize("configuation.db");
    m_strTableName = tableName;
}

bool Sloong::CConfiguation::LoadAll()
{
    LoadControlConfig();
    LoadDataConfig();
    LoadDBConfig();
    LoadProcessConfig();
    LoadProxyConfig("", m_oProxyConfig);
}

bool Sloong::CConfiguation::SaveAll()
{
    SaveControlConfig();
    SaveDataConfig();
    SaveDBConfig();
    SaveProcessConfig();
    SaveProxyConfig();
}

void Sloong::CConfiguation::LoadGlobalConfig(string domain, string ip, GLOBAL_CONFIG* config)
{
    config->set_loglevel(GetInt(domain, ip, "LogLevel", LOGLEVEL::Info));
    config->set_logpath(GetString(domain, ip, "LogPath", "/var/log/sloong/"));
    config->set_listenport(GetInt(domain, ip, "ListenPort", 0));
    config->set_certfilepath(GetString(domain, ip, "CertFilePath", ""));
    config->set_certpasswd(GetString(domain, ip, "CertPasswd", ""));
    config->set_connecttime(GetInt(domain, ip, "ConnectTime", 2));
    config->set_debugmode(GetBoolen(domain, ip, "DebugMode", false));
    config->set_enablessl(GetBoolen(domain, ip, "EnableSSL", false));
    config->set_epollthreadquantity(GetInt(domain, ip, "EPollThreadQuantity", 3));
    config->set_keyfilepath(GetString(domain, ip, "KeyFilePath", ""));
    config->set_mqthreadquantity(GetInt(domain, ip, "MQThreadQuantity", 3));
    config->set_receivetime(GetInt(domain, ip, "ReceiveTime", 5));
}

void Sloong::CConfiguation::LoadProxyConfig(string serverIp, PROXY_CONFIG &config)
{
    config.set_clientcheckkey( GetString("proxy", serverIp, "ClientCheckKey", "sloong.com"));
    config.set_clientchecktime( GetInt("proxy", serverIp, "ClientCheckTime", 2));
    config.set_timeoutcheckinterval( GetInt("proxy", serverIp, "TimeoutCheckInterval", 5));
    LoadGlobalConfig("proxy", serverIp, config.mutable_serverconfig());
}

bool Sloong::CConfiguation::GetBoolen(string domain, string ip, string key, bool def)
{
    auto res = GetString(domain, ip, key, CUniversal::ntos(def));
    if (res.compare("true") == 0)
        return true;
    else if (res.compare("false") == 0)
        return false;
    else
        throw normal_except(CUniversal::Format("The config value in DB cannot convert to bool. doamin[%s],key[%s],value[%s]", domain, key, res));
}

string Sloong::CConfiguation::GetString(string domain, string ip, string key, string def)
{
    try
    {
        string value = m_pDB->QueryEx(m_strTableName, domain, key);
        return value;
    }
    catch (normal_except e)
    {
        return def;
    }
}

int Sloong::CConfiguation::GetInt(string domain, string ip, string key, int def)
{
    auto res = GetString(domain, ip, key, CUniversal::ntos(def));
    return atoi(res.c_str());
}
