local Ex_Req = {};


Ex_Req.GetFileData = function( u, req, res )
	local md5 = req['MD5'] or nil;
	local h = tonumber(req['height']) or nil
	local w = tonumber(req['width']) or nil
	local q = tonumber(req['quality']) or 5
	if not md5 or not h or not w then
		return -1,'param error';
	end
	local cmd = "SELECT `Path` FROM `Walls_FileList` WHERE `MD5`='" .. md5 .. "'"
	showLog("run sql cmd:" .. cmd);
	local res = querySql(cmd);
	showLog(res);
	local thumbpath = Sloongnet_GetThumbImage(res,w,h,q)
	local errno,errmsg=Sloongnet_SendFile(thumbpath);
	if errno == -1 then
		return errno,errmsg;
	else
		return 0,thumbpath,errno;
	end
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

AddModule(g_ex_function);
