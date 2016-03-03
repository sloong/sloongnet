-- When message is recved, the fm will call this function.
require_ex('ex');

local main_Req = {};

main_Req.ReloadScript = function( u, req, res )
	res:setdata('operation','ReloadScript');
	return 0;
end

main_Req.SqlTest = function( u, req, res )
	local cmd = req['cmd'] or '';
	showLog("run sql cmd:" .. cmd);
	local res = querySql(cmd);
	showLog(res);
	return 0,res;
end

main_Req.TextTest = function( u, req, res )
	return 0, getEngineVer()  .. ' -- Sloong Network Engine -- Copyright 2015 Sloong.com. All Rights Reserved';
end

g_all_request_processer = 
{
	['Reload'] = main_Req.ReloadScript,
	['GetText'] = main_Req.TextTest,
	['RunSql'] = main_Req.SqlTest,
}
AddModule(g_ex_function);