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
* registration calss of the whole app
* credits goes also to the author of this repo: https://github.com/mariusherzog/ImageRegistration
*
****************************************************************/
#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "optimize.hpp"
#ifdef HW_REG
   #include "mutual_information_hw.hpp"
   #define HWUSAGE true
#endif


/**
 * @brief The registration interface defines the signatures of a registration
 *        operation of a floating image to a reference image.
 */
class registration
{
   public:
      virtual cv::Mat register_images(cv::Mat ref, cv::Mat flt) = 0;
      virtual ~registration() = 0;
};

registration::~registration()
{
}


static cv::Mat transform(cv::Mat image, double tx, double ty, double a11, double a12, double a21, double a22)
{
   cv::Mat trans_mat = (cv::Mat_<double>(2,3) << a11, a12, tx, a21, a22, ty);

   cv::Mat out = image.clone();
   warpAffine(image, out, trans_mat, image.size());
   return out;
}

static bool is_inverted(cv::Mat ref, cv::Mat flt)
{
   std::vector<double> hist_ref(256, 0);
   std::vector<double> hist_flt(256, 0);

   for (int i=0; i<ref.rows; ++i) {
      for (int j=0; j<ref.cols; ++j) {
         int val = ref.at<uchar>(i, j);
         hist_ref[val]++;
      }
   }

   for (int i=0; i<flt.rows; ++i) {
      for (int j=0; j<flt.cols; ++j) {
         int val = flt.at<uchar>(i, j);
         hist_flt[val]++;
      }
   }

   std::transform(hist_ref.begin(), hist_ref.end(), hist_ref.begin(), [&ref](int val) { return 1.0*val / (1.0*ref.cols*ref.rows); });
   std::transform(hist_flt.begin(), hist_flt.end(), hist_flt.begin(), [&flt](int val) { return 1.0*val / (1.0*flt.cols*flt.rows); });

   std::vector<double> distances(256, 0);
   std::transform(hist_ref.begin(), hist_ref.end(), hist_flt.begin(), distances.begin(),
                  [](double ref_val, double flt_val) { return fabs(ref_val - flt_val); });

   double distance_flt = std::accumulate(distances.begin(), distances.end(), 0.0);

   // invert
   std::reverse(hist_flt.begin(), hist_flt.end());

   std::transform(hist_ref.begin(), hist_ref.end(), hist_flt.begin(), distances.begin(),
                  [](double ref_val, double inv_val) { return fabs(ref_val - inv_val); });

   double distance_inv = std::accumulate(distances.begin(), distances.end(), 0.0);

   return distance_flt > distance_inv;
}

/**
 * @brief The mutual information strategy uses mutual information as a
 *        similarity metric for registration.
 * Optimization is performed by Powell's method which iteratively optimizes a
 * parameter in the parameter space one after another.
 * Image moments are used for an initial estimation.
 */
class mutualinformation : public registration
{
   public:
      cv::Mat register_images(cv::Mat ref, cv::Mat flt) override
      {
         cv::Size origsize(512, 512);
         cv::resize(ref, ref, origsize);

         if (is_inverted(ref, flt)) {
            cv::bitwise_not(flt, flt);
         }

	 #ifndef NOBLUR
         cv::Size ksize(5,5);
         cv::GaussianBlur(ref, ref, ksize, 10, 10);
         cv::GaussianBlur(flt, flt, ksize, 10, 10);
	 #else

	 #endif
         auto max_size = ref.size();
         cv::resize(flt, flt, max_size);

         double tx, ty, a11, a12, a21, a22;
         estimate_initial(ref, flt, tx, ty, a11, a12, a21, a22);


         std::vector<double> init {tx, ty, a11, a12, a21, a22};
         std::vector<double> rng {80.0, 80.0, 1.0, 1.0, 1.0, 1.0};

         std::pair<std::vector<double>::iterator, std::vector<double>::iterator> o {init.begin(), init.end()};
         optimize_powell(o, {rng.begin(), rng.end()}, std::bind(cost_function, ref, flt, std::placeholders::_1));

         tx = init[0];
         ty = init[1];
         a11 = init[2];
         a12 = init[3];
         a21 = init[4];
         a22 = init[5];

         cv::Mat fin = transform(flt, tx, ty, a11, a12, a21, a22);
         #ifdef HW_REG
            float mutual_inf = HwRegistration::s_mutual_information(ref, fin);
         #else
            double mutual_inf = mutual_information(ref, fin);
         #endif
         std::cout << exp(-mutual_inf) << "*** \n";

         return fin.clone();
      }

   private:
      static double mutual_information(cv::Mat ref, cv::Mat flt)
      {
         cv::Mat joint_histogram(256, 256, CV_64FC1, cv::Scalar(0));

         for (int i=0; i<ref.cols; ++i) {
            for (int j=0; j<ref.rows; ++j) {
               int ref_intensity = ref.at<uchar>(j,i);
               int flt_intensity = flt.at<uchar>(j,i);
               joint_histogram.at<double>(ref_intensity, flt_intensity) = joint_histogram.at<double>(ref_intensity, flt_intensity)+1;
            }
         }



         for (int i=0; i<256; ++i) {
            for (int j=0; j<256; ++j) {
               joint_histogram.at<double>(j, i) = joint_histogram.at<double>(j, i)/(1.0*ref.rows*ref.cols);
            }
         }
	 #ifndef NOBLUR
         cv::Size ksize(7, 7);
         cv::GaussianBlur(joint_histogram, joint_histogram, ksize, 7, 7);
	 #else

	/* code to include if the token is defined */

	#endif

         double entropy = 0.0;
         for (int i=0; i<256; ++i) {
            for (int j=0; j<256; ++j) {
               double v = joint_histogram.at<double>(j, i);
               if (v > 0.000000000000001) {
                  entropy += v*log(v)/log(2);
               }
            }
         }
         entropy *= -1;


         std::vector<double> hist_ref(256, 0.0);
         for (int i=0; i<joint_histogram.rows; ++i) {
            for (int j=0; j<joint_histogram.cols; ++j) {
               hist_ref[i] += joint_histogram.at<double>(i, j);
            }
         }

         cv::Size ksize2(5,0);


         std::vector<double> hist_flt(256, 0.0);
         for (int i=0; i<joint_histogram.cols; ++i) {
            for (int j=0; j<joint_histogram.rows; ++j) {
               hist_flt[i] += joint_histogram.at<double>(j, i);
            }
         }




         double entropy_ref = 0.0;
         for (int i=0; i<256; ++i) {
            if (hist_ref[i] > 0.000000000001) {
               entropy_ref += hist_ref[i] * log(hist_ref[i])/log(2);
            }
         }
         entropy_ref *= -1;

         double entropy_flt = 0.0;
         for (int i=0; i<256; ++i) {
            if (hist_flt[i] > 0.000000000001) {
               entropy_flt += hist_flt[i] * log(hist_flt[i])/log(2);
            }
         }
         entropy_flt *= -1;

         double mutual_information = entropy_ref + entropy_flt - entropy;
         return mutual_information;
      }

      static double cost_function(cv::Mat ref, cv::Mat flt,
                           std::vector<double>::iterator affine_params)
      {
         const double tx = affine_params[0];
         const double ty = affine_params[1];
         const double a11 = affine_params[2];
         const double a12 = affine_params[3];
         const double a21 = affine_params[4];
         const double a22 = affine_params[5];
         #ifdef HW_REG
            return exp(-HwRegistration::s_mutual_information(ref, transform(flt, tx, ty, a11, a12, a21, a22)));
         #else
            return exp(-mutual_information(ref, transform(flt, tx, ty, a11, a12, a21, a22)));
         #endif
      }

      static void estimate_initial(cv::Mat ref, cv::Mat flt,
                            double& tx, double& ty,
                            double& a11, double& a12, double& a21, double& a22)
      {
         cv::Moments im_mom = moments(ref);
         cv::Moments pt_mom = moments(flt);

         cv::Mat ref_bin = ref.clone();
         cv::Mat flt_bin = flt.clone();
         cv::threshold(ref, ref_bin, 40, 256, 0);
         cv::threshold(flt, flt_bin, 40, 256, 0);


         double pt_avg_10 = pt_mom.m10/pt_mom.m00;
         double pt_avg_01 = pt_mom.m01/pt_mom.m00;
         double pt_mu_20 = (pt_mom.m20/pt_mom.m00*1.0)-(pt_avg_10*pt_avg_10);
         double pt_mu_02 = (pt_mom.m02/pt_mom.m00*1.0)-(pt_avg_01*pt_avg_01);
         double pt_mu_11 = (pt_mom.m11/pt_mom.m00*1.0)-(pt_avg_01*pt_avg_10);

         double im_avg_10 = im_mom.m10/im_mom.m00;
         double im_avg_01 = im_mom.m01/im_mom.m00;
         double im_mu_20 = (im_mom.m20/im_mom.m00*1.0)-(im_avg_10*im_avg_10);
         double im_mu_02 = (im_mom.m02/im_mom.m00*1.0)-(im_avg_01*im_avg_01);
         double im_mu_11 = (im_mom.m11/im_mom.m00*1.0)-(im_avg_01*im_avg_10);

         tx = im_mom.m10/im_mom.m00 - pt_mom.m10/pt_mom.m00;
         ty = im_mom.m01/im_mom.m00 - pt_mom.m01/pt_mom.m00;


         double rho = 0.5f * atan((2.0*pt_mu_11)/(pt_mu_20 - pt_mu_02));
         double rho_im = 0.5f * atan((2.0*im_mu_11)/(im_mu_20 - im_mu_02));

         const double rho_diff = rho_im - rho;

         const double roundness = (pt_mom.m20/pt_mom.m00) / (pt_mom.m02/pt_mom.m00);
         if (abs(roundness-1.0) >= 0.3) {
            a11 = cos(rho_diff);
            a12 = -sin(rho_diff);
            a21 = sin(rho_diff);
            a22 = cos(rho_diff);
         } else {
            a11 = 1.0;
            a12 = 0.0;
            a21 = 0.0;
            a22 = 1.0;
         }
      }
};

/**
 * @brief The identity class performs no registration at all and leaves
 *        the floating image unmodified.
 */
class identity : public registration
{
      cv::Mat register_images(cv::Mat, cv::Mat flt) override
      {
         return flt.clone();
      }
};


#endif // REGISTER_HPP
