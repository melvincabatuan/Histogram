#include "com_cabatuan_histogram_MainActivity.h"
#include <android/log.h>
#include <android/bitmap.h>

#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

#define  LOG_TAG    "Histogram"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)



/// Real-time 12-20 fps
void calculateHistogram(const Mat &Image, Mat &histoImage)
{
	int histSize = 255;
	float range[] = { 0, 256 } ;
	const float* histRange = { range };
	bool uniform = true; 
	bool accumulate = false;
	Mat b_hist, g_hist, r_hist;
	std::vector<Mat> bgr_planes;
	split(Image, bgr_planes );	
	
	calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );
	
	int bin_w = cvRound( (float) Image.cols/histSize );
	Mat histImage( Image.rows, Image.cols, CV_8UC3, Scalar( 0,0,0) );

	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	for( int i = 1; i < histSize; i++ ){
		line( histImage, Point( bin_w*(i-1), Image.rows - cvRound(b_hist.at<float>(i-1)) ) , Point( bin_w*(i), Image.rows - cvRound(b_hist.at<float>(i)) ), Scalar(255, 0, 0), 2, 8, 0 );
		line( histImage, Point( bin_w*(i-1), Image.rows - cvRound(g_hist.at<float>(i-1)) ) , Point( bin_w*(i), Image.rows - cvRound(g_hist.at<float>(i)) ), Scalar(0, 255, 0), 2, 8, 0 );
		line( histImage, Point( bin_w*(i-1), Image.rows - cvRound(r_hist.at<float>(i-1)) ) , Point( bin_w*(i), Image.rows - cvRound(r_hist.at<float>(i)) ), Scalar(0, 0, 255), 2, 8, 0 );
	}

	histoImage = histImage;
}








double t;
Mat srcBGR;
Mat histogramImg;

/*
 * Class:     com_cabatuan_histogram_MainActivity
 * Method:    process
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_com_cabatuan_histogram_MainActivity_process
  (JNIEnv *pEnv, jobject clazz, jobject pTarget, jbyteArray pSource){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent; // Links to Bitmap content

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   /// cv::Mat for YUV420sp source and output BGRA 
    Mat src(bitmapInfo.height + bitmapInfo.height/2, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);

/***********************************************************************************************/
    /// Native Image Processing HERE... 
    if(srcBGR.empty())
       srcBGR = Mat(bitmapInfo.height, bitmapInfo.width, CV_8UC3);
    
    cvtColor(src, srcBGR, CV_YUV420sp2RGB);
    
    /*
    std::vector<Mat> channels;
	split( srcBGR, channels);
	
	equalizeHist(channels[0], channels[0]);
	equalizeHist(channels[1], channels[1]);
	equalizeHist(channels[2], channels[2]);
	*/
	
	// merge( channels, equalized );
    
    if(histogramImg.empty())
       histogramImg = Mat(bitmapInfo.height, bitmapInfo.width, CV_8UC3);
    
    calculateHistogram(srcBGR, histogramImg);
    
    cvtColor(histogramImg, mbgra, CV_BGR2BGRA);
 
/************************************************************************************************/ 
   
   /// Release Java byte buffer and unlock backing bitmap
   pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();

}
