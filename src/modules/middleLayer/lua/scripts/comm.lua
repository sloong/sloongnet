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

function Debug(msg)
	ShowLog(msg,"Debug")
end

function Info(msg)
	ShowLog(msg,"Info")
end

function Warn(msg)
	ShowLog(msg,"Warn")
end

function Error(msg)
	ShowLog(msg,"Error")
end

function Assert(msg)
	ShowLog(msg,"Assert")
end

function Fatal(msg)
	ShowLog(msg,"Fatal")
end

function Verbos(msg)
	ShowLog(msg,"Verbos")
end

function Log(msg,title)
	if title == nil or title == '' then
		return;
	end
	ShowLog(msg,title)
end