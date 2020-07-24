/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-19 17:40:15
 * @Description: file content
 */
#pragma once

#include <sys/time.h>

// univ head file
#include "univ/defines.h"
#include "univ/univ.h"
#include "univ/univ.hpp"
#include "univ/log.h"
#include "univ/threadpool.h"
#include "univ/hash.h"
#include "univ/base64.h"
using namespace Sloong;
using namespace Sloong::Universal;

#include "result.hpp"

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

namespace Sloong
{
	typedef std::function<CResult(const string &, DataPackage *)> FunctionHandler;

	inline timeval GetTimeval()
	{
		struct timeval cur;
		gettimeofday(&cur, NULL);
		return cur;
	}

	inline double GetClock()
	{
		auto cur = GetTimeval();
		return cur.tv_sec * 1000 + cur.tv_usec / 1000.0;
	}

	inline string FormatRecord(DataPackage *pack)
	{
		string str;
		auto clocks = pack->clocks();
		auto start = clocks.begin();
		for (auto item = start+1 ; item != clocks.end(); item++)
		{
			str = Helper::Format("%s[%.2f]", str.c_str(), *item - *start);
		}
		return str;
	}

	template <typename T, typename K>
	inline T STATIC_TRANS(K p)
	{
		T tmp = static_cast<T>(p);
		assert(tmp);
		return tmp;
	}

	template <typename T, typename K>
	inline auto DYNAMIC_TRANS(K p)
	{
		auto tmp = dynamic_cast<T>(p);
		assert(tmp);
		return tmp;
	}

	const int s_PriorityLevel = 3;

	template <class T>
	inline shared_ptr<T> ConvertStrToObj(const string &obj)
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

	inline bool ConvertStrToInt(const string &str, int *out_res, string *err_msg = nullptr)
	{
		try
		{
			if (out_res)
				(*out_res) = stoi(str);
			return true;
		}
		catch (const invalid_argument &e)
		{
			if (err_msg)
				*err_msg = "invalid_argument";
			return false;
		}
		catch (const out_of_range &e)
		{
			if (err_msg)
				*err_msg = "out_of_range";
			return false;
		}
		catch (...)
		{
			if (err_msg)
				*err_msg = "unknown";
			return false;
		}
	}

	inline string &FormatFolderString(string &str)
	{
		char tag = str.at(str.length() - 1);
		if (tag != '/' && tag != '\\')
		{
			str.append("/");
		}
		return str;
	}
} // namespace Sloong
