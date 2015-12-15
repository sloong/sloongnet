-- When message is recved, the fm will call this function.
print('epollproc is load')


OnRecvMessage = function( uinfo, request, response )
    local msg = string.split(request:getdata("message"),'|');
	local func = g_all_function[msg[1]];
	
	if type(func) == 'function' then
		local res,msg = func(uinfo, request, response, msg);
		response:setdata('errno', tostring(res));
		response:setdata('errmsg',msg or "success");
	else
		response:setdata("errno","2");
		response:setdata('errmsg','not find the processer. the name is %s.' .. msg[1]);
	end
end

local Func_ReloadScript = function( uinfo, request, response, msg )
	response:setdata("operation","reload");
    return 0;
end


local Func_QueryUserInfo = function( uinfo, request, response, msg )
	local row = msg[2];
	if not row or row == '' then
		row = '*';
	end
	local where = msg[3] or '';
	if where and where ~= '' then
		where = 'WHERE ' .. where;
	end
	
	local cmd = 'SELECT ' .. row .. ' FROM users ' .. where ;
	showLog("run sql cmd:" .. cmd);
	local res = querySql(cmd);
	showLog(res);
	response:setdata('message',res);
	return 0;
end

local Func_AddUser = function( uinfo, request, response, msg )
	local cmd = string.format("INSERT INTO `sloong`.`users` (`id`, `name`, `regdate`, `solelyid`) VALUES ( %d, '%s', '%s', %d )", msg[2] ,msg[3], tostring(os.date()) , 8988787668)
	showLog("run sql cmd:" .. cmd);
	local res = modifySql(cmd);
	if res == '-1' then
		local err = getSqlError()
		showLog(err);
		return 1, err;
	end
	showLog('succeed. add rows:' .. res);
	response:setdata('message','succeed. add rows:' .. res);
	return 0, res
end

local Func_ReadFile = function( uinfo, request, response, msg )
	response:setdata('operation','loadfile');
	response:setdata('message',msg[2]);
	return 0
end

g_all_function = 
{
	['50001'] = Func_ReloadScript,
	['60001'] = Func_QueryUserInfo,
	['60002'] = Func_AddUser,
	-- ['60003'] = Func_UpdateUserInfo,
	-- ['60004'] = Func_DeleteUser,
	['70001'] = Func_ReadFile,
}

-- send message to client
SendMessage = function( uinfo,request,result )
	result:setdata("type","send_message");
	result:setdata("user_id","333");
	result:setdata("message","hello");
end

