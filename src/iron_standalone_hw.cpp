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
* C++ testbench for real hardware usage
*
****************************************************************/
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <random>
#include <stdio.h>
#include <chrono>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <string>
#include <cstring>


#include "platform.hpp"

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
#define REPETITIONS 30

typedef float data_t;
typedef unsigned char MY_PIXEL;

std::ofstream logger;
int rep = 0;
char test [20]="";
// perform a test with random images
void random_test();
//given two images and the pointer to the result compute the MI by IRON IP
void hw_img_test(MY_PIXEL * ref, MY_PIXEL * flt, data_t * mihlshost);
// execute IRON and wait
void execAndWait();
//prepare the buffers and set the IRON IP
void prepareIron(MY_PIXEL * ref, MY_PIXEL * flt);
// given two images and the pointer to the result compute the MI by software
double sw_mi(MY_PIXEL * ref, MY_PIXEL * flt);
// execute a single test and display results and time
void execTest(MY_PIXEL * ref, MY_PIXEL * flt);
// execute the test in order to get avg execution times
void execTestMultiple(MY_PIXEL * ref, MY_PIXEL * flt);
//
void logging(double time, char * plt, double mi);



DonutDriver *platform = 0;
void * hw_ref_buff;
void * hw_flt_buff;

void logging(double time, char * plt, double mi){
   logger << std::fixed << test << std::scientific << "\t" << time << "\t" << plt << "\t"  << rep << "\t" << mi << std::endl;

}

void hw_img_test(MY_PIXEL * ref, MY_PIXEL * flt, data_t * mihlshost){
   prepareIron(ref,flt);
   execAndWait();
   uint32_t result_u32 = platform->readJamRegAddr(ADDR_MI_DATA);
   uint32_t isvalid = platform->readJamRegAddr(ADDR_MI_CTRL);
   *mihlshost = *((float*)((void*)(&result_u32)));
}

void execAndWait(){
   int ap_ctrl  = platform->readJamRegAddr(ADDR_AP_CTRL);
   platform->writeJamRegAddr(ADDR_AP_CTRL, ((ap_ctrl &0x80)| 0x01) );
   bool finish = false;
   while(!finish){
      ap_ctrl = platform->readJamRegAddr(ADDR_AP_CTRL);
      finish = (ap_ctrl >> 1) & 0x1;
   }
}

void prepareIron(MY_PIXEL * ref, MY_PIXEL * flt){

   size_t ref_bytes = DIM * DIM * sizeof(MY_PIXEL);

   hw_ref_buff = platform->allocAccelBuffer(ref_bytes);
   hw_flt_buff = platform->allocAccelBuffer(ref_bytes);

   platform->copyBufferHostToAccel(ref, hw_ref_buff, ref_bytes );
   platform->copyBufferHostToAccel(flt, hw_flt_buff, ref_bytes );

   platform->writeJamRegAddr(ADDR_REF,(uint64_t )hw_ref_buff);
   platform->writeJamRegAddr(ADDR_FLT, (uint64_t )hw_flt_buff);
}


double sw_mi(MY_PIXEL * ref, MY_PIXEL * flt){

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



void execTest(MY_PIXEL * ref, MY_PIXEL * flt){
  auto start_sw = std::chrono::system_clock::now();
  double mutualinfo = sw_mi(ref,flt);
   auto end_sw = std::chrono::system_clock::now();
   double nseconds_sw = std::chrono::duration_cast<std::chrono::nanoseconds>(end_sw - start_sw).count();
   logging(nseconds_sw, "sw", mutualinfo);
   printf("%f\n",mutualinfo);
   data_t mihlshost;
   auto start_hw = std::chrono::system_clock::now();
   hw_img_test(ref,flt,&mihlshost);
   auto end_hw = std::chrono::system_clock::now();
   double nseconds_hw = std::chrono::duration_cast<std::chrono::nanoseconds>(end_hw - start_hw).count(); 
   logging(nseconds_hw, "hw", mihlshost);
   printf("%f\n", mihlshost);
   std::cout << "mutual information hw " << mihlshost << " mutual information sw " << mutualinfo << std::endl;

   std::cout << "time hw " << nseconds_hw << "[ns] time sw " << nseconds_sw << " [ns] " <<std::endl;
}

void execTestMultiple(MY_PIXEL * ref, MY_PIXEL * flt){
  double tot_time_sw = 0.0;
  double tot_time_hw = 0.0;
  double sw_avg = 0.0;
  double hw_avg = 0.0;
  for (int i = 0; i < REPETITIONS;i++)
  {
    auto start_sw = std::chrono::system_clock::now();
    double mutualinfo = sw_mi(ref,flt);
    auto end_sw = std::chrono::system_clock::now();
    double nseconds_sw = std::chrono::duration_cast<std::chrono::nanoseconds>(end_sw - start_sw).count();
   logging(nseconds_sw, "sw", mutualinfo);
    rep++;
    tot_time_sw += nseconds_sw;
  }
  rep=0;
  sw_avg = tot_time_sw / REPETITIONS;

    for (int i = 0; i < REPETITIONS;i++)
  {
   data_t mihlshost;
   auto start_hw = std::chrono::system_clock::now();
   hw_img_test(ref,flt,&mihlshost);
   auto end_hw = std::chrono::system_clock::now();
   double nseconds_hw = std::chrono::duration_cast<std::chrono::nanoseconds>(end_hw - start_hw).count();
   logging(nseconds_hw, "hw", mihlshost);
   rep++;
   tot_time_hw += nseconds_hw;
  }
  rep=0;
  hw_avg = tot_time_hw / REPETITIONS;

   std::cout << "Average time hw " << hw_avg << " [ns]  Average time sw " << sw_avg << " [ns] " <<std::endl;
}


void random_test(){
   std::cout << "************************" << std::endl;
   std::cout  << std::endl;
   std::cout << "Random input images test" << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;

   MY_PIXEL ref[DIM * DIM];
   MY_PIXEL flt[DIM * DIM];

   int myseed = 1234;

   std::default_random_engine rng(myseed);
   std::uniform_int_distribution<int> rng_dist(0, 255);

   for(int i=0;i<DIM;i++){
      for(int j=0;j<DIM;j++){
         ref[i *DIM + j]=static_cast<unsigned char>(rng_dist(rng));
      }
   }

   for(int i=0;i<DIM;i++){
      for(int j=0;j<DIM;j++){
         flt[i *DIM + j]=static_cast<unsigned char>(rng_dist(rng));
      }
  }
   std::string s = "Rand";
   strcpy(test, s.c_str()); 
   execTest(ref, flt);
    #ifdef AVERAGE_REPS
      execTestMultiple(ref, flt);
    #endif
}




int main(int argc, char * argv[]){


   const char * attachName = "iron_wrapper";
   platform = initPlatform();
   platform->attach(attachName);
   logger.open ("benchmark.csv");
   logger <<"Test\tTime[ns]\tHW/SW";
    #ifdef AVERAGE_REPS
      logger << "\tRepetion#";
    #endif
    logger << std::endl;
   
   random_test();
   std::cout  << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;
   std::cout  << std::endl;
   std::cout << "End of Random input images test" << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;
   if( ((argc+1) %2 )!= 0){
      std::cout<< "[WARNING] no more tests or passing a number of inputs not correct, expecting 2*k images" << std::endl;
   }else{
    int j =0;
      for(int i=1; i<argc;i=i+2){
         std::cout << "************************" << std::endl;
         std::cout  << std::endl;
         std::cout << "Test number "<<j+1 << std::endl;
         std::cout  << std::endl;
         std::cout << "************************" << std::endl;
         snprintf(test, 20, "%d",j+1);
         cv::Size origsize(512, 512);
         cv::Mat ref = cv::imread(argv[i], cv::IMREAD_GRAYSCALE);
         cv::resize(ref, ref, origsize);
         cv::Mat flt = cv::imread(argv[i+1], cv::IMREAD_GRAYSCALE);
         cv::resize(flt, flt, origsize);
         execTest(ref.data, flt.data);

         std::cout  << std::endl;
         #ifdef AVERAGE_REPS
          execTestMultiple(ref.data, flt.data);
          #endif
         std::cout  << std::endl;
         std::cout << "************************" << std::endl;
         std::cout  << std::endl;
         std::cout << "End of test number "<<j+1 << std::endl;
         std::cout  << std::endl;
         std::cout << "************************" << std::endl;
         std::cout  << std::endl;
         std::cout  << std::endl;
         j++;
      }
   }

   platform->deallocAccelBuffer(hw_ref_buff);
   platform->deallocAccelBuffer(hw_flt_buff);

   deinitPlatform(platform);
   platform = 0;

   return 0;
}


