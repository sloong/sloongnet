--分割字符串
--str 需要分割的字符串，sp 分割符， ignorspace 指定是否忽略空的字段，默认忽略
--返回一个字符串数组
function string.split(str, delimiter)
	if str==nil or str=='' or delimiter==nil then
		return nil
	end

		local result = {}
		for match in (str..delimiter):gmatch("(.-)"..delimiter) do
			table.insert(result, match)
		end
		return result;
end

-- 增加扩展模块
function AddModule(bm)
	for cmd, req in pairs(bm) do
		g_all_request_processer[cmd] = req;
	end
end