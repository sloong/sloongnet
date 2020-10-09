/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2017-11-28 20:03:08
 * @LastEditTime: 2020-08-11 19:40:10
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/defines.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description:
 */
/***
  * @......................................&&.........................
  * @....................................&&&..........................
  * @.................................&&&&............................
  * @...............................&&&&..............................
  * @.............................&&&&&&..............................
  * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
  * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
  * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
  * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
  * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
  * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
  * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
  * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
  * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
  * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
  * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
  * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
  * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
  * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
  * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
  * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
  * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
  * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
  * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
  * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
  * @.....&&&&&&&&&&&&&&&&............................&&..............
  * @....&&&&&&&&&&&&&&&.................&&...........................
  * @...&&&&&&&&&&&&&&&.....................&&&&......................
  * @...&&&&&&&&&&.&&&........................&&&&&...................
  * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
  * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
  * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
  * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
  * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
  * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
  * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
  * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
  * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
  * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
  * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
  * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
  * @........&&...................&&&&&&.........................&&&..
  * @.........&.....................&&&&........................&&....
  * @...............................&&&.......................&&......
  * @................................&&......................&&.......
  * @.................................&&..............................
  * @..................................&..............................
  */

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
#include "defines.h"
#include "univ.h"
#include "univ.hpp"
#include "log.h"
#include "threadpool.h"
#include "hash.h"
#include "base64.h"
#include "CRC.hpp"
using namespace Sloong;
using namespace Sloong::Universal;

#include "result.h"

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
		auto clocks = pack->reserved().clocks();
		auto start = clocks.begin();
		for (auto item = start + 1; item != clocks.end(); item++)
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

	inline bool ConvertStrToInt64(const string &str, int64_t *out_res, string *err_msg = nullptr)
	{
		try
		{
			if (out_res)
				(*out_res) = stold(str);
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

	inline void FormatFolderString(string &str)
	{
		char tag = str.at(str.length() - 1);
		if (tag != '/' && tag != '\\')
		{
#ifndef _WINDOWS
			str.append("/");
#else
			str.append("\\");
#endif
		}
	}

	inline bool FileExist(const string &path)
	{
		if (-1 == access(path.c_str(), R_OK))
		{
			return false;
		}
		return true;
	}

	inline int ReadFile(const string &file, string &out_data)
	{
		ifstream in(file.c_str(), ios::in | ios::binary);
		streampos pos = in.tellg();
		in.seekg(0, ios::end);
		int nSize = in.tellg();
		in.seekg(pos);
		out_data.clear();
		out_data.resize(nSize);
		in.read(out_data.data(), nSize);
		in.close();
		return nSize;
	}

	inline constexpr int g_max_package_size = 5 * 1024 * 1024;
	inline bool IsOverflowPackage(DataPackage *pack)
	{
		if (pack->ByteSize() > g_max_package_size)
			return true;
		else
			return false;
	}

	inline constexpr int g_big_package_size = 1 * 1024;
	inline bool IsBigPackage(DataPackage *pack)
	{
		if (pack->ByteSize() > g_big_package_size)
			return true;
		else
			return false;
	}

	inline constexpr int g_max_package_print_log = 512;
	inline bool IsPrintLog(DataPackage* pack)
	{
		if( !pack->extend().data().empty())
			return false;
		if( pack->ByteSize() > g_max_package_print_log )
			return false;
		return true;
	}

	inline void PrintPackage( CLog* log, DataPackage* package, const string& title, LOGLEVEL level = LOGLEVEL::Verbos )
	{
		if(IsPrintLog( package))
		{
			log->Log( title +  package->ShortDebugString(), level );
		}
		else
		{
			log->Log( title + Helper::Format("Function: %d Priority: %d ID: %lld Content[L]: %d Extend[L]: %d",
			 package->function(), package->priority(), package->id(), package->content().data().length(), package->extend().data().length()), level );
		}
	}

	inline string ConvertToHexString( const char* d, int start, int end )
	{
		string s;
		s.resize((end-start+1)*2);
		char* p = s.data();
		int i = 0;
		for ( i=start; i<end; i++) {
			sprintf(p+i*2, "%02x", d[i]);
		}
		p[i*2] = 0;
		return s;
	}

	inline uint32_t CRCEncode32( const string& s )
	{
		return CRC::Calculate( s.c_str(), s.size(), CRC::CRC_32());
	}
} // namespace Sloong
