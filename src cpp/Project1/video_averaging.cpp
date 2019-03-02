﻿#include <iostream>
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
	//cout << "maxLoc = " << maxLoc << endl;
	//minMaxIdx()
	imshow("CCF", outputImg);
	return 0;
}

int main() 
{
	VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\video1.avi");
	String srtOutPath = "D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\output\\";

	//VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	Mat imgRef;
	cap >> imgRef;

	//const Rect roi = Rect(Point2i(1571, 186), Point2i(1746, 338));
	//const Rect roi = Rect(Point2i(1617, 294), Point2i(1662, 310));
	//const Rect roi = Rect(Point2i(630, 855), Point2i(702, 920));

	const Rect roiRef = Rect(Point2i(1571, 186), Point2i(1746, 338));
	const Rect roiTemplate = Rect(Point2i(1616, 294), Point2i(1662, 310));
	//const Rect roiB = Rect(Point2i(630, 855), Point2i(702, 920));

	const int ScaleFactor = 4;
	imgRef = imgRef(roiRef);
	cvtColor(imgRef, imgRef, COLOR_BGR2GRAY);
	imwrite(srtOutPath + "Firstframe.jpg", imgRef);

	ImresizeInFreqFilter filter;
	filter.Process(imgRef, imgRef, ScaleFactor);
	
	Point offset(roiTemplate.x - roiRef.x, roiTemplate.y - roiRef.y);
	offset *= ScaleFactor;

	Mat imgAvgA = Mat(roiTemplate.size() * ScaleFactor, CV_32F, Scalar(0));
	Mat imgAvgB = Mat(roiTemplate.size() * ScaleFactor, CV_32F, Scalar(0));
	int i = 0;
	while (1)
	{
		Mat frameCam;
		cap >> frameCam;
		if (frameCam.empty())
			break;
		Mat imgTemplate;
		imgTemplate = frameCam(roiTemplate);
		cvtColor(imgTemplate, imgTemplate, COLOR_BGR2GRAY);
		filter.Process(imgTemplate, imgTemplate, ScaleFactor);
		imgTemplate.convertTo(imgTemplate, CV_32F);
		imgRef.convertTo(imgRef, CV_32F);
		//imgRef+= frame;
		Point maxLoc;
		FindOffset(imgRef, imgTemplate, maxLoc);
		normalize(imgTemplate, imgTemplate, 0, 255, NORM_MINMAX);
		imgTemplate.convertTo(imgTemplate, CV_8U);
		imshow("imgTemplate", imgTemplate);

		Mat imgRef = frameCam(roiRef);
		cvtColor(imgRef, imgRef, COLOR_BGR2GRAY);
		filter.Process(imgRef, imgRef, ScaleFactor);
		Point offsetRel = maxLoc - offset;
		Rect roi = Rect(maxLoc, roiTemplate.size() * ScaleFactor);
		Mat imgRefA = imgRef(roi).clone();
		imgAvgA += imgRefA;
		normalize(imgRefA, imgRefA, 0, 255, NORM_MINMAX);
		imgRefA.convertTo(imgRefA, CV_8U);
		imshow("imgRefA", imgRefA);

		roi = Rect(offset, roiTemplate.size() * ScaleFactor);
		Mat imgRefB = imgRef(roi).clone();
		imgAvgB += imgRefB;
		normalize(imgRefB, imgRefB, 0, 255, NORM_MINMAX);
		imgRefB.convertTo(imgRefB, CV_8U);
		imshow("imgRefB", imgRefB);

		if (waitKey(10) >= 0) 
			break;
		cout << "frame number: " << i++;
		cout << "\t maxLoc = " << maxLoc << endl;
		cout << "\t maxLoc - offset = " << offsetRel << endl;
	}
	cap.release();

	normalize(imgRef, imgRef, 0, 255, NORM_MINMAX);
	imgRef.convertTo(imgRef, CV_8U);
	imshow("Average", imgRef);
	imwrite(srtOutPath + "imgRef.jpg", imgRef);

	normalize(imgAvgA, imgAvgA, 0, 255, NORM_MINMAX);
	imgAvgA.convertTo(imgAvgA, CV_8U);
	imwrite(srtOutPath + "imgAvgA.jpg", imgAvgA);

	normalize(imgAvgB, imgAvgB, 0, 255, NORM_MINMAX);
	imgAvgB.convertTo(imgAvgB, CV_8U);
	imwrite(srtOutPath + "imgAvgB.jpg", imgAvgB);

	waitKey(0);

	//destroyAllWindows();
	return 0;
}