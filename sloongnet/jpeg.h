#ifndef JPEG_H
#define JPEG_H

#include <jpeglib.h>
#include <string>
using std::string;

class CJPEG
{
public:
    CJPEG();
    void Read( string path );

    struct jpeg_decompress_struct cinfo;
};

#endif // JPEG_H
