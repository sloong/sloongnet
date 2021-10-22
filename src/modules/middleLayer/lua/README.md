# Copyright

&copy;2018 Sloong.com

[TOC]

## Default config

{
    "LuaContextQuantity":3,
    "LuaScriptFolder":"./scripts",
    "LuaEntryFile":"init.lua",
    "LuaEntryFunction":"Init",
    "LuaSocketCloseFunction":"SocketCloseProcess",
    "LuaProcessFunction":"ProgressMessage",
    "LuaEventFunction":"ProcessEvent"
}

## Default function ranges

Start 601 - *

## Reference modules

FileCenter
DataCenter

## Interface list

### ShowLog

Write log message to log file and standard output environment.

Params|Type|Note
------|----|----
1|string| log message.
2|string| log title. In log file it will show like this:[Script:$Title]:[$log message]

Return:
no

### GetEngineVer

Get engine version info

Params:
no

Returns|Type|Note
------|---|----
1|string|version info.

### Base64Encode

Params|Type|Note
------|----|----
1 |string |The string of need encode with base64.

Returns|Type|Note
------|---|----
1 | string|Encoding result

### Base64Decode

Params|Type|Note
------|----|----
1 |string |The string of need Decode with base64.

Returns|Type|Note
------|---|----
1 | string|Decoding result

### HashEncode

Params|Type|Note
------|----|----
1 |string |need hash string. if file mode, is the file path
2 |int | hash type. support : 0(MD5), 1(SHA-1),2(SHA-256), 3(SHA-512). default by SHA-1
3 |bool | file mode. default by false

Returns|Type|Note
------|---|----
1 | string|hash result

### ReloadScript

Reload script file. When call this interface, engine will do reload operation in next message process for current process thread. If have more than one process thread, the other thread reload need wait next process message time.
Params:
no
Return:
no

### GetConfig

Get config item value from run config file.
Params|Type|Note
-------|-------|--------
1|string|The config item section.
2|string|The config item key.
3|string|The config item default value.

Return|Type|Note
-------|-------|------
1|string|The config item value

### GenUUID

Gen new UUID
Params:
no

Return|Type|Note
-------|-------|------
1|string|New uuid

### SetCommData

No realization

### GetCommData

No realization

### MoveFile

Call system API to move one file.
Params|Type|Note
-------|-------|------
1|string|The old file path
2|string|The new file path

Return|Type|Note
-------|-------|------
1|int|return 0 if succeed. If param is empty ,return -2. If no access to old file or new folder, return -1.
2|string|error message.

### SendFile

See

[SetExtendDataByFile]: #SetExtendDataByFile "1"

### ReceiveFile

Receive file from client with tcp/ip mode. Enable this function with config file

```json
[Server]
EnableDataReceive=true
```

and set the data receive port for config

```json
[Server]
DataReceivePort=8008
```

Params|Note
------|----
1 | save forlder
2 | file list. format is __Lua Table__. key is hash string, value is file name.

Returns|Note
------|----
1 | send request UUID.

Remarks:
Client need connect the data translate before this function return .
When connect ready, need send the uuid (36Bit) string first in 3(default)* sencond.
Before send UUID string, start send each file info data.
data format:
{file length with 8bit}->{file hash with 32bit}->{file data}.

*1 > Client check time is define in config file :

```json
[Security]
ClientCkeckTime=3
```

This function is used the socket base on tcp/ip block mode. So if the receive course no done, the function will block. So should set the max size with different user. And in once time, no call too many receive function.
And then, the client after connect, must send the package according the order:

### CheckRecvStatus

When send the file done, call this function can check the recv status.
Params|Note
------|----
1  |the uuid when call ReceiveFile function.
2  |target file hash string.

Returns|Note
-------|----
1  |status code,0 > Wait;1 > Receiving;2 > Saving;3 > Done;4 > VerificationError;5 > OtherError
2  |save path if receive succeed. else is the error message.

### SetExtendData

### SetExtendDataByFile

### ConnectToDBCenter

### SQLQueryToDBCenter

### SQLInsertToDBCenter

### SQLUpdateToDBCenter

### SQLDeleteToDBCenter

### PrepareUpload

### UploadEnd

### GetThumbnail

### ConvertImageFormat

Send convert image format request to FileCenter.

Params|Type|Note
----|----|----
1|string|file index(uuid)
2|int|target format with protocol : FileCenter.SupportFormat
3|int|quality. 0-100(best)\
4|bool|retain old file when convert is over.
5|int|timeout

Return|Type|Note
----|----|-----
1|int|Result with Base.ResultType
2|string|error message(__Only error__). If succeed, see next table.

Succeed:
Return|Type|Note
----|----|-----
1|int|Result with Base.ResultType
2|table| new file info. key list:index, sha256, size, format.
3|table| __Only target is Best__.extend file info. key list: sha256, size, format.

### SetTimeout

### PushEvent
