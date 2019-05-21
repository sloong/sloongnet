#include "configuation.h"

Sloong::CConfiguation::CConfiguation()
{
    m_pDB = make_unique<CSQLiteEx>();
    m_oServerConfigList[ModuleType::ControlCenter] = GLOBAL_CONFIG();
    m_oServerConfigList[ModuleType::Proxy] = GLOBAL_CONFIG();
    m_oServerConfigList[ModuleType::Process] = GLOBAL_CONFIG();
    m_oServerConfigList[ModuleType::Firewall] = GLOBAL_CONFIG();
    m_oServerConfigList[ModuleType::DataCenter] = GLOBAL_CONFIG();
    m_oServerConfigList[ModuleType::DBCenter] = GLOBAL_CONFIG();
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
    LoadFirewallConfig("",m_oFirewallConfig);
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
    config->set_processthreadquantity(GetInt(domain, ip, "ProcessThreadQuantity", 5));
    config->set_receivetime(GetInt(domain, ip, "ReceiveTime", 5));
    config->set_prioritysize(GetInt(domain, ip, "PrioritySize", 5));
}


void Sloong::CConfiguation::LoadControlConfig( string serverIp, ProtobufMessage::CONTROL_CONFIG& config )
{
    LoadGlobalConfig("control", serverIp, &m_oServerConfigList[ModuleType::ControlCenter]);
}

void Sloong::CConfiguation::LoadProxyConfig(string serverIp, PROXY_CONFIG &config)
{
    string module_name = "proxy";
    config.set_clientcheckkey( GetString(module_name, serverIp, "ClientCheckKey", "sloong.com"));
    config.set_clientchecktime( GetInt(module_name, serverIp, "ClientCheckTime", 2));
    config.set_timeoutcheckinterval( GetInt(module_name, serverIp, "TimeoutCheckInterval", 5));
    config.set_processaddress( GetString(module_name, serverIp, "ProcessAddress",""));
    LoadGlobalConfig(module_name, serverIp, &m_oServerConfigList[ModuleType::Proxy]);
}


void Sloong::CConfiguation::LoadProcessConfig( string serverIp, ProtobufMessage::PROCESS_CONFIG& config )
{
    string module_name = "process";
    config.set_luacontextquantity( GetInt(module_name,serverIp, "LuaContextQuantity", 10));
    config.set_luaentryfile(GetString(module_name,serverIp,"LuaEntryFile", "init.lua"));
    config.set_luaentryfunction(GetString(module_name, serverIp, "LuaEntryFunction", "Init"));
    config.set_luaprocessfunction(GetString(module_name, serverIp, "LuaProcessFunction", "ProgressMessage"));
    config.set_luascriptfolder(GetString(module_name, serverIp, "LuaScriptFolder", "./scripts"));
    config.set_luasocketclosefunction(GetString(module_name, serverIp, "LuaSocketCloseFunction", "SocketCloseProcess"));
    LoadGlobalConfig(module_name, serverIp, &m_oServerConfigList[ModuleType::Process]);
}


void Sloong::CConfiguation::LoadDataConfig( string serverIp, ProtobufMessage::DATA_CONFIG& config )
{
    string module_name = "data";
    config.set_datareceiveport(GetInt(module_name, serverIp, "DataReceivePort", 0));
    config.set_datarecvtime(GetInt(module_name, serverIp, "DataRecvTime", 5));
    LoadGlobalConfig("data", serverIp, &m_oServerConfigList[ModuleType::DataCenter]);
}

void Sloong::CConfiguation::LoadDBConfig( string serverIp, ProtobufMessage::DB_CONFIG& config )
{
    LoadGlobalConfig("db", serverIp, &m_oServerConfigList[ModuleType::DBCenter]);
}

void Sloong::CConfiguation::LoadFirewallConfig( string serverIp, ProtobufMessage::FIREWALL_CONFIG& config )
{
    string module_name = "firewall";
    LoadGlobalConfig(module_name, serverIp, &m_oServerConfigList[ModuleType::Firewall]);
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
        return def;
    }
    if( dbRes->GetLinesNum() == 1) 
    {
        return dbRes->GetData(0,"value");
    }
    else if ( dbRes->GetLinesNum() == 0) 
    {
        if( domain == "global")
            return def;
        return GetStringConfig(table_name,"global",key,def);
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
