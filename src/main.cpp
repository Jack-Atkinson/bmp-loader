/*
  main.cpp

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

#include <cstdlib>
#include <unistd.h>
#include <exception>
#include <stdexcept>

#include "bmp.h"
#include "common.h"

bool BMPError(void) { throw std::runtime_error("Exiting..."); }

template<class Predicate>
struct Was : Predicate
{};

struct BMPSuccess {
  bool operator()(int value) const
  {
    bool succeeded = false;
    switch(value) {
      case BMP_OK:
        std::cout << "OK" << std::endl;
        succeeded = true;
        break;
      case BMP_ERR_OPENING_FILE:
        std::cerr << "FAIL\n    - Could not open the file" << std::endl;
        break;
      case BMP_ERR_CREATING_FILE:
        std::cerr << "FAIL\n    - Could not create file" << std::endl;
        break;
      case BMP_ERR_READING_FILE:
        std::cerr << "FAIL\n    - Could not read data from the file" << std::endl;
        break;
      case BMP_ERR_WRITING_FILE:
        std::cerr << "FAIL\n    - Could not write data to the file" << std::endl;
        break;
      case BMP_ERR_FILE_NOT_SUPPORTED:
        std::cerr << "FAIL\n    - The file is not in BMP format or is not 24 bits" << std::endl;
        break;
      case BMP_ERR_IMAGE_TOO_LARGE:
        std::cerr << "FAIL\n    - The image is too large for this program" << std::endl;
        break;
      case BMP_VALUE_NOT_SET:
        std::cerr << "FAIL\n    - Value not set!" << std::endl;
    }
    return succeeded;
  }
};

template<class Type, class Predicate>
bool operator>>(Type const& value, Was<Predicate> const& validator)
{
  return validator(value);
}

int CppMain(int argc, char *argv[])
{
  std::string input_image_file  = "input.bmp";
  std::string output_image_file = "output.bmp";
  std::string test_file = "test.bmp";

  BMPImage inputimage(input_image_file);

  inputimage.OpenImage()
    >> Was<BMPSuccess>()
    || BMPError();

  inputimage.ReadImageHeader()
    >> Was<BMPSuccess>()
    || BMPError();

  inputimage.ReadImagePixels()
    >> Was<BMPSuccess>()
    || BMPError();

  DWORD width  = inputimage.width();
  DWORD height = inputimage.height();
  DWORD size = inputimage.size();
  std::vector<BGR> pixels;

  inputimage.index(2, 2)[BLUE] = 255;
  inputimage.index(2, 2)[GREEN] = 255;
  inputimage.index(2, 2)[RED] = 255;
  
  for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++)
      inputimage.index(j, i) = BGR(255, 255, 255);

  pixels = inputimage.pixels();

  BMPImage outputimage(output_image_file);

  outputimage.set_width(width);
  outputimage.set_height(height);
  outputimage.set_pixels(pixels);

  outputimage.CreateImage()
    >> Was<BMPSuccess>()
    || BMPError();

  outputimage.WriteImageHeader()
    >> Was<BMPSuccess>()
    || BMPError();

  outputimage.WriteImagePixels()
    >> Was<BMPSuccess>()
    || BMPError();

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  try {
    CppMain(argc, argv);
  } catch(std::exception const& error) {
    std::cout << error.what() << std::endl;
    return EXIT_FAILURE;
  } catch (BMPErrors error) {
    switch(error) {
      case ERROR_OUT_OF_BOUNDS:
        std::cerr << "Error index value out of bounds" << std::endl;
        break;
      case ERROR_MYIMAGE_NOT_DEF:
        std::cerr << "Error my image is not defined" << std::endl;
        break;
      }
  } catch(...) {
    std::cout << "Something Went Wrong" << std::endl;
  }

  return EXIT_SUCCESS;
}
