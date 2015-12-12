-- When message is recved, the fm will call this function.
print('epollproc is load')

local paserMessage = function( msg )
    --if msg == nil then
        print(msg)
        local res = string.split(msg,"|");
    for k,v in pairs(res) do
        print(v);
    end
    return res[1];
end


OnRecvMessage = function( uinfo, request, response )
	print('sql test')
	local res = querySql('select * from users')
	print(res);
showLog('insert test')
if '-1' == modifySql("INSERT INTO `sloong`.`users` (`id`, `name`, `regdate`, `solelyid`) VALUES ('2', 'gaoerer', '2015-12-11', '48971786')") then
    showLog(getSqlError())
   end

    print('Recv message process is called.')
    local msg = request:getdata("message");
    print(msg);
    local funcid = paserMessage(msg);
    if funcid == 'L-0-reload' then
        response:setdata("operation","reload");
        return true;
        --response:setdata("")
    end
    response:setdata("funcid",funcid);
    response:setdata("result","success");
end


-- send message to client
SendMessage = function( uinfo,request,result )
	result:setdata("type","send_message");
	result:setdata("user_id","333");
	result:setdata("message","hello");
end

