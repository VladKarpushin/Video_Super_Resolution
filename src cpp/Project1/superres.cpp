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
	//VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\video1.avi");
	//VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\video1_new.avi");
	VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\screen1.avi");
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

	//const Rect roiRef = Rect(Point2i(1571, 186), Point2i(1746, 338));
	//const Rect roiTemplate = Rect(Point2i(1616, 294), Point2i(1662, 310));
	//const Rect roiRef = Rect(Point2i(543, 740), Point2i(901, 983));
	//const Rect roiTemplate = Rect(Point2i(630, 855), Point2i(702, 920));
	const Rect roiRef = Rect(Point2i(117, 525), Point2i(662, 871));
	const Rect roiTemplate = Rect(Point2i(301, 621), Point2i(407, 731));
	const int ScaleFactor = 2;
	const int MAXOFFSET = 15;

	Mat imgTemplate = frameCam(roiTemplate).clone();
	ImresizeInFreqFilter filter;
	filter.Process(imgTemplate, imgTemplate, ScaleFactor);
	imgTemplate.convertTo(imgTemplate, CV_32F);
	
	Point offset(roiTemplate.x - roiRef.x, roiTemplate.y - roiRef.y);
	offset *= ScaleFactor;
	const Rect roiRefTemplate = Rect(offset, roiTemplate.size() * ScaleFactor);

	Mat imgAvgA = Mat(roiTemplate.size() * ScaleFactor, CV_32F, Scalar(0));
	Mat imgAvgB = Mat(roiTemplate.size() * ScaleFactor, CV_32F, Scalar(0));
	int i = 0;
	int iNumAveragedFrames = 0;
	while (1)
	{
		cap >> frameCam;
		if (frameCam.empty())
			break;
		cvtColor(frameCam, frameCam, COLOR_BGR2GRAY);

		Mat imgRef = frameCam(roiRef).clone();
		imgRef.convertTo(imgRef, CV_32F);
		filter.Process(imgRef, imgRef, ScaleFactor);

		Point maxLoc;
		FindOffset(imgRef, imgTemplate, maxLoc);

		Point offsetRef = offset - maxLoc;
		//if ((abs(offsetRef.x) < MAXOFFSET) && (abs(offsetRef.y) < MAXOFFSET))
		if (sqrt(offsetRef.x * offsetRef.x + offsetRef.y * offsetRef.y) < MAXOFFSET)
		{
			Mat imgRefA = imgRef(roiRefTemplate - offsetRef).clone();
			imgAvgA += imgRefA;
			normalize(imgRefA, imgRefA, 0, 255, NORM_MINMAX);
			imgRefA.convertTo(imgRefA, CV_8U);
			imshow("imgRefA(stabilized)", imgRefA);
			iNumAveragedFrames++;
		}
		else
			cout << "!!!MAXOFFSET!!!" << offsetRef << endl;

		Mat imgRefB = imgRef(roiRefTemplate).clone();
		imgAvgB += imgRefB;
		normalize(imgRefB, imgRefB, 0, 255, NORM_MINMAX);
		imgRefB.convertTo(imgRefB, CV_8U);
		imshow("imgRefB", imgRefB);

		if (waitKey(1) >= 0) 
			break;
		cout << "frame number: " << i++;
		cout << "\t maxLoc = " << maxLoc;
		cout << "\t maxLoc - offset = " << offsetRef << endl;
	}
	cap.release();

	normalize(imgAvgA, imgAvgA, 0, 255, NORM_MINMAX);
	imgAvgA.convertTo(imgAvgA, CV_8U);
	imwrite(srtOutPath + "imgAvgA(stabilized).jpg", imgAvgA);

	normalize(imgAvgB, imgAvgB, 0, 255, NORM_MINMAX);
	imgAvgB.convertTo(imgAvgB, CV_8U);
	imwrite(srtOutPath + "imgAvgB.jpg", imgAvgB);

	cout << "Number of averaged frames = " << iNumAveragedFrames << endl;

	waitKey(0);

	//destroyAllWindows();
	return 0;
}