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
	//cout << "maxLoc = " << maxLoc << endl;
	//minMaxIdx()
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

	Mat imgRefFirstFrame = frameCam(roiRef).clone();
	imwrite(srtOutPath + "imgRefFirstFrame.jpg", imgRefFirstFrame);

	ImresizeInFreqFilter filter;
	filter.Process(imgRefFirstFrame, imgRefFirstFrame, ScaleFactor);
	
	Point offset(roiTemplate.x - roiRef.x, roiTemplate.y - roiRef.y);
	offset *= ScaleFactor;
	const Rect roiTemplateRef = Rect(offset, roiTemplate.size() * ScaleFactor);

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

		Mat imgTemplate;
		imgTemplate = frameCam(roiTemplate).clone();
		filter.Process(imgTemplate, imgTemplate, ScaleFactor);
		imgTemplate.convertTo(imgTemplate, CV_32F);
		imgRefFirstFrame.convertTo(imgRefFirstFrame, CV_32F);

		Point maxLoc;
		FindOffset(imgRefFirstFrame, imgTemplate, maxLoc);
		normalize(imgTemplate, imgTemplate, 0, 255, NORM_MINMAX);
		imgTemplate.convertTo(imgTemplate, CV_8U);
		imshow("imgTemplate", imgTemplate);

		Mat imgRef = frameCam(roiRef).clone();
		filter.Process(imgRef, imgRef, ScaleFactor);

		Point offsetRef = maxLoc - offset;
		const int MAXOFFSET = 15;
		//if ((abs(offsetRef.x) < MAXOFFSET) && (abs(offsetRef.y) < MAXOFFSET))
		if (sqrt(offsetRef.x * offsetRef.x + offsetRef.y * offsetRef.y) < MAXOFFSET)
		{
			Mat imgRefA = imgRef(roiTemplateRef - offsetRef).clone();
			imgAvgA += imgRefA;
			normalize(imgRefA, imgRefA, 0, 255, NORM_MINMAX);
			imgRefA.convertTo(imgRefA, CV_8U);
			imshow("imgRefA", imgRefA);
			iNumAveragedFrames++;
		}
		else
			cout << "!!!MAXOFFSET!!!" << offsetRef << endl;

		Mat imgRefB = imgRef(roiTemplateRef).clone();
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
	imwrite(srtOutPath + "imgAvgA.jpg", imgAvgA);

	normalize(imgAvgB, imgAvgB, 0, 255, NORM_MINMAX);
	imgAvgB.convertTo(imgAvgB, CV_8U);
	imwrite(srtOutPath + "imgAvgB.jpg", imgAvgB);

	cout << "Number of averaged frames = " << iNumAveragedFrames << endl;

	waitKey(0);

	//destroyAllWindows();
	return 0;
}