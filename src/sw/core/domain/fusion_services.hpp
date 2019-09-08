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
#ifndef DOMAIN_FUSION_HPP
#define DOMAIN_FUSION_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef HW_REG
#include "../fusion_algorithms.hpp"
#include "../register_algorithms.hpp"
#else
#include "fusion_algorithms.hpp"
#include "register_algorithms.hpp"
#endif
/**
 * @brief available_fusion_algorithms
 * @return available strategies for fusion
 */
std::vector<std::string> available_fusion_algorithms()
{
   return fusion_algorithms::available();
}

/**
 * @brief available_registration_algorithms
 * @return availabe strategies for registration
 */
std::vector<std::string> available_registration_algorithms()
{
   return register_algorithms::available();
}


/**
 * @brief fusion performs a fusion by registering the floating to the reference
 *        image and then perform the fusion
 * @param ref reference image
 * @param flt floating image
 * @param register_strategy method for registration
 * @param fusion_strategy method for fusion
 * @return fused image
 */
cv::Mat fuse_images(cv::Mat ref, cv::Mat flt,
           std::string register_strategy, std::string fusion_strategy)
{
   using namespace cv;
   Size origsize(512, 512);
   resize(ref, ref, origsize);
   resize(flt, flt, origsize);

   std::unique_ptr<fusion> fusion_algorithm =
         fusion_algorithms::pick(fusion_strategy);
   std::unique_ptr<registration> registration_algorithm =
         register_algorithms::pick(register_strategy);

   // register to align images
   Mat fin = registration_algorithm->register_images(ref, flt);

   // now do the fusion
   Mat fused = fusion_algorithm->fuse(ref, fin);

   return fused;
}



#endif // FUSION_HPP
