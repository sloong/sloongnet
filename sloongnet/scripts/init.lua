function require_ex( _mname )
  if package.loaded[_mname] then
    print( string.format("require_ex module[%s] reload", _mname))
  end
  package.loaded[_mname] = nil
  return require( _mname )
end

g_all_request_processer = {};

Init = function( path )
    package.path = path .. 'scripts/?.lua';
    assert(require_ex('comm'),'load comm lua file failed.')
    assert(require_ex('main'),'load main lua file failed.')
    JSON = (assert(loadfile(path .. 'scripts/json.lua')))()
end

OnError = function( msg )
    print('error proc:' .. tostring(msg));
end

SocketCloseProcess = function( u) 
	Info("socket closed")
end

ProgressMessage = function( uinfo, c_req, c_res )
    local req = c_req:getdata('json_request_message')
    local jreq = JSON:decode(req)
    local jres = JSON:decode('{}')
    local func = g_all_request_processer[jreq['funcid']];
    Info('Call process function : ' .. jreq['funcid'] );
   
    if type(func) == 'function' then
      local code,msg,res,len = func( uinfo, jreq, jres );
      jres['errno'] = tostring(code);
      jres['errmsg'] = msg or 'success';
    else
      jres['errno'] = "-999"
      jres['errmsg'] = string.format('not find the processer. the name is %s.' ,jreq['funcid']);
    end
    Info('code:' .. jres['errno'] .. ',msg:' .. jres['errmsg'], 'Global')
    local str_res = JSON:encode(jres);
    c_res:setdata('json_response_message',str_res);
    if res then
	c_res:setdata('NeedExData',tostring(true))
	c_res:setdata('ExDataUUID',tostring(res))
	c_res:setdata('ExDataSize',tostring(len))
    else
	c_res:setdata('NeedExData',tostring(false))
    end
    return c_res;
end

