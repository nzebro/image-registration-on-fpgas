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
* High-Level-Synthesis header file for Mutual Information computation
*
****************************************************************/
#ifndef MUTAL_INF_H
#define MUTAL_INF_H


#define MYROWS 512
#define MYCOLS 512
#define J_HISTO_ROWS 256
#define J_HISTO_COLS 256


#define ANOTHER_DIMENSION 256

#define TWO_FLOAT 2.0f
#define OUT_BUFF_SIZE 1

typedef float data_t;
typedef ap_uint<8> MY_PIXEL;


void mutual_information_master(MY_PIXEL * ref, MY_PIXEL * flt, data_t * mutual_info);
#endif
