-- When message is recved, the fm will call this function.
require_ex('ex');

local main_Req = {};

main_Req.ReloadScript = function( u, req, res )
	ReloadScript();
	return 0;
end

main_Req.SqlTest = function( u, req, res )
	local cmd = req['cmd'] or '';
	showLog("run sql cmd:" .. cmd);
	local res = querySql(cmd);
	showLog(res);
	return 0,res;
end

main_Req.TextTest = function( u, req, res )
        res['TestText'] = getEngineVer()  .. ' -- Sloong Network Engine -- Copyright 2015 Sloong.com. All Rights Reserved';
        return 0
end

-- 上传文件流程
-- 客户端准备要上传的文件信息,包括style 和 文件的md5,以及扩展名
-- 服务端检查md5信息,并根据检查结果,返回是否需要上传.如无需上传则直接秒传并保存文件记录
-- 如需要上传,则构建一个uuid, 将路径改为uploadurl/user/uuid+扩展名的格式返回.
-- 客户端根据返回,将需要上传的文件传至指定目录.
-- 客户端发送UploadEnd消息,并附带参数为目标路径
-- 
-- 
-- 服务端按照年/月/日/uuid的结构来存储文件
-- get the total for the file need upload
-- then check the all file md5, if file is have one server, 
-- then gen the new guid and create the folder with the guid name.
-- return the path with guid.
-- then client upload the file to the folder, 
function main_Req.UploadStart(u, req, res)
	res['ftpuser']=Get('FTP','User','');
	res['ftppwd']=Get('FTP','Password','');
	showLog(res['filename'])
	res['filename']=req['filename'];
	res['fullname']=req['fullname'];
	local baseUrl = Get('FTP','UploadUrl','') 
	res['ftpurl']=baseUrl
	-- get guid from c++
	--GetGUID()
	local uuid = GenUUID();
	res['uuid']=uuid;
	-- Return a floder path.
	local path = uuid .. '/';
	res['filepath']=path;
	res['UploadURL'] = path;
	return 0;
end

function main_Req.UploadEnd( u, req, res )
	local folder = Get('FTP','UploadFolder','')
	local path = folder .. req['UploadURL'] .. req['filename'];
	local newPath = folder .. os.date('%Y%m%d') .. '/' .. req['filename'];
	local errmsg, errcode = MoveFile(path,newPath);
	if errcode ~= 0 then
		showLog(errmsg)
		res['errno'] = errcode
		res['errmsg'] = errmsg
	end
end

g_all_request_processer = 
{
	['Reload'] = main_Req.ReloadScript,
	['GetText'] = main_Req.TextTest,
	['RunSql'] = main_Req.SqlTest,
	['UploadStart'] = main_Req.UploadStart,
	['UploadEnd'] = main_Req.UploadEnd,
}
AddModule(g_ex_function);
