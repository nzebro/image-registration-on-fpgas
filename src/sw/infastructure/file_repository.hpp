/******************************************
*MIT License
*
*Copyright (c) [2019] [iron-team]
*
*Permission is hereby granted, free of charge, to any person obtaining a copy
*of this software and associated documentation files (the "Software"), to deal
*in the Software without restriction, including without limitation the rights
*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*copies of the Software, and to permit persons to whom the Software is
*furnished to do so, subject to the following conditions:
*
*The above copyright notice and this permission notice shall be included in all
*copies or substantial portions of the Software.
*
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*SOFTWARE.
*/
/***************************************************************
*
* credits goes to the author of this repo: https://github.com/mariusherzog/ImageRegistration
*
****************************************************************/

#ifndef IMAGE_FROM_FILE_HPP
#define IMAGE_FROM_FILE_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include <string>
#ifndef HW_REG
#include "../interfaces/image_repository.hpp"
#else
#include "image_repository.hpp"
#endif
/**
 * @brief The file_repository class implements the repository by loading the
 *        image from files.
 */
class file_repository : public Iimage_repository
{
   private:
      std::string path_reference;
      std::string path_floating;

   public:
      file_repository(std::string path_reference, std::string path_floating) :
         path_reference {path_reference},
         path_floating {path_floating}
      {
      }

      cv::Mat reference_image() override
      {
         return cv::imread(path_reference, cv::IMREAD_GRAYSCALE);
      }

      cv::Mat floating_image() override
      {
         return cv::imread(path_floating, cv::IMREAD_GRAYSCALE);
      }
};

#endif // IMAGE_FROM_FILE_HPP
