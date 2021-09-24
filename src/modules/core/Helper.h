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

#include <fmt/format.h>
using fmt::format;

#include <spdlog/spdlog.h>

#include <jsoncpp/json/json.h>


// univ head file
#include "univ.h"
using namespace Sloong;
using namespace Sloong::Universal;


#include "result.h"
#include "package.hpp" 

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

namespace Sloong
{
	typedef std::function<CResult(const string &, Package *)> FunctionHandler;

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

	inline string ConvertObjToStr(Package *obj)
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
			// save to temp value. because the out_res maybe nullptr, the user just want check the string can't to int.
			auto out = stoi(str);
			if (out_res)
				(*out_res) = out;
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
	inline bool IsOverflowPackage(Package *pack)
	{
		if (pack->ByteSizeLong() > g_max_package_size)
			return true;
		else
			return false;
	}

	inline constexpr int g_big_package_size = 1 * 1024;
	inline bool IsBigPackage(Package *pack)
	{
		if (pack->ByteSizeLong() > g_big_package_size)
			return true;
		else
			return false;
	}

	inline constexpr int g_max_package_print_log = 512;
	inline bool IsPrintLog(Package* pack)
	{
		if( !pack->extend().empty())
			return false;
		if( pack->ByteSizeLong() > g_max_package_print_log )
			return false;
		return true;
	}

	inline void PrintPackage( spdlog::logger* log, Package* package, const string& title, spdlog::level::level_enum level = spdlog::level::level_enum::trace )
	{
		if(IsPrintLog( package))
		{
			log->log( level, title +  package->ShortDebugString() );
		}
		else
		{
			log->log(  level, title + format("Function: %d Priority: %d ID: %llu Content[L]: %d Extend[L]: %d",
			 package->function(), package->priority(), package->id(), package->content().length(), package->extend().length()) );
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

	inline bool CheckJsonString( const Json::Value& j, const string& key ){ 
		return j.isMember(key) && j[key].isString();
	}
	inline bool CheckJsonInt( const Json::Value& j, const string& key, bool allowString = true ){
		if( j.isMember(key) )
		{
			auto& v = j[key];
			if( v.isInt() )
				return true;

			if(!allowString) return false;
			if(!v.isString() ) return false;

			return ConvertStrToInt(v.asString(), nullptr);
		}
		return false;
	}
	inline int GetJsonInt(const Json::Value& j, const string& key, bool allowString = true){
		if( j.isMember(key) )
		{
			auto& v = j[key];
			if( v.isInt() )
				return v.asInt();

			if(!allowString) throw invalid_argument("Json: value not int type.");
			if(!v.isString() ) throw invalid_argument("Json: value not int/string type.");
			int res = 0;
			if(!ConvertStrToInt(v.asString(), &res) )
				throw invalid_argument(format("Json: convert [{}] to int type failed.",v.asString()));
			return res;
		}
		throw invalid_argument("Json: not have the key.");
	}
} // namespace Sloong
