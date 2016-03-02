#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>
#include "jpeg.h"
#include <univ/defines.h>
using namespace std;
using namespace Sloong;

CJPEG::CJPEG()
{
	m_pData = NULL;
}

Sloong::CJPEG::~CJPEG()
{
	SAFE_DELETE_ARR(m_pData);
}


bool CJPEG::Load(string path)
{
	//read
	struct jpeg_decompress_struct cinfo_decompress;
	FILE* infile;
	int row_stride;
	struct jpeg_error_mgr jerr;

	if ((infile = fopen(path.c_str(), "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", path.c_str());
		return -1;
	}

	cinfo_decompress.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo_decompress);
	jpeg_stdio_src(&cinfo_decompress, infile);
	int ret = jpeg_read_header(&cinfo_decompress, TRUE);
	if (ret != JPEG_HEADER_OK) return -1;
	jpeg_start_decompress(&cinfo_decompress);
	row_stride = cinfo_decompress.output_width * cinfo_decompress.output_components;
	int buffer_height = 1;
	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * buffer_height);
	buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_stride);
	//JSAMPARRAY buffer = (*cinfo_decompress.mem->alloc_sarray)((j_common_ptr)&cinfo_decompress, JPOOL_IMAGE, row_stride, 1);
	m_nWidth = cinfo_decompress.output_width;
	m_nHeight = cinfo_decompress.output_height;
	m_nBPP = cinfo_decompress.output_components;

	m_pData = new byte[cinfo_decompress.output_width * cinfo_decompress.output_height * cinfo_decompress.output_components];
	long counter = 0;

	while (cinfo_decompress.output_scanline < cinfo_decompress.output_height) {
		jpeg_read_scanlines(&cinfo_decompress, buffer, 1);
		memcpy(m_pData + counter, buffer[0], row_stride);
		counter += row_stride;
	}

	jpeg_finish_decompress(&cinfo_decompress);
	jpeg_destroy_decompress(&cinfo_decompress);

	fclose(infile);
	return true;
}


bool CJPEG::Save(int quality, int width /*= 0*/, int height /* =0 */, string strImageName /* = "" */ )
{
	if ( strImageName == "" )
	{
		strImageName = m_strPath;
	}
	if (width == 0)
	{
		width = GetWidth();
	}
	if ( height ==0)
	{
		height = GetHeight();
	}
	
	//write
	FILE * outfile;
	JSAMPROW row_pointer[1];
	struct jpeg_error_mgr jerr;
	int row_stride_dst;
	struct jpeg_compress_struct cinfo_compress;
	cinfo_compress.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo_compress);

	if ((outfile = fopen(strImageName.c_str(), "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", strImageName.c_str());
		//exit(1);
		return -1;
	}

	jpeg_stdio_dest(&cinfo_compress, outfile);

	cinfo_compress.image_width = width;
	cinfo_compress.image_height = height;
	cinfo_compress.input_components = 3;
	cinfo_compress.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo_compress);
	jpeg_set_quality(&cinfo_compress, quality, TRUE);
	jpeg_start_compress(&cinfo_compress, TRUE);

	row_stride_dst = width * 3;

	//image_buffer = new unsigned char[row_stride_dst * cinfo_compress.image_height];
	//memcpy(image_buffer, imageData->pixels, row_stride_dst * cinfo_compress.image_height);

	while (cinfo_compress.next_scanline < cinfo_compress.image_height) {
		//row_pointer[0] = &image_buffer[cinfo_compress.next_scanline * row_stride_dst];
		row_pointer[0] = &m_pData[cinfo_compress.next_scanline * row_stride_dst];
		jpeg_write_scanlines(&cinfo_compress, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo_compress);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo_compress);

	return 0;
}

int Sloong::CJPEG::GetHeight()
{
	return m_nHeight;
}

int Sloong::CJPEG::GetWidth()
{
	return m_nWidth;
}

int Sloong::CJPEG::GetBPP()
{
	return m_nBPP;
}
