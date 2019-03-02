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
	//VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	//namedWindow("Video", cv::WINDOW_AUTOSIZE);
	//namedWindow("Average", cv::WINDOW_AUTOSIZE);

	Mat avrFrame;
	cap >> avrFrame;

	//const Rect roi = Rect(Point2i(1571, 186), Point2i(1746, 338));
	//const Rect roi = Rect(Point2i(1617, 294), Point2i(1662, 310));
	//const Rect roi = Rect(Point2i(630, 855), Point2i(702, 920));

	const Rect roiA = Rect(Point2i(1571, 186), Point2i(1746, 338));
	const Rect roiB = Rect(Point2i(1617, 294), Point2i(1662, 310));
	//const Rect roiB = Rect(Point2i(630, 855), Point2i(702, 920));

	const int ScaleFactor = 5;
	avrFrame = avrFrame(roiA);
	cvtColor(avrFrame, avrFrame, COLOR_BGR2GRAY);
	imwrite("Firstframe.jpg", avrFrame);

	ImresizeInFreqFilter filter;
	filter.Process(avrFrame, avrFrame, ScaleFactor);
	
	int i = 0;
	while (1)
	{
		Mat frame;
		cap >> frame;
		frame = frame(roiB);
		if (frame.empty()) 
			break;
		cvtColor(frame, frame, COLOR_BGR2GRAY);
		filter.Process(frame, frame, ScaleFactor);
		frame.convertTo(frame, CV_32F);
		avrFrame.convertTo(avrFrame, CV_32F);
		//avrFrame+= frame;
		Point maxLoc;
		FindOffset(avrFrame, frame, maxLoc);
		normalize(frame, frame, 0, 255, NORM_MINMAX);
		frame.convertTo(frame, CV_8U);
		imshow("frame", frame);
		if (waitKey(10) >= 0) 
			break;
		cout << "frame number: " << i++;
		cout << "\t maxLoc = " << maxLoc << endl;
	}
	cap.release();

	normalize(avrFrame, avrFrame, 0, 255, NORM_MINMAX);
	avrFrame.convertTo(avrFrame, CV_8U);
	imshow("Average", avrFrame);
	imwrite("avrFrame.jpg", avrFrame);
	waitKey(0);

	//destroyAllWindows();
	return 0;
}