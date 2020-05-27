package.path = package.path ..';./librarys/?.lua';
package.cpath = package.cpath .. ';./librarys/?.so'
local pb = require "pb" -- 载入 pb.dll
local protoc = require "protoc"

assert(pb.loadfile "protobuf/core.pb") -- 载入刚才编译的pb文件
assert(pb.loadfile "protobuf/processer.pb")

-- print(pb.type "ResultType")

-- print(pb.field("Processer.Functions", "GetText"))

-- notice that you needn't receive all return values from iterator
-- for name, number, type in pb.fields "Core.ResultType" do
--   print(name, number, type)
-- end

-- for name, number, type in pb.fields "Processer.Functions" do
--   print(name, number, type)
-- end

-- for name, basename, type in pb.types() do
--     print(name, basename, type)
--   end

-- encode lua table data into binary format in lua string and return
-- local bytes = assert(pb.encode("Person", data))
-- print(pb.tohex(bytes))

-- and decode the binary data back into lua table
--local data2 = assert(pb.decode("Person", bytes))
--print(require "serpent".block(data2))
TextTest = function( )
  print("t")
end

g_module_function = {
  ['Processer.Functions'] = {
    ['Reload'] = TextTest,
  }
};
g_all_request_processer ={};


function BuildRequestList()
  for module_name, funcs in pairs(g_module_function) do
    -- print(module_name,funcs)
    for func_name, func in pairs(funcs) do
      -- print(func_name,func)
      id = pb.enum(module_name,func_name)
      print(module_name .. '.' .. func_name, id)
      g_all_request_processer[id] = func;
    end
	end
end

BuildRequestList();
g_all_request_processer[1]()
