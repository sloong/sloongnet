function require_ex( _mname )
  if package.loaded[_mname] then
    print( string.format("require_ex module[%s] reload", _mname))
  end
  package.loaded[_mname] = nil
  return require( _mname )
end



Init = function( path )
    package.path = path .. 'scripts/?.lua';
    assert(require_ex('comm'),'load comm lua file failed.')
    assert(require_ex('main'),'load main lua file failed.')
    JSON = (assert(loadfile(path .. 'scripts/json.lua')))()
end

OnError = function( msg )
    print('error proc:' .. msg);
end


ProgressMessage = function( uinfo, request, response )
  local param = JSON:decode(request:getdata('message'))
   
    local func = g_all_request_processer[param['funcid']];

  if type(func) == 'function' then
    local code,res = func( uinfo, param, response );
    response:setdata('errno', tostring(code));
    response:setdata('errmsg',res or "success");
  else
    response:setdata("errno","-999");
    response:setdata('errmsg','not find the processer. the name is %s.' .. param['funcid']);
  end
end

