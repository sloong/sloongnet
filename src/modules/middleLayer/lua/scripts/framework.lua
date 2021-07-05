---@diagnostic disable: undefined-global
--
-- Author: Chuanbin Wang - wcb@sloong.com
-- Date: 2021-04-06 14:39:16
-- LastEditTime: 2021-04-07 14:30:32
-- LastEditors: Chuanbin Wang
-- FilePath: /engine/src/modules/middleLayer/lua/scripts/framework.lua
-- symbol_custom_string_obkoro1: Copyright 2015-2020 Sloong.com. All Rights Reserved
-- Description:

-- @description: Functions provided by the framework
Framework = {}

-- @description: Show log.
-- @param :
--      msg(string) : log message.
--      title(string) : log title.
-- @return:
--      no
Framework.ShowLog = function(msg, title)
    ShowLog(msg, title)
end

-- @description: Get framework version.
-- @param :
--      no
-- @return:
--      1(string) : the version string.
Framework.GetEngineVer = function()
    local v = GetEngineVer()
    return v
end

-- @description: Encode with base64.
-- @param :
--      str(string) : data for need encode.
-- @return:
--      1(string) : encoded data
Framework.Base64Encode = function(str)
    local v = Base64Encode(str)
    return v
end

-- @description: Decode with base64.
-- @param :
--      str(string) : data for need decode.
-- @return:
--      1(string) : decoded data
Framework.Base64Decode = function(str)
    local v = Base64Decode(str)
    return v
end

-- @description: Calculate the hash value.
-- @param :
--      algorithm(int) : hash algorithm.  0(MD5), 1(SHA-1),2(SHA-256), 3(SHA-512). default by SHA-1
--      str(string) : data for need calculate hash.
--      is_file(string) : if want calculate the file hash, set true, and the second params set to file path. in default, this value is false.
-- @return:
--      1(string) : the hash.
Framework.HashEncode = function(algorithm, str, is_file)
    return HashEncode(str, algorithm, is_file)
end

-- @description: Reload script.
-- @param :
--      no
-- @return:
--      no
Framework.ReloadScript = function()
    ReloadScript()
end

-- @description: Get node config.
-- @param :
--      section(string) : section in config.
--      key(string) : key.
--      def(string) : default value.
-- @return:
--      1(string) : the value
Framework.GetConfig = function(section, key, def)
    local v = GetConfig(section, key, def)
    return v
end

-- @description: Generator uuid with libuuid
-- @param :
--      no
-- @return:
--      1(string) : the uuid
Framework.GenUUID = function()
    local uuid = GenUUID()
    return uuid
end

-- @description: Set comm data
-- @param :
--      key(string) : key
--      value(string) : value
-- @return:
--      no
Framework.SetCommData = function(key, value)
    SetCommData(key, value)
end

-- @description: Get comm data
-- @param :
--      key(string) : key
-- @return:
--      1(string) -> the data.
Framework.GetCommData = function(key)
    local s = GetCommData(key)
    return s
end

-- @description: Move file
-- @param :
--      org : source file.
--      new: new file.
-- @return:
--     1 -> 0 if succeed. else return nozero.
--     2 -> error message.
Framework.MoveFile = function(org, new)
    return MoveFile(org, new)
end

-- See SetExtendDataByFile function
Framework.SendFile = function()
end

Framework.ReceiveFile = function(save_folder, receive_list)
end
Framework.CheckRecvStatus = function()
end
Framework.SetExtendData = function()
end
Framework.SetExtendDataByFile = function()
end

-- @description: Connect to DBCenter
-- @param :
--      key(string) : key
-- @return:
--      1(string) -> the data.
Framework.ConnectToDBCenter = function()
end
Framework.SQLQueryToDBCenter = function()
end
Framework.SQLInsertToDBCenter = function()
end
Framework.SQLDeleteToDBCenter = function()
end
Framework.SQLUpdateToDBCenter = function()
end
Framework.PrepareUpload = function()
end
Framework.UploadEnd = function()
end

Framework.GetThumbnail = function()
end

--
-- @description: Send ConvertImageFormat request to file center.
-- @param:
--      uuid -> file index.
--      uuid(string) -> file index(uuid)
--      target(int) ->	target format with protocol : FileCenter.SupportFormat
--      quality(int) ->	quality. 0-100(best)\
--      retain_old_file(bool) -> retain old file when convert is over.
--      timeout(int) -> timeout
-- @return
--      1(int) -> Result with Base.ResultType
--      Error:
--          2	string	error message(Only error). If succeed, see next table.
--      Succeed:
--          2	table	new file info. key list:index, crc32, md5, sha1, sha256, size, format.
--          3	table	Only target is Best.extend file info. key list: crc32, md5, sha1, sha256, size, format.
--
Framework.ConvertImageFormat = function(uuid, target, quality, retain_old_file, timeout)
    return ConvertImageFormat(uuid, target, quality, retain_old_file, timeout)
end

Framework.SetTimeout = function(t)
    SetTimeout(t)
end

-- @description: Push event to framework and framework will call EventHandler with other thread.
-- @param :
--      event(int) : Event id.
--      params(string) : Serialized param.
-- @return:
--      no
Framework.PushEvent = function(event, params)
    PushEvent(event, params)
end
