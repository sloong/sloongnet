package.path = package.path ..';./librarys/?.lua'
package.cpath = package.cpath .. ';./librarys/?.so'

__g_module_function__ = {}
__g_function_name__ = {}
__g_all_processer_request__ = {}


function require_ex( _mname )
  if package.loaded[_mname] then
    print( string.format("require_ex module[%s] reload", _mname))
  end
  package.loaded[_mname] = nil
  return require( _mname )
end

function AddModule(name,bm)
	__g_module_function__[name] = bm
end

function BuildRequestList()
  for module_name, funcs in pairs(g_module_function) do
    for func_name, func_info in pairs(funcs) do
      local id = pb.enum(module_name,func_name)
      __g_function_name__[id] = func_name
      __g_all_processer_request__[id] = func_info;
    end
	end
end

function LoadResultValue()
  Succeed = pb.enum('Base.ResultType','Succeed')
  Error = pb.enum('Base.ResultType','Error')
  Warning = pb.enum('Base.ResultType','Warning')
  Retry = pb.enum('Base.ResultType','Retry')
  Invalid = pb.enum('Base.ResultType','Invalid')
end


Init = function( path )
    local pb = require "pb" -- 载入 pb.dll
    assert(pb.loadfile "protobuf/base.pb")
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

ProgressMessage = function( func, uinfo, c_req, c_res )
    local str_req = c_req:getdata('request_message')
    local func = __g_all_processer_request__[jreq[func]];
    local func_name = __g_function_name__[func]
    Info('Call process function : ' ..  func_name  );
   
    local res,msg
    if type(func[1]) == 'function' then
      local req = {}
      local res = {}
      if #func[2] > 0 then
        req = assert(pb.decode(func[2],str_req))
      end
      result,errmsg = func( uinfo, req, res );
      if result == Succeed and #func[3]>0 then
        msg = assertpb.encode(func[3], res))
      else
        msg = errmsg
      end
    else
      res = Error
      msg = string.format('not find the processer. the name is %s.' ,func_name);
    end
    Info( string.format('Function [%s] << [%s][%s]', func_name, msg))
    c_res:setdata('response_result',res)
    c_res:setdata('response_message',msg);
end
