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
* High-Level-Synthesis implementation file for Mutual Information computation
*
****************************************************************/
#include "hls_math.h"
#include <ap_int.h>
#include "mutual_info.hpp"
#include <stdio.h>
#include <string.h>
void mutual_information_master( MY_PIXEL * ref, MY_PIXEL * flt,  data_t * mutual_info){
#pragma HLS INTERFACE m_axi port=ref depth=512 offset=slave
#pragma HLS INTERFACE m_axi port=flt depth=512 offset=slave

#pragma HLS INTERFACE s_axilite port=return bundle=AXI_Lite_1
#pragma HLS INTERFACE s_axilite port=ref bundle=AXI_Lite_1
#pragma HLS INTERFACE s_axilite port=flt bundle=AXI_Lite_1
#pragma HLS INTERFACE s_axilite port=mutual_info register bundle=AXI_Lite_1



    data_t joint_histogram[J_HISTO_ROWS * J_HISTO_COLS];
    MY_PIXEL ref_int [MYROWS*MYCOLS];
    MY_PIXEL flt_int [MYROWS*MYCOLS];

    data_t hist_ref[ANOTHER_DIMENSION];
    data_t hist_flt[ANOTHER_DIMENSION];
    data_t mutual_information[OUT_BUFF_SIZE];

    H_REF_FLT_INIT: for (int i = 0; i < ANOTHER_DIMENSION; i++)
    {
#pragma HLS PIPELINE
        hist_ref[i] = 0.0;
        hist_flt[i] = 0.0;
    }

    MI_INIT: for (int i = 0; i < OUT_BUFF_SIZE; i++)
    {
#pragma HLS PIPELINE
        mutual_information[i] = 0.0;
    }
    
    H_JOINT_INIT: for (int i=0; i<J_HISTO_ROWS;i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
#pragma HLS PIPELINE
            joint_histogram[ i * J_HISTO_ROWS + j ] = 0.0;
        }
    }

    memcpy(ref_int,(const MY_PIXEL*) ref,sizeof(MY_PIXEL)*MYROWS*MYCOLS);
    memcpy(flt_int,(const MY_PIXEL*) flt,sizeof(MY_PIXEL)*MYROWS*MYCOLS);

    H_JOINT_COMP: for (int i=0; i<MYCOLS; i++) {
        for (int j=0; j<MYROWS; j++) {
#pragma HLS PIPELINE
            MY_PIXEL ref_intensity = ref_int[i * MYCOLS + j];
            MY_PIXEL flt_intensity = flt_int[i * MYCOLS + j];
            joint_histogram[ref_intensity * J_HISTO_ROWS + flt_intensity] = joint_histogram[ref_intensity * J_HISTO_ROWS + flt_intensity] + 1;
        }
    }

    H_JNT_PROB: for (int i=0; i<J_HISTO_ROWS;i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
#pragma HLS PIPELINE
            joint_histogram[ j * J_HISTO_ROWS + i ] = joint_histogram[j * J_HISTO_ROWS + i]/(1.0*MYROWS*MYCOLS);
        }
    }


    data_t entropy = 0.0;
    ENTRPY: for (int i=0; i<J_HISTO_ROWS; i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
#pragma HLS PIPELINE
            data_t v = joint_histogram[j * J_HISTO_ROWS + i];
            if (v > 0.000000000000001) {
                entropy += v*hls::log(v)/hls::log(TWO_FLOAT);
            }
        }
    }
    entropy *= -1;


    H_REF_COMP: for (int i=0; i<J_HISTO_ROWS;i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
#pragma HLS PIPELINE
            hist_ref[i] += joint_histogram[i * J_HISTO_COLS + j];
        }
    }


    H_FLT_COMP: for (int i=0; i<J_HISTO_COLS; i++) {
        for (int j=0; j< J_HISTO_ROWS; j++) {
#pragma HLS PIPELINE
            hist_flt[i] += joint_histogram[j * J_HISTO_ROWS + i ];
        }
    }




    data_t entropy_ref = 0.0;
    ENTRPY_REF: for (int i=0; i<ANOTHER_DIMENSION; i++) {
#pragma HLS PIPELINE
        if (hist_ref[i] > 0.000000000001) {
            entropy_ref += hist_ref[i] * hls::log(hist_ref[i])/hls::log(TWO_FLOAT);
        }
    }
    entropy_ref *= -1;


    data_t entropy_flt = 0.0;
    ENTRPY_FLT: for (int i=0; i<ANOTHER_DIMENSION; i++) {
#pragma HLS PIPELINE
        if (hist_flt[i] > 0.000000000001) {
            entropy_flt += hist_flt[i] * hls::log(hist_flt[i])/hls::log(TWO_FLOAT);
        }
    }
    entropy_flt *= -1;

    mutual_information[0]= entropy_ref + entropy_flt - entropy;

    *mutual_info = *mutual_information;



    return ;
}
