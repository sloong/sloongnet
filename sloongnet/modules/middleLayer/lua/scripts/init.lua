
__g_module_function__ = {}
__g_function_name__ = {}
__g_all_processer_request__ = {}

__g_result_value_to_name__ = {}

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
  for module_name, funcs in pairs(__g_module_function__) do
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
  __g_result_value_to_name__[Succeed] = 'Succeed'
  __g_result_value_to_name__[Error] = 'Error'
  __g_result_value_to_name__[Warning] = 'Warning'
  __g_result_value_to_name__[Retry] = 'Retry'
  __g_result_value_to_name__[Invalid] = 'Invalid'
end


Init = function( path )
    package.path = string.format('%s;%s/?.lua;%s/librarys/?.lua',package.path,path,path)
    package.cpath = string.format('%s;%s/?.so;%s/librarys/?.so',package.cpath,path,path)
    pb = require "pb"
    assert(pb.loadfile(path .. "/protobuf/base.pb"))
    assert(pb.loadfile(path .. "/protobuf/processer.pb"))
    JSON = (assert(loadfile(path .. '/librarys/json.lua')))()
    assert(require_ex('comm'),'load comm lua file failed.')
    assert(require_ex('main'),'load main lua file failed.')

    BuildRequestList()
    LoadResultValue()
end

OnError = function( msg )
    print('error proc:' .. tostring(msg));
end

SocketCloseProcess = function( u) 
	Info("socket closed")
end

ProgressMessage = function( funcid, uinfo, c_req, c_res )
    local str_req = c_req:getdata('request_message')
    local func_info = __g_all_processer_request__[funcid];
    local func_name = __g_function_name__[funcid] or ''
    Info('Call process function : ' ..  func_name  );
   
    local result,resmsg
    if type(func_info[1]) == 'function' then
      local req_table = nil
      local res_table = {}
      if #func_info[2] > 0 then
        req_table = assert(pb.decode(func_info[2],str_req))
      end
      result,errmsg = func_info[1]( uinfo, req_table, res_table );
      if result == Succeed and #func_info[3]>0 then
        resmsg = assert(pb.encode(func_info[3], res_table))
      else
        resmsg = errmsg
      end
    else
      result = Error
      resmsg = string.format('not find the processer. the name is %s.' ,func_name);
    end
    Info( string.format('Function [%s] << [%s]', func_name, __g_result_value_to_name__[result] ))
    c_res:setdata('response_result',result)
    c_res:setdata('response_message',resmsg);
end
