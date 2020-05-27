package.path = package.path ..';./librarys/?.lua';
package.cpath = package.cpath .. ';./librarys/?.so'

__g_module_function__ = {};
__g_all_processer_request__ ={};

function require_ex( _mname )
  if package.loaded[_mname] then
    print( string.format("require_ex module[%s] reload", _mname))
  end
  package.loaded[_mname] = nil
  return require( _mname )
end

function AddModule(name,bm)
	__g_module_function__[name] = bm;
end

function BuildRequestList()
  for module_name, funcs in pairs(g_module_function) do
    for func_name, func in pairs(funcs) do
      __g_all_processer_request__[pb.enum(module_name,func_name)] = func;
    end
	end
end


Init = function( path )
    local pb = require "pb" -- 载入 pb.dll
    assert(pb.loadfile "protobuf/core.pb")
    assert(pb.loadfile "protobuf/processer.pb")
    JSON = (assert(loadfile(path .. 'json.lua')))()
    assert(require_ex('comm'),'load comm lua file failed.')
    assert(require_ex('main'),'load main lua file failed.')

    BuildRequestList()
end

OnError = function( msg )
    print('error proc:' .. tostring(msg));
end

SocketCloseProcess = function( u) 
	Info("socket closed")
end

ProgressMessage = function( func, c_req, uinfo, c_res )
    local req = c_req:getdata('json_request_message')
    local jreq = JSON:decode(req)
    local jres = JSON:decode('{}')
    local func = __g_all_processer_request__[jreq['funcid']];
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
