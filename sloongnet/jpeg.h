#ifndef JPEG_H
#define JPEG_H

typedef unsigned char UCHAR;

#include <string>
using std::string;
namespace Sloong
{
	struct jpeg_decompress_struc;
	class CJPEG
	{
	public:
		CJPEG();
		virtual ~CJPEG();
		bool Load(string path);
		bool Save(int quality, int width = 0, int height = 0, string path = "");

		int GetHeight();
		int GetWidth();
		int GetBPP();

	protected:
		string m_strPath;
		int m_nWidth;
		int m_nHeight;
		int m_nBPP;
		UCHAR*	m_pData;
	};
}

#endif // JPEG_H
