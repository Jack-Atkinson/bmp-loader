/*
	bmp.h

	Contributed by Jack Atkinson <jack.atkinson@outlook.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program, see the file COPYING. If not, see
	<http://www.gnu.org/licenses/>.
*/

#ifndef BMP_H_
#define BMP_H_

#include <vector>
#include <fstream>
#include <algorithm>
#include <memory>

#include "common.h"

enum BMPColors {
  RED,
  GREEN,
  BLUE
};

enum BMPReturns {
  BMP_OK = 0,
  BMP_ERR_OPENING_FILE = 1,
  BMP_ERR_CREATING_FILE = 2,
  BMP_ERR_READING_FILE = 3,
  BMP_ERR_WRITING_FILE = 4,
  BMP_ERR_FILE_NOT_SUPPORTED = 5,
  BMP_ERR_IMAGE_TOO_LARGE = 6,
  BMP_VALUE_NOT_SET = 7
};

enum BMPErrors {
  ERROR_OUT_OF_BOUNDS,
  ERROR_MYIMAGE_NOT_DEF
};

struct BMPFileHeader {
  WORD  signature;
  DWORD file_size;
  DWORD reserved;
  DWORD file_offset;
} __attribute__((packed));

struct BMPInfoHeader {
  DWORD header_size;
  DWORD width;
  DWORD height;
  WORD  planes;
  WORD  bpp;
  DWORD compression;
  DWORD img_size;
  DWORD x_resolution, y_resolution;
  DWORD colours;
  DWORD important_colours;
} __attribute__((packed));

struct BGR {
  BYTE& operator[](const BMPColors& color)
  {
    switch (color) {
      case BLUE:  return b;
      case GREEN: return g;
      case RED:   return r;
    }
    throw ERROR_OUT_OF_BOUNDS;
  }

  DWORD operator()() //return rgb
  {
    return (r << 16) | (g << 8) | b;
  }

  BGR()
  : b(0), g(0), r(0)
  {}
  BGR(const BYTE* BGR)
  : b(BGR[0]), g(BGR[1]), r(BGR[2])
  {}
  BGR(BYTE blue, BYTE green, BYTE red)
  : b(blue), g(green), r(red)
  {}
  BYTE b, g, r;
};

class BMPImage {
 public:
  BMPImage(void);
  BMPImage(std::string);
  BMPImage(std::string, DWORD, DWORD);

  ~BMPImage(void);

  /* Copy Constructors */
  BMPImage(const BMPImage&);
  BMPImage(BMPImage&);

  /* Assignment Operators */
  BMPImage& operator=(const BMPImage&);
  BMPImage& operator=(BMPImage&);

  int OpenImage(void);
  int CreateImage(void);
  int ReadImageHeader(void);
  int ReadImagePixels(void);
  int WriteImageHeader(void);
  int WriteImagePixels(void);

  BGR& index(DWORD, DWORD);
  DWORD size(void);

  /* Getters and Setters */
  std::string file_name(void);
  DWORD width(void);
  DWORD height(void);
  std::vector<BGR> pixels(void);
  void set_file_name(std::string);
  void set_width(DWORD);
  void set_height(DWORD);
  void set_pixels(std::vector<BGR>);

 private:
  struct MyImage {
    std::string       file_name;
    std::fstream      working_file;
    BMPFileHeader     *file_header;
    BMPInfoHeader     *info_header;
    BYTE              ref_count;
    DWORD             width;
    DWORD             height;
    std::vector<BGR>  pixels;
    MyImage(WORD ref_count, WORD width, WORD height)
    : ref_count(ref_count), width(width), height(height)
    {}
    MyImage(std::string file_name, WORD ref_count, WORD width, WORD height)
    : file_name(file_name), ref_count(ref_count), width(width), height(height)
    {}
  };
  std::shared_ptr<MyImage> my_image_;
  std::shared_ptr<MyImage> MyImageData(BMPErrors);
  void MyImageCheck(void);
};

#endif
