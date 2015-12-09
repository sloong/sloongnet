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
    print('Recv message process is called.')
    local msg = request:getdata("message");
    print(msg);
    local funcid = paserMessage(msg);
    if funcid == '500010' then
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

