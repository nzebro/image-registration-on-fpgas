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
* header and class def of the sw part that collaborate with the hw IP
*
****************************************************************/
#ifndef MUTUAL_INFORMATION_HW_HPP
#define MUTUAL_INFORMATION_HW_HPP
#include "platform.hpp"
#include <opencv2/core/core.hpp>

#define MYROWS 512
#define MYCOLS 512
#define J_HISTO_ROWS 256
#define J_HISTO_COLS 256

#define ANOTHER_DIMENSION 256

#define TWO_FLOAT 2
#define DIM 512

#define ADDR_AP_CTRL 0x0000
#define ADDR_REF 0x10
#define ADDR_FLT 0x18
#define ADDR_MI_DATA 0x20
#define ADDR_MI_CTRL 0x24

#define ERROR 0.00000000000

typedef float data_t;
typedef unsigned char MY_PIXEL;


class HwRegistration{

public:
    int matrix_dimension;
    int histogram_levels;

    HwRegistration (){
        platform = 0;
        platform = initPlatform();
        const char * attachName = "iron_wrapper";
        platform->attach(attachName);
        matrix_dimension = DIM;
        histogram_levels = J_HISTO_ROWS;
    };
    HwRegistration (int dim, int lvl){
        platform = 0;
        platform = initPlatform();
        const char * attachName = "iron_wrapper";
        platform->attach(attachName);
        matrix_dimension = dim;
        histogram_levels = lvl;
    };
    HwRegistration(int dim, int lvl, void * ref_buf, void * flt_buf, DonutDriver * plt){
        matrix_dimension = dim;
        const char * attachName = "iron_wrapper";
        histogram_levels = lvl;
        hw_ref_buff = ref_buf;
        hw_flt_buff = flt_buf;
        platform = plt;
        platform->attach(attachName);
    };
    ~HwRegistration(){
        platform->deallocAccelBuffer(hw_ref_buff);
        platform->deallocAccelBuffer(hw_flt_buff);
        deinitPlatform(platform);
        platform = 0;
    };

    data_t mutual_information(cv::Mat ref, cv::Mat flt, bool hw_ex);
    static data_t s_mutual_information(cv::Mat ref, cv::Mat flt);



protected:
    void * hw_ref_buff;
    void * hw_flt_buff;
    DonutDriver *platform;
    void execAndWait();
    void prepareIron(MY_PIXEL * ref, MY_PIXEL * flt);
    data_t sw_mi(MY_PIXEL * ref, MY_PIXEL * flt);

};

#endif //MUTUAL_INFORMATION_HW_HPP
