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
* implementation of the sw part that collaborate with the hw IP
*
****************************************************************/
#include "mutual_information_hw.hpp"
data_t HwRegistration::s_mutual_information(cv::Mat ref, cv::Mat flt){

    void * s_hw_ref_buff;
    void * s_hw_flt_buff;
    DonutDriver * s_platform;
    const char * s_attachName = "iron_wrapper";
    s_platform = 0;
    s_platform = initPlatform();
    s_platform->attach(s_attachName);


//preparing iron
    size_t ref_bytes = DIM * DIM * sizeof(MY_PIXEL);

   s_hw_ref_buff = s_platform->allocAccelBuffer(ref_bytes);
   s_hw_flt_buff = s_platform->allocAccelBuffer(ref_bytes);

   s_platform->copyBufferHostToAccel(ref.data, s_hw_ref_buff, ref_bytes );
   s_platform->copyBufferHostToAccel(flt.data, s_hw_flt_buff, ref_bytes );

   s_platform->writeJamRegAddr(ADDR_REF,(uint64_t )s_hw_ref_buff);
   s_platform->writeJamRegAddr(ADDR_FLT, (uint64_t )s_hw_flt_buff);

//executing it
    int ap_ctrl  =  s_platform->readJamRegAddr(ADDR_AP_CTRL);
    s_platform->writeJamRegAddr(ADDR_AP_CTRL, ((ap_ctrl &0x80)| 0x01) );
    bool finish = false;
    while(!finish){
        ap_ctrl =  s_platform->readJamRegAddr(ADDR_AP_CTRL);
        finish = (ap_ctrl >> 1) & 0x1;
    }
    uint32_t result_u32 = s_platform->readJamRegAddr(ADDR_MI_DATA);
    uint32_t isvalid = s_platform->readJamRegAddr(ADDR_MI_CTRL);
    data_t mihlshost = 0.0;
    s_platform->deallocAccelBuffer(s_hw_ref_buff);
    s_platform->deallocAccelBuffer(s_hw_flt_buff);
    deinitPlatform(s_platform);
    s_platform = 0;
    if(isvalid)
        mihlshost = *((float*)((void*)(&result_u32)));
    return mihlshost;
}



data_t HwRegistration::mutual_information(cv::Mat ref, cv::Mat flt, bool hw_ex){

    data_t mihlshost;
    
    if(hw_ex){
        HwRegistration::prepareIron(ref.data, flt.data);
        HwRegistration::execAndWait();
        
        uint32_t result_u32 = this->platform->readJamRegAddr(ADDR_MI_DATA);
        uint32_t isvalid = this->platform->readJamRegAddr(ADDR_MI_CTRL);
        if(isvalid){
            mihlshost = *((float*)((void*)(&result_u32)));
            return mihlshost;
        }else
            return ERROR;
    }else{
        return sw_mi(ref.data, flt.data);
    }
}



void HwRegistration::execAndWait(){
   int ap_ctrl  = this->platform->readJamRegAddr(ADDR_AP_CTRL);
   this->platform->writeJamRegAddr(ADDR_AP_CTRL, ((ap_ctrl &0x80)| 0x01) );
   bool finish = false;
   while(!finish){
      ap_ctrl = this->platform->readJamRegAddr(ADDR_AP_CTRL);
      finish = (ap_ctrl >> 1) & 0x1;
   }
}

void HwRegistration::prepareIron(MY_PIXEL * ref, MY_PIXEL * flt){

	std::size_t ref_bytes = DIM * DIM * sizeof(MY_PIXEL);

   hw_ref_buff = this->platform->allocAccelBuffer(ref_bytes);
   hw_flt_buff = this->platform->allocAccelBuffer(ref_bytes);

   this->platform->copyBufferHostToAccel(ref, hw_ref_buff, ref_bytes );
   this->platform->copyBufferHostToAccel(flt, hw_flt_buff, ref_bytes );

   this->platform->writeJamRegAddr(ADDR_REF,(uint64_t )hw_ref_buff);
   this->platform->writeJamRegAddr(ADDR_FLT, (uint64_t )hw_flt_buff);
}

data_t HwRegistration::sw_mi(MY_PIXEL * ref, MY_PIXEL * flt){

    double j_h[J_HISTO_ROWS][J_HISTO_COLS];
    for(int i=0;i<J_HISTO_ROWS;i++){
        for(int j=0;j<J_HISTO_COLS;j++){
            j_h[i][j]=0.0;
        }
    }


    for(int i=0;i<DIM;i++){
        for(int j=0;j<DIM;j++){
            int a=ref[i *DIM + j];
            int b=flt[i *DIM + j];
            j_h[a][b]= (j_h[a][b])+1;
        }
    } 
    
    for (int i=0; i<J_HISTO_ROWS; i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
            j_h[j][i] = j_h[j][i]/(1.0*DIM*DIM);
        }
    }

    double entropy = 0.0;
    for (int i=0; i<J_HISTO_ROWS; i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
            double v = j_h[j][i];
            if (v > 0.000000000000001) {
                entropy += v*log(v)/log(2);
            }
        }
    }
    entropy *= -1;

    double href[ANOTHER_DIMENSION];
    for(int i=0;i<ANOTHER_DIMENSION;i++){
        href[i]=0.0;
    }

    for (int i=0; i<ANOTHER_DIMENSION; i++) {
        for (int j=0; j<ANOTHER_DIMENSION; j++) {
            href[i] += j_h[i][j];
        }
    }

    double hflt[ANOTHER_DIMENSION];
    for(int i=0;i<ANOTHER_DIMENSION;i++){
        hflt[i]=0.0;
    }

    for (int i=0; i<J_HISTO_ROWS; i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
            hflt[i] += j_h[j][i];
        }
    }

    double eref = 0.0;
    for (int i=0; i<ANOTHER_DIMENSION; i++) {
        if (href[i] > 0.000000000001) {
            eref += href[i] * log(href[i])/log(2);
        }
    }
    eref *= -1;


    double eflt = 0.0;
    for (int i=0; i<ANOTHER_DIMENSION; i++) {
        if (hflt[i] > 0.000000000001) {
            eflt += hflt[i] * log(hflt[i])/log(2);
        }
    }
    eflt =  eflt * (-1);
    double mutualinfo = eref + eflt - entropy;

    return mutualinfo;
}
