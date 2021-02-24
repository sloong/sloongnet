/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-08-17 11:16:27
 * @LastEditTime: 2021-02-24 11:27:54
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/filecenter/ImageProcesser.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#include "ImageProcesser.h"

using namespace Sloong;


CResult RunCMD( const string &sourceFile, const string &targetFile, const string &str_cmd){
	if (sourceFile.empty() || targetFile.empty())
	{
		return CResult::Make_Error("Param error.");
	}

	if (access(sourceFile.c_str(), ACC_E) == -1)
	{
		return CResult::Make_Error("Cannot read the file.");
	}

	if (access(targetFile.c_str(), ACC_W) == 0)
	{
		return CResult::Make_Error("no access.");
	}
	CUniversal::CheckFileDirectory(targetFile);

	if (CUniversal::RunSystemCmd(str_cmd))
	{
		return CResult::Succeed;
	}
	else
	{
		return CResult::Make_Error("run cmd fialed.");
	}
}


CResult ImageProcesser::GetThumbnail(const string &sourceFile, const string &targetFile,  int width, int height, int quality)
{
	if (height == 0 || width == 0 || quality == 0)
	{
		return CResult::Make_Error("Param error.");
	}

	string str_cmd = Helper::Format("convert -sample %dx%d %s %s", width, height, sourceFile.c_str(), targetFile.c_str());
	return RunCMD(sourceFile, targetFile, str_cmd);
}

CResult ImageProcesser::ConvertFormat(const string &sourceFile, const string &targetFile, FileCenter::SupportFormat fmt, int quality)
{
	string str_cmd = "";
	switch ( fmt ){
		case FileCenter::SupportFormat::WEBP:{
			str_cmd = Helper::Format("convert %s -quality %d -define webp:method=6 %s.webp", sourceFile.c_str(), quality, targetFile.c_str());
		}
		default:
		{
			return CResult::Make_Error("Unsupport format.");
		}
	}
	auto res = RunCMD(sourceFile, targetFile, str_cmd);
	if (res.IsFialed())
		return res;

	return CResult::Succeed;
}
