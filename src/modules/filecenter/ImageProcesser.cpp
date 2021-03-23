/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-08-17 11:16:27
 * @LastEditTime: 2021-03-23 20:08:39
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/filecenter/ImageProcesser.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#include "ImageProcesser.h"
#include <algorithm>

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
	string out_file = targetFile;
	switch ( fmt ){
		case FileCenter::SupportFormat::WEBP:{
			out_file += ".webp";
			str_cmd = Helper::Format("convert %s -quality %d -define webp:method=6 %s", sourceFile.c_str(), quality, out_file.c_str());
		}
		case FileCenter::SupportFormat::AVIF:{
			out_file += ".avif";
			str_cmd = Helper::Format("convert %s -quality %d %s", sourceFile.c_str(), quality, out_file.c_str());
		}
		case FileCenter::SupportFormat::HEIF:{
			out_file += ".heif";
			str_cmd = Helper::Format("convert %s -quality %d %s", sourceFile.c_str(), quality, out_file.c_str());
		}
		case FileCenter::SupportFormat::Best:{
			return ConvertBestFormat(sourceFile,targetFile,quality);
		}
		default:
		{
			return CResult::Make_Error("Unsupport format.");
		}
	}
	auto res = RunCMD(sourceFile, targetFile, str_cmd);
	if (res.IsFialed())
		return res;

	return CResult::Make_OK(out_file);
}

int GetFileSize(const string &source)
{
    ifstream in(source, ios::in | ios::binary);
    in.seekg(0, ios::end);
    int nSize = in.tellg();
    return nSize;
}



CResult ImageProcesser::ConvertBestFormat(const string &sourceFile, const string &targetFile, int quality)
{
	auto res_webp = ConvertFormat(sourceFile,targetFile,FileCenter::SupportFormat::WEBP,quality);
	if (res_webp.IsFialed())
		return res_webp;
	auto res_avif = ConvertFormat(sourceFile,targetFile,FileCenter::SupportFormat::AVIF,quality);
	if (res_avif.IsFialed())
		return res_avif;
	auto res_heif = ConvertFormat(sourceFile,targetFile,FileCenter::SupportFormat::HEIF,quality);
	if (res_heif.IsFialed())
		return res_heif;
	
	auto s_webp = GetFileSize(res_webp.GetMessage());
	auto s_avif = GetFileSize(res_avif.GetMessage());
	auto s_heif = GetFileSize(res_heif.GetMessage());

	if( s_webp > s_avif )
	{
		if( s_avif > s_heif ){
			// heif is best
			return res_heif;
		}
		else{
			// avif is best
			return res_avif;
		}
	}else{
		if( s_webp > s_heif ){
			// heif is best
			return res_heif;
		}
		else{
			// webp is best
			return res_webp;
		}
	}
}