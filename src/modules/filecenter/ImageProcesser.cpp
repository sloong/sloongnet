/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-08-17 11:16:27
 * @LastEditTime: 2020-08-17 11:19:29
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/filecenter/ImageProcesser.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#include "ImageProcesser.h"

using namespace Sloong;

CResult ImageProcesser::GetThumbnail(const string &sourceFile, const string &targetFile, int height, int width, int quality)
{
	if (sourceFile.empty() || targetFile.empty() || height == 0 || width == 0 || quality == 0)
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

	//string str_cmd = std::format("convert -sample {}x{} {} {}", width, height, src_path.c_str(), save_path.c_str());
	string str_cmd = Helper::Format("convert -sample %dx%d %s %s", width, height, sourceFile.c_str(), targetFile.c_str());

	if (CUniversal::RunSystemCmd(str_cmd))
	{
		return CResult::Succeed;
	}
	else
	{
		return CResult::Make_Error("run convert cmd fialed.");
	}
}

CResult ImageProcesser::ConvertFormat(const string &sourceFile, const string &targetFile, FileCenter::SupportFormat fmt)
{
	return CResult::Make_Error("No support now.");
	/*if (sourceFile.empty() || targetFile.empty())
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

	string str_cmd = Helper::Format("convert -resize %dx%d %s %s -quality %d", width, height, src_path.c_str(), save_path.c_str(), quality);
	if (CUniversal::RunSystemCmd(str_cmd))
	{
		return CResult::Succeed;
	}
	else
	{
		return CResult::Make_Error("run convert cmd fialed.");
	}*/
}
