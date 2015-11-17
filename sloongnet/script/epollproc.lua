-- When message is recved, the fm will call this function.
OnRecvMessage = function( uinfo, request, result )
	local msg = request:getdata("mesaage");
    paserMessage(msg);
	result:setdata("result","success");
end


-- send message to client
SendMessage = function( uinfo,request,result )
	result:setdata("type","send_message");
	result:setdata("user_id","333");
	result:setdata("message","hello");
end

local paserMessage = function( msg )
	local res = string.split(msg,"|");
    for k,v in pairs(res) do
        print(v);
    end
end