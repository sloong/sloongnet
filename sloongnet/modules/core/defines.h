/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-19 17:40:15
 * @Description: file content
 */
#ifndef SLOONGNET_DEFINES_H
#define SLOONGNET_DEFINES_H

// univ head file
#include "univ/defines.h"
#include "univ/univ.h"
#include "univ/univ.hpp"
#include "univ/log.h"
#include "univ/exception.h"
#include "univ/threadpool.h"
#include "univ/hash.h"
#include "univ/base64.h"
using namespace Sloong;
using namespace Sloong::Universal;

#include "protocol/core.pb.h"
using namespace Core;

template <typename T>
inline T TYPE_TRANS(LPVOID p)
{
	T tmp = static_cast<T>(p);
	assert(tmp);
	return tmp;
}

const int s_PriorityLevel = 3;

enum RecvStatus
{
	Wait = 0,
	Receiving = 1,
	Saveing = 2,
	Done = 3,
	VerificationError = 4,
	OtherError = 5,
};

// Receive file struce for GFunc
struct RecvDataPackage
{
	string strMD5 = "";
	RecvStatus emStatus = RecvStatus::Wait;
	string strName = "";
	string strPath = "";
};

enum HashType
{
	MD5 = 0,
	SHA_1 = 1,
	SHA_256 = 2,
	SHA_512 = 3,
};

template <class T>
inline shared_ptr<T> ConvertStrToObj(string obj)
{
	shared_ptr<T> item = make_shared<T>();
	if (!item->ParseFromString(obj))
		return nullptr;
	return item;
}

inline string ConvertObjToStr(::google::protobuf::Message *obj)
{
	string str_res;
	if (!obj->SerializeToString(&str_res))
		return "";
	return str_res;
}

inline bool ConvertStrToInt(string str, int *out_res, string* err_msg = nullptr)
{
	
	try
	{
		if (out_res)
			(*out_res) = stoi(str);
		return true;
	}
	catch (const invalid_argument &e)
	{
		if( err_msg )
			*err_msg = "invalid_argument";
		return false;
	}
	catch (const out_of_range &e)
	{
		if( err_msg )
			*err_msg = "out_of_range";
		return false;
	}
	catch (...)
	{
		if( err_msg )
			*err_msg = "unknown";
		return false;
	}
}

#endif