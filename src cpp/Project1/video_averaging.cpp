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
	VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\video1.avi");
	String srtOutPath = "D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\output\\";

	//VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	Mat avrFrame;
	cap >> avrFrame;

	//const Rect roi = Rect(Point2i(1571, 186), Point2i(1746, 338));
	//const Rect roi = Rect(Point2i(1617, 294), Point2i(1662, 310));
	//const Rect roi = Rect(Point2i(630, 855), Point2i(702, 920));

	const Rect roiA = Rect(Point2i(1571, 186), Point2i(1746, 338));
	const Rect roiB = Rect(Point2i(1617, 294), Point2i(1662, 310));
	//const Rect roiB = Rect(Point2i(630, 855), Point2i(702, 920));

	const int ScaleFactor = 10;
	avrFrame = avrFrame(roiA);
	cvtColor(avrFrame, avrFrame, COLOR_BGR2GRAY);
	imwrite(srtOutPath + "Firstframe.jpg", avrFrame);

	ImresizeInFreqFilter filter;
	filter.Process(avrFrame, avrFrame, ScaleFactor);
	
	Point offset(roiB.x - roiA.x, roiB.y - roiA.y);
	offset *= ScaleFactor;
	int i = 0;
	while (1)
	{
		Mat frameCam;
		cap >> frameCam;
		if (frameCam.empty())
			break;
		Mat imgTemplate;
		imgTemplate = frameCam(roiB);
		cvtColor(imgTemplate, imgTemplate, COLOR_BGR2GRAY);
		filter.Process(imgTemplate, imgTemplate, ScaleFactor);
		imgTemplate.convertTo(imgTemplate, CV_32F);
		avrFrame.convertTo(avrFrame, CV_32F);
		//avrFrame+= frame;
		Point maxLoc;
		FindOffset(avrFrame, imgTemplate, maxLoc);
		normalize(imgTemplate, imgTemplate, 0, 255, NORM_MINMAX);
		imgTemplate.convertTo(imgTemplate, CV_8U);

		imshow("imgTemplate", imgTemplate);
		//imgTemplate = frameCam(roiB);

		Point offsetRel = maxLoc - offset;
		//Rect roiBCorrected = Rect(ScaleFactor * (roiB.tl() - roiA.tl()), ScaleFactor * (roiB.br() - roiA.br()) - offsetRel);
		//Mat imgA = frameCam(roiA);
		//cvtColor(imgA, imgA, COLOR_BGR2GRAY);
		//filter.Process(imgA, imgA, ScaleFactor);
//		imshow("imgTemplCorr", imgA(roiBCorrected));

		if (waitKey(10) >= 0) 
			break;
		cout << "frame number: " << i++;
		cout << "\t maxLoc = " << maxLoc << endl;
		cout << "\t maxLoc - offset = " << offsetRel << endl;
	}
	cap.release();

	normalize(avrFrame, avrFrame, 0, 255, NORM_MINMAX);
	avrFrame.convertTo(avrFrame, CV_8U);
	imshow("Average", avrFrame);
	imwrite(srtOutPath + "avrFrame.jpg", avrFrame);
	waitKey(0);

	//destroyAllWindows();
	return 0;
}