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
* main of the whole application
* credits goes also to the author of this repo: https://github.com/mariusherzog/ImageRegistration
*
****************************************************************/

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <algorithm>
#include <functional>
#include <memory>
#include <iterator>
#include <chrono>
#include <numeric>

#ifdef HW_REG
#include "fusion_algorithms.hpp"
#include "register_algorithms.hpp"
#include "imagefusion.hpp"
#define HWUSAGE true
#else
#include "core/fusion_algorithms.hpp"
#include "core/register_algorithms.hpp"
#include "app/imagefusion.hpp"
#endif
using namespace cv;
using namespace std;
using namespace std::placeholders;


void getBackwardSplit(size_t size, char * string, char * dst){
 int i;
 int j;
 for (i = size; i >= 0; i--)
 {
  if(string[i] == '.')
   j = i;
 if(string[i] == '/')
   break;
}
strncpy(dst,  string + i + 1 ,  (size - j - 1));
dst[size - j - 1] = '\0';
}

void getFinalName(char * im1name, char * im2name, char * finalname){

 size_t im1size = strlen(im1name);
 size_t im2size = strlen(im2name);
 strcat(finalname,im1name);
 strcat(finalname, im2name);
 const char * format =".jpeg";
   #ifdef HW_REG
 const char * currplat = "-hw";

   #else
 const char * currplat = "-sw";

   #endif
 strcat(finalname, currplat);
 strcat(finalname, format);
}

int main(int argc, char ** argv)
{

  if(argc < 4){
    std::cout << "[WARNING] Iron needs 3 more arguments" <<std::endl<<std::endl;
    std::cout << "Example usage:"<<std::endl;
    std::cout << "\t <iron executable> <reference img> <floating img> <img result folder>" <<std::endl;
    return -1;
  }
  auto available_fusion_names = imagefusion::fusion_strategies();
  auto available_register_names = imagefusion::register_strategies();
  auto start_rep = std::chrono::system_clock::now();
  Mat fused = imagefusion::perform_fusion_from_files(argv[1], argv[2] , "mutualinformation", "alphablend");
  auto end_rep = std::chrono::system_clock::now();
  double nseconds_rep = std::chrono::duration_cast<std::chrono::nanoseconds>(end_rep - start_rep).count();

  size_t im1size = strlen(argv[1]);
  size_t im2size = strlen(argv[2]);

  char separator = '/';

  char im1name[25];
  char im2name[25];
  getBackwardSplit(im1size,argv[1], im1name);

  getBackwardSplit(im2size,argv[2], im2name);

  char tmp [50]="";
  getFinalName(im1name,im2name, tmp);
  char finalname [50]="";
  strncpy(finalname, argv[3], strlen(argv[3]));
  finalname[strlen(argv[3])]='\0';

  strncpy(finalname+strlen(finalname), tmp, strlen(tmp));
  finalname[strlen(finalname)]='\0';

  std::cout << finalname << std::endl;

  imwrite( finalname , fused );
 #ifndef HW_REG
    #ifdef SHOW
      cv::imshow("fused transformed", fused);
    #endif
  #endif

  cout << "Time executed " << std::fixed << nseconds_rep << std::scientific << "ns\n";

  waitKey(0);
}


