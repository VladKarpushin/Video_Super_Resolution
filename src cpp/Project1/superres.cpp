// 2019-03-03 Video super resolution and camera stabilization
//
//

#include <iostream>
#include "opencv2/opencv.hpp"
#include  "ImresizeInFreqFilter.hpp"

using namespace cv;
using std::cout;
using std::endl;

int FindOffset(const Mat & inputImg, const Mat & inputImgTemplate, Point & maxLoc)
{
	Mat outputImg;
	// TM_CCOEFF_NORMED
	matchTemplate(inputImg, inputImgTemplate, outputImg, TM_CCORR_NORMED);
	double minVal; double maxVal; Point minLoc;
	minMaxLoc(outputImg, &minVal, &maxVal, &minLoc, &maxLoc);
	normalize(outputImg, outputImg, 0, 255, NORM_MINMAX);
	outputImg.convertTo(outputImg, CV_8U);
	imshow("CCF", outputImg);
	return 0;
}

int main() 
{
 	//VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\screen1.avi");
	VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\!moon_zoom_2.MOV");
	String srtOutPath = "D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\output\\";

	if (!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	Mat frameCam;
	cap >> frameCam;
	if (frameCam.empty())
	{
		cout << "frameCam.empty()" << endl;
		return -1;
	}
	cvtColor(frameCam, frameCam, COLOR_BGR2GRAY);

	const Rect roiRef = Rect(Point2i(800, 350), Point2i(1300, 800));		// for !moon_zoom_2.MOV
	const Rect roiTemplate = Rect(Point2i(972, 475), Point2i(1075, 586));	// for !moon_zoom_2.MOV
	const int SCALE_FACTOR = 5; // for screen1.avi
	const int MAX_OBJ_OFFSET = 100 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100 for screen1.avi

	//const Rect roiRef = Rect(Point2i(117, 525), Point2i(662, 871));		// for screen1.avi
	//const Rect roiTemplate = Rect(Point2i(301, 621), Point2i(407, 731));	// for screen1.avi
	//const int SCALE_FACTOR = 2; // for screen1.avi
	//const int MAX_OBJ_OFFSET = 15; // for screen1.avi

	Mat imgTemplate = frameCam(roiTemplate).clone();
	ImresizeInFreqFilter filter;
	filter.Process(imgTemplate, imgTemplate, SCALE_FACTOR);
	imgTemplate.convertTo(imgTemplate, CV_32F);
	
	Point offset(roiTemplate.x - roiRef.x, roiTemplate.y - roiRef.y);
	offset *= SCALE_FACTOR;
	const Rect roiRefTemplate = Rect(offset, roiTemplate.size() * SCALE_FACTOR);

	Mat img_averaged = Mat(roiTemplate.size() * SCALE_FACTOR, CV_32F, Scalar(0));	// superresolution image
	//Mat imgAvgB = Mat(roiTemplate.size() * SCALE_FACTOR, CV_32F, Scalar(0));
	int i = 0;
	int iNumAveragedFrames = 0;
	//while (1)
	while (iNumAveragedFrames < 10)
	{
		cap >> frameCam;
		if (frameCam.empty())
			break;
		cvtColor(frameCam, frameCam, COLOR_BGR2GRAY);

		Mat imgRef = frameCam(roiRef).clone();
		imgRef.convertTo(imgRef, CV_32F);
		filter.Process(imgRef, imgRef, SCALE_FACTOR);

		Point maxLoc;
		FindOffset(imgRef, imgTemplate, maxLoc);

		Point object_offset = offset - maxLoc;	// offset of detected object
		//auto nnn = norm(object_offset);
		//if ((abs(offsetRef.x) < MAX_OBJ_OFFSET) && (abs(offsetRef.y) < MAX_OBJ_OFFSET))
		//if (sqrt(object_offset.x * object_offset.x + object_offset.y * object_offset.y) < MAX_OBJ_OFFSET)
		if (norm(object_offset) < MAX_OBJ_OFFSET)
		{
			Mat img_obj = imgRef(roiRefTemplate - object_offset).clone();	// extracted object from the frame
			img_averaged += img_obj;
			normalize(img_obj, img_obj, 0, 255, NORM_MINMAX);
			img_obj.convertTo(img_obj, CV_8U);
			imshow("img_obj(stabilized)", img_obj);
			iNumAveragedFrames++;
		}
		else
			cout << "!!!MAX_OBJ_OFFSET!!!" << object_offset << " norm = " << norm(object_offset) << endl;

		Mat img_obj_b = imgRef(roiRefTemplate).clone();
		//imgAvgB += img_obj_b;
		normalize(img_obj_b, img_obj_b, 0, 255, NORM_MINMAX);
		img_obj_b.convertTo(img_obj_b, CV_8U);
		imshow("img_obj_b", img_obj_b);

		if (waitKey(1) >= 0)
			break;
		cout << "frame number: " << i++;
		cout << "\t maxLoc = " << maxLoc;
		cout << "\t maxLoc - offset = " << object_offset << endl;
	}
	cap.release();

	normalize(imgTemplate, imgTemplate, 0, 255, NORM_MINMAX);
	imgTemplate.convertTo(imgTemplate, CV_8U);
	imwrite(srtOutPath + "imgTemplate(first frame).jpg", imgTemplate);

	normalize(img_averaged, img_averaged, 0, 255, NORM_MINMAX);
	img_averaged.convertTo(img_averaged, CV_8U);
	imwrite(srtOutPath + "img_averaged(stabilized).jpg", img_averaged);

	//normalize(imgAvgB, imgAvgB, 0, 255, NORM_MINMAX);
	//imgAvgB.convertTo(imgAvgB, CV_8U);
	//imwrite(srtOutPath + "imgAvgB.jpg", imgAvgB);

	cout << "Number of averaged frames = " << iNumAveragedFrames << endl;

	waitKey(0);

	//destroyAllWindows();
	return 0;
}