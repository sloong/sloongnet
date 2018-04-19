-- When message is recved, the fm will call this function.
require_ex('ex');
require_ex('sql');

local main_Req = {};

main_Req.ReloadScript = function( u, req, res )
	Sloongnet_ReloadScript();
	return 0;
end


main_Req.TextTest = function( u, req, res )
        res['TestText'] = Sloongnet_GetEngineVer()  .. ' -- Sloong Network Engine -- Copyright 2015-2018 Sloong.com. All Rights Reserved';
        return 0
end

-- 上传文件流程
-- 客户端准备要上传的文件信�?,包括style �? 文件的md5,以及扩展�?
-- 服务端检查md5信息,并根据检查结�?,返回是否需要上�?.如无需上传则直接秒传并保存文件记录
-- 如需要上�?,则构建一个uuid, 将路径改为uploadurl/user/uuid+扩展名的格式返回.
-- 客户端根据返�?,将需要上传的文件传至指定目录.
-- 客户端发送UploadEnd消息,并附带参数为目标路径
-- 
-- 
-- 服务端按照年/�?/�?/uuid的结构来存储文件
-- get the total for the file need upload
-- then check the all file md5, if file is have one server, 
-- then gen the new guid and create the folder with the guid name.
-- return the path with guid.
-- then client upload the file to the folder, 
function main_Req.UploadStart(u, req, res)
	res['ftpuser']=Sloongnet_Get('FTP','User','');
	res['ftppwd']=Sloongnet_Get('FTP','Password','');
	Debug(res['filename'])
	res['filename']=req['filename'];
	res['fullname']=req['fullname'];
	local baseUrl = Sloongnet_Get('FTP','UploadUrl','') 
	res['ftpurl']=baseUrl
	-- get guid from c++
	--GetGUID()
	local uuid = Sloongnet_GenUUID();
	res['uuid']=uuid;
	-- Return a floder path.
	local path = uuid .. '/';
	res['filepath']=path;
	res['UploadURL'] = path;
	return 0;
end

function main_Req.UploadEnd( u, req, res )
	local folder = Sloongnet_Get('FTP','UploadFolder','')
	local path = folder .. req['UploadURL'] .. req['filename'];
	local newPath = folder .. os.date('%Y%m%d') .. '/' .. req['filename'];
	local errmsg ,errcode = Sloongnet_MoveFile(path,newPath);
	return errcode, errmsg;
end

function main_Req.GetIP( u, req, res )
	res['IPInfo'] = u:getdata('ip') .. ':' .. u:getdata('port')
	return 0;
end

function main_Req.UploadWithTCP( u, req, res )
	res['uuid'] = Sloongnet_GenUUID();
	res['port'] = '17001';
	return 0;
end

function main_Req.UploadWithTCPStart(u, req, res)
	local total = tonumber(req['total']) or 0
	local fielList = {};
	for i=1,total do
		local md5 = req['md5' .. tostring(i)]
		local filename = req['filename' .. tostring(i)]
		local path = '/tmp/sloong/'  .. filename;
		fielList[md5] = path;
	end
	local suc_num,suc_list,errmsg = Sloongnet_ReceiveFile(req['uuid'],17001,100*1024*1024,fielList,10,'/tmp/temp.tmp');
	if res == 0 then
		return -1,errmsg;
	else
		return 0;
	end
end

function main_Req.GetThumbImage(u,req, res)
    local path = Sloongnet_GetThumbImage(req['path'],100,100,5,'/tmp/thumbpath');
    return 0,path
end

g_main_request_processer = 
{
	['Reload'] = main_Req.ReloadScript,
	['GetText'] = main_Req.TextTest,
	['UploadStart'] = main_Req.UploadStart,
	['UploadEnd'] = main_Req.UploadEnd,
	['UploadWithTCP'] = main_Req.UploadWithTCP,
	['UploadWithTCPStart'] = main_Req.UploadWithTCPStart,
	['GetIP'] = main_Req.GetIP,
    ['GetThumbImage'] = main_Req.GetThumbImage,
}
AddModule(g_main_request_processer)