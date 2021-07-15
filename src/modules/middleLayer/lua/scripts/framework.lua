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

-- @description: Reload script file. When call this interface, engine will do reload operation in next message process for current process thread. 
--      If have more than one process thread, the other thread reload need wait next process message time.
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
--      1(string) : the data.
Framework.GetCommData = function(key)
    local s = GetCommData(key)
    return s
end

-- @description: Call system API to move one file. 
-- @param :
--      org : source file.
--      new: new file.
-- @return:
--     1 : 0 if succeed. else return nozero.
--     2 : error message.
Framework.MoveFile = function(org, new)
    return MoveFile(org, new)
end

-- See SetExtendDataByFile function
Framework.SendFile = function(file_path)
    return Framework.SetExtendDataByFile(file_path)
end

-- @description: Receive file from client with tcp/ip mode. Enable this function with config file 
--      ```
--      [Server]
--      EnableDataReceive=true
--      ```
--      and set the data receive port for config 
--      ```
--      [Server]
--      DataReceivePort=8008
--      ```
-- @param :
--      save_folder : save folder.
--      receive_list: file list. format is Lua Table. key is hash string, value is file name.
-- @return:
--     1 : send request UUID.
-- @remarks:
--     Client need connect the data translate before this function return . 
--     When connect ready, need send the uuid (36Bit) string first in 3(default)* sencond. 
--     Before send UUID string, start send each file info data.
--     data format:
--     {file length with 8bit}->{file hash with 32bit}->{file data}.
--     *1 > Client check time is define in config file :
--     ```
--     [Security]
--     ClientCkeckTime=3
--     ```
--     This function is used the socket base on tcp/ip block mode. So if the receive course no done, the function will block. So should set the max size with different user. And in once time, no call too many receive function.
--     And then, the client after connect, must send the package according the order:
Framework.ReceiveFile = function(save_folder, receive_list)
    return ReceiveFile(receive_list,save_folder)
end
-- @description: When send the file done, call this function can check the recv status. 
-- @param :
--      uuid : the uuid when call ReceiveFile function.
--      hash: target file hash string.
-- @return:
--     1 : status code, 0 > Wait, 1 > Receiving, 2 > Saving, 3 > Done, 4 > VerificationError, 5 > OtherError
--     2 : save path if receive succeed. else is the error message.
Framework.CheckRecvStatus = function(uuid,hash)
    return CheckRecvStatus(uuid,hash)
end
-- @description: Set extend data to engine pool.
-- @param :
--      data : data with string format.
-- @return:
--     1(string) : The resource index in engine. 
Framework.SetExtendData = function(data)
    return SetExtendData(data)
end
-- @description: Send one file to current socket. Engine will load the file and return the index, when lua script returned, need return this index to engine, so just can send one file in one time.
-- @param :
--      file_path : the file path
-- @return:
--     1 : The file resource index in engine. 
Framework.SetExtendDataByFile = function(file_path)
    return SetExtendDataByFile(file_path)
end

-- @description: Connect to DBCenter
-- @param :
--      db_name(string) : target database name.
-- @return:
--      1(int) : the Result.
--      Error: 
--          2 : is the error message with string.
--      Succeed:
--          2 : session id with int.  
Framework.ConnectToDBCenter = function(db_name)
    return ConnectToDBCenter(db_name)
end


-- @description: Send query sql to DBCenter
-- @param :
--      sessionid(int) : DBCenter session id.
--      sql(string) : query cmd
-- @return:
--      1(int) : the Result.
--      Error: 
--          2 : is the error message with string.
--      Succeed:
--          2(int) : Total number of query results
--          3(table) : detail for query results.
Framework.SQLQueryToDBCenter = function(sessionid, sql)
    return SQLQueryToDBCenter(sessionid,sql)
end

-- @description: Send insert sql to DBCenter
-- @param :
--      sessionid(int) : DBCenter session id.
--      sql(string) : insert cmd
-- @return:
--      1(int) : the Result.
--      Error: 
--          2 : is the error message with string.
--      Succeed:
--          2(int) : identity rows for sql cmd.
Framework.SQLInsertToDBCenter = function(sessionid, sql)
    return SQLInsertToDBCenter(sessionid, sql)
end

-- @description: Send delete sql to DBCenter
-- @param :
--      sessionid(int) : DBCenter session id.
--      sql(string) : delete cmd
-- @return:
--      1(int) : the Result.
--      Error: 
--          2 : is the error message with string.
--      Succeed:
--          2(int) : identity rows for sql cmd.
Framework.SQLDeleteToDBCenter = function(sessionid, sql)
    return SQLDeleteToDBCenter(sessionid, sql)
end

-- @description: Send update sql to DBCenter
-- @param :
--      sessionid(int) : DBCenter session id.
--      sql(string) : update cmd
-- @return:
--      1(int) : the Result.
--      Error: 
--          2 : is the error message with string.
--      Succeed:
--          2(int) : identity rows for sql cmd.
Framework.SQLUpdateToDBCenter = function(sessionid, sql)
    return SQLUpdateToDBCenter(sessionid, sql)
end

-- @description: Prepare to update load file to file center.
-- @param :
--      hash(string) : the hash value for file.
--      size(int) : the file size.
-- @return:
--      1(int) : the Result.
--      Error: 
--          2 : is the error message with string.
--      Succeed:
--          2(string) : upload token.
-- @remarks:
--      When get upload token, the client just need send the file data with FileCenter.Upload. 
--      The Gateway will trans the data to filecenter node. when upload succeeded, call the UploadEnd to confirm the upload information.
Framework.PrepareUpload = function(hash, size)
    return PrepareUpload(hash,size)
end

-- @description: File upload end
-- @param :
--      token(string) : the file upload token
-- @return:
--      1(int) : the Result.
--      2 : The message for result.
Framework.UploadEnd = function(token)
    return UploadEnd(token)
end

-- @description: Get thumbnail image from file center.
-- @param :
--      uuid(string) : uuid for original file.
--      h(int) : height for thumbnail file.
--      w(int) : width for thumbnail file.
--      q(int) : quality for thumbnail file.
-- @return:
--      1(int) : result with Base.ResultType.
--      2(string) : The extend data id. which is returned as the thrird parameter when the function returns.
Framework.GetThumbnail = function(uuid, h, w, q)
    local r, s = GetThumbnail(uuid, h, w, q)
    return r, s
end

--
-- @description: Send ConvertImageFormat request to file center.
-- @param:
--      uuid : file index.
--      uuid(string) : file index(uuid)
--      target(int) :	target format with protocol : FileCenter.SupportFormat
--      quality(int) :	quality. 0-100(best)\
--      retain_old_file(bool) : retain old file when convert is over.
--      timeout(int) : timeout
-- @return
--      1(int) : Result with Base.ResultType
--      Error:
--          2	string	error message(Only error). If succeed, see next table.
--      Succeed:
--          2	table	new file info. key list:index, crc32, md5, sha1, sha256, size, format.
--          3	table	Only target is Best.extend file info. key list: crc32, md5, sha1, sha256, size, format.
--
Framework.ConvertImageFormat = function(uuid, target, quality, retain_old_file, timeout)
    return ConvertImageFormat(uuid, target, quality, retain_old_file, timeout)
end

-- @description: Set the framework operation timeout time. such as the send request to DBCenter or filecenter. 
-- @param :
--      timeout(int): timeout time.
-- @returns
--      no
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
