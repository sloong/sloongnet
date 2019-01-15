require "sloongnet_mysql"

local Sql_Req = {};

local db = sloongnet_mysql.new();
local res,msg = db:Connect('127.0.0.1',3389,'root','pwe','db');
Sloongnet_ShowLog(msg);

local log_ptr =  Sloongnet_GetLogObject();
sloongnet_mysql.SetLog(log_ptr,true,true)

Sql_Req.SqlTest = function( u, req, res )
	local cmd = req['cmd'] or '';
	local code,res = db:Query(cmd);
	return code,res
end


g_sql_function = 
{
	['RunSql'] = Sql_Req.SqlTest,
}

AddModule(g_sql_function);
