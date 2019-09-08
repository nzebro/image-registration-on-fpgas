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
* High-Level-Synthesis testbench file for Mutual Information computation
*
****************************************************************/
#include <iostream>
#include <cmath>
#include <random>
#include <stdio.h>
#include <ap_int.h>
#define MYROWS 512
#define MYCOLS 512
#define J_HISTO_ROWS 256
#define J_HISTO_COLS 256

#define ANOTHER_DIMENSION 256

#define TWO_FLOAT 2
#define DIM 512

typedef float data_t;
typedef ap_uint<8> MY_PIXEL;


void mutual_information_master( MY_PIXEL * ref, MY_PIXEL * flt,  data_t * mutual_info);


int main(){

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
   printf("%f\n",mutualinfo);



   data_t mihls;

   mutual_information_master(ref,flt,&mihls);

   printf("%f\n", mihls);
   if(mutualinfo - mihls > 0.01){
       return 1;
   }


   return 0;
}


