#include "configuation.h"

Sloong::CConfiguation::CConfiguation()
{
    m_pDB = make_unique<CSQLiteEx>();
}


bool Sloong::CConfiguation::Initialize(string tableName)
{
    m_pDB->Initialize("configuation.db");
    m_strTableName = tableName;
}

bool Sloong::CConfiguation::LoadAll()
{
    LoadControlConfig("",m_oControlConfig);
    LoadDataConfig("",m_oDataConfig);
    LoadDBConfig("",m_oDBConfig);
    LoadProcessConfig("",m_oProcessConfig);
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


void Sloong::CConfiguation::LoadControlConfig( string serverIp, ProtobufMessage::GLOBAL_CONFIG& config )
{
    LoadGlobalConfig("control", serverIp, &config);
}

void Sloong::CConfiguation::LoadProxyConfig(string serverIp, PROXY_CONFIG &config)
{
    config.set_clientcheckkey( GetString("proxy", serverIp, "ClientCheckKey", "sloong.com"));
    config.set_clientchecktime( GetInt("proxy", serverIp, "ClientCheckTime", 2));
    config.set_timeoutcheckinterval( GetInt("proxy", serverIp, "TimeoutCheckInterval", 5));
    LoadGlobalConfig("proxy", serverIp, config.mutable_serverconfig());
}


void Sloong::CConfiguation::LoadProcessConfig( string serverIp, ProtobufMessage::PROCESS_CONFIG& config )
{
    //config.set_luaentryfile()
}


void Sloong::CConfiguation::LoadDataConfig( string serverIp, ProtobufMessage::DATA_CONFIG& config )
{

}

void Sloong::CConfiguation::LoadDBConfig( string serverIp, ProtobufMessage::DB_CONFIG& config )
{

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

string Sloong::CConfiguation::GetStringConfig(string table_name, string domain, string key, string def)
{
    EasyResult dbRes = make_shared<CDBResult>();
    string error;
    string sql = CUniversal::Format("SELECT `ip`,`value` FROM `%s` WHERE `domain`=\"%s\" and `key`=\"%s\"",
                                    table_name.c_str(), domain.c_str(), key.c_str());

    if (!m_pDB->Query(sql, dbRes, error))
    {
        // m_pLog->Error(error);
        return def;
    }
    if( dbRes->GetLinesNum() == 1) 
    {
        return dbRes->GetData(0,"value");
    }
    // TODO: need support the ip config.
    else
    {
        throw normal_except("No support function.");
    }
  
}

string Sloong::CConfiguation::GetString(string domain, string ip, string key, string def)
{
    try
    {
        string value = GetStringConfig(m_strTableName, domain, key,def);
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
