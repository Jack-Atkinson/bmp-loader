/*
  bmp.cpp

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

#include "bmp.h"

BMPImage::BMPImage(void)
{
  my_image_.reset();
}

BMPImage::BMPImage(std::string file_name)
{
  my_image_.reset(new MyImage(file_name, 1, 0, 0));
}

BMPImage::BMPImage(std::string file_name, DWORD width, DWORD height)
{
  my_image_.reset(new MyImage(file_name, 1, width, height));
}

BMPImage::~BMPImage(void)
{
  my_image_->working_file.close();
  delete my_image_->file_header;
  delete my_image_->info_header;
}

BMPImage::BMPImage(const BMPImage &bmpimage)
{
  *this = const_cast<BMPImage&>(bmpimage);
}

BMPImage::BMPImage(BMPImage &bmpimage)
{
  *this = bmpimage;
}

BMPImage &BMPImage::operator=(const BMPImage &bmpimage)
{
  return *this = const_cast<BMPImage&>(bmpimage);
}

BMPImage &BMPImage::operator=(BMPImage &bmpimage)
{
  if(&bmpimage != this) {
    my_image_.swap(bmpimage.my_image_);
    my_image_->ref_count++;
  }

  return *this;
}

int BMPImage::OpenImage(void)
{
  MyImageCheck();
  std::cout << "[+] Opening BMP file \"" << my_image_->file_name << "\"...";
  my_image_->working_file.open(my_image_->file_name.c_str(), std::fstream::in | std::fstream::binary);
  if (!my_image_->working_file.is_open())
    return BMP_ERR_OPENING_FILE;

  return BMP_OK;
}

int BMPImage::CreateImage(void)
{
  MyImageCheck();
  std::cout << "[+] Creating BMP file \"" << my_image_->file_name << "\"...";
  my_image_->working_file.open(my_image_->file_name.c_str(), std::fstream::out | std::fstream::binary);
  if (!my_image_->working_file.is_open())
    return BMP_ERR_CREATING_FILE;

  return BMP_OK;
}

int BMPImage::ReadImageHeader(void)
{
  MyImageCheck();

  std::cout << "[+] Reading image headers from \"" << my_image_->file_name << "\" into memory...";
  
  if (!my_image_->working_file.is_open())
    return BMP_ERR_READING_FILE;

  BYTE *image_data[2];

  my_image_->file_header = new BMPFileHeader();
  my_image_->info_header = new BMPInfoHeader();

  image_data[0] = new BYTE[sizeof(BMPFileHeader)];
  image_data[1] = new BYTE[sizeof(BMPInfoHeader)];

  my_image_->working_file.read(reinterpret_cast<char*>(image_data[0]),
                               sizeof(BMPFileHeader));
  my_image_->working_file.read(reinterpret_cast<char*>(image_data[1]),
                               sizeof(BMPInfoHeader));

  memcpy(my_image_->file_header, image_data[0], sizeof(BMPFileHeader));
  memcpy(my_image_->info_header, image_data[1], sizeof(BMPInfoHeader));


  if ((my_image_->file_header->signature != 0x4D42) ||
      (my_image_->info_header->bpp       != 24))
    return BMP_ERR_FILE_NOT_SUPPORTED;

  my_image_->height = my_image_->info_header->height;
  my_image_->width  = my_image_->info_header->width;

  delete[] image_data[0];
  delete[] image_data[1];

  return BMP_OK;
}

int BMPImage::ReadImagePixels(void)
{
  MyImageCheck();
  std::cout << "[+] Reading image pixels from \"" << my_image_->file_name << "\" into memory...";
  
  if (!my_image_->working_file.is_open())
    return BMP_ERR_READING_FILE;

  DWORD height    = my_image_->height;
  DWORD width     = my_image_->width;
  DWORD buffer    = width * height;
  BYTE pad_offset = ( ((width * 3 + 3) & (~3)) - (width * 3) ) * sizeof(BYTE);
  BGR temp;

  my_image_->working_file.seekg(my_image_->file_header->file_offset);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      my_image_->working_file.read(reinterpret_cast<char*>(&temp),
                                   sizeof(BGR));
      my_image_->pixels.push_back(temp);
    }
    my_image_->working_file.seekg(pad_offset, std::ios::cur);
  }

  return BMP_OK;
}

int BMPImage::WriteImageHeader(void)
{
  MyImageCheck();
  std::cout << "[+] Writing image headers to \"" << my_image_->file_name << "\"...";
  
  if (!my_image_->working_file.is_open())
    return BMP_ERR_WRITING_FILE;

  my_image_->file_header = new BMPFileHeader();
  my_image_->info_header = new BMPInfoHeader();

  DWORD height = my_image_->height;
  DWORD width  = my_image_->width;
  DWORD width_padded = (width * 3 + 3) & (~3);
  DWORD img_size = width_padded * height;

  BYTE fileheader_data[] = {"\x42\x4D"           //signature
                            "\x00\x00\x00\x00"   //file size
                            "\x00\x00\x00\x00"   //reserved
                            "\x36\x00\x00\x00"}; //file_offset
  BYTE infoheader_data[] = {"\x28\x00\x00\x00"   //header size
                            "\x00\x00\x00\x00"   //width
                            "\x00\x00\x00\x00"   //height
                            "\x01\x00"           //planes
                            "\x18\x00"           //bits per pixel
                            "\x00\x00\x00\x00"   //compression
                            "\x00\x00\x00\x00"   //imgSize
                            "\x12\x0b\x00\x00"   //X Resolution
                            "\x12\x0b\x00\x00"   //Y Resolution
                            "\x00\x00\x00\x00"   //colours
                            "\x00\x00\x00\x00"}; //important Colours

  memcpy(my_image_->file_header, fileheader_data, sizeof(BMPFileHeader));
  memcpy(my_image_->info_header, infoheader_data, sizeof(BMPInfoHeader));

  my_image_->info_header->width    = width;
  my_image_->info_header->height   = height;
  my_image_->info_header->img_size = img_size;

  my_image_->working_file.seekg(0);
  my_image_->working_file.write(reinterpret_cast<const char*>(my_image_->file_header),
                                sizeof(BMPFileHeader));
  my_image_->working_file.write(reinterpret_cast<const char*>(my_image_->info_header),
                                sizeof(BMPInfoHeader));

  return BMP_OK;
}

int BMPImage::WriteImagePixels(void)
{
  MyImageCheck();
  std::cout << "[+] Writing image pixels to \"" << my_image_->file_name << "\"...";
  
  if (!my_image_->working_file.is_open())
    return BMP_ERR_WRITING_FILE;

  DWORD height        = my_image_->height;
  DWORD width         = my_image_->width;
  BYTE padding_amount = ((width * 3 + 3) & (~3)) - (width * 3);
  BYTE *padding       = new BYTE[padding_amount];

  my_image_->working_file.seekg(my_image_->file_header->file_offset);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++)
      my_image_->working_file.write(reinterpret_cast<const char*>(&my_image_->pixels[(i * width) + j]),
                                    sizeof(BGR));
    my_image_->working_file.write(reinterpret_cast<const char*>(padding), padding_amount);
  }

  delete[] padding;

  return BMP_OK;
}

BGR& BMPImage::index(DWORD x, DWORD y)
{
  MyImageCheck();
  if ((x >= my_image_->width) ||
      (y >= my_image_->height))
    throw ERROR_OUT_OF_BOUNDS;
  
  return my_image_->pixels[(y * my_image_->height) + x];
}

DWORD BMPImage::size(void)
{
  MyImageCheck();
  return (my_image_->width * my_image_->height);
}

std::string BMPImage::file_name(void)
{
  MyImageCheck();
  return my_image_->file_name;
}

void BMPImage::set_file_name(std::string file_name)
{
  MyImageCheck();
  my_image_->file_name = file_name;
}

DWORD BMPImage::width(void)
{
  MyImageCheck();
  return my_image_->width;
}

DWORD BMPImage::height(void) 
{
  MyImageCheck();
  return my_image_->height;
}

std::vector<BGR> BMPImage::pixels(void)
{
  MyImageCheck();
  return my_image_->pixels;
}

void BMPImage::set_width(DWORD width)
{
  MyImageCheck();
  my_image_->width = width;
}


void BMPImage::set_height(DWORD height)
{
  MyImageCheck();
  my_image_->height = height;
}

void BMPImage::set_pixels(std::vector<BGR> pixels)
{
  MyImageCheck();
  my_image_->pixels = pixels;
}

std::shared_ptr<BMPImage::MyImage> BMPImage::MyImageData(BMPErrors error)
{
  if (my_image_ == NULL) throw error;

  return my_image_;
}

void BMPImage::MyImageCheck(void)
{
  if (my_image_ == NULL) throw ERROR_MYIMAGE_NOT_DEF;
}
