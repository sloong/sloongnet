-- When mesaage is recved, the fm will call this function.
OnRecvMessage = function( uinfo, request, result )
	local msg = request:getdata("mesaage");
    local res = string.split(msg,"|");
    for k,v in pairs(res) do
        print(v);
    end
	result:setdata("result","success");
end
