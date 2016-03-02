local Ex_Req = {};


Ex_Req.GetFileData = function( u, req, res )
	local cmd = "SELECT * FROM `fileList` WHERE `fileMD5`='" .. req['md5'] .. "'"
	showLog("run sql cmd:" .. cmd);
	local res = querySql(cmd);
	showLog(res);
	return 0,res;
end

Ex_Req.SetUInfo = function( u, req, res )
	local key = req['key'];
	local value = req['value'];
	u:setdata(key,value);
	return 0, 'set ' .. key .. ' to ' .. value;
end

Ex_Req.GetUInfo = function( u, req, res )
	local key = req['key']
	return 0,u:getdata(key) or 'nil';
end

g_ex_function = 
{
	['GetFile'] = Ex_Req.GetFileData,
	['SetUInfo'] = Ex_Req.SetUInfo,
	['GetUInfo'] = Ex_Req.GetUInfo,
}

