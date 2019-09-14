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
	//VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\plane3.MOV");
	String srt_out_path = "D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\output\\";

	if (!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	Mat frame_cam;
	cap >> frame_cam;
	if (frame_cam.empty())
	{
		cout << "frame_cam.empty()" << endl;
		return -1;
	}
	cvtColor(frame_cam, frame_cam, COLOR_BGR2GRAY);

	// for plane3.MOV
	//const Rect roiRef = Rect(Point2i(820, 312), Point2i(1200, 640));
	//const Rect roiTemplate = Rect(Point2i(954, 445), Point2i(1043, 498));
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 300 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100 for screen1.avi

	// for plane1.MOV
	//const Rect roiRef = Rect(Point2i(878, 435), Point2i(1225, 753));
	//const Rect roiTemplate = Rect(Point2i(942, 524), Point2i(1092, 619));
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 200 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100

	// for plane2.MOV
	//const Rect roiRef = Rect(Point2i(845, 304), Point2i(1300, 663));
	//const Rect roiTemplate = Rect(Point2i(955, 415), Point2i(1115, 528));
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 200 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
												   
	//for !moon_zoom_2.MOV
	const Rect roiRef = Rect(Point2i(800, 350), Point2i(1300, 800));
	const Rect roiTemplate = Rect(Point2i(972, 475), Point2i(1075, 586));
	const int SCALE_FACTOR = 5;
	const int MAX_OBJ_OFFSET = 100 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100

	// for screen1.avi
	//const Rect roiRef = Rect(Point2i(117, 525), Point2i(662, 871));		
	//const Rect roiTemplate = Rect(Point2i(301, 621), Point2i(407, 731));
	//const int SCALE_FACTOR = 2;
	//const int MAX_OBJ_OFFSET = 15;

	Mat imgTemplate = frame_cam(roiTemplate).clone();
	ImresizeInFreqFilter filter;
	filter.Process(imgTemplate, imgTemplate, SCALE_FACTOR);
	imgTemplate.convertTo(imgTemplate, CV_32F);
	
	Point offset(roiTemplate.x - roiRef.x, roiTemplate.y - roiRef.y);
	offset *= SCALE_FACTOR;
	Size2i size_out = (roiTemplate.size() - Size2i(1, 1)) * SCALE_FACTOR;
	const Rect roiRefTemplate = Rect(offset, size_out);
	Mat img_averaged = Mat(size_out, CV_32F, Scalar(0));	// superresolution image
	//Mat imgAvgB = Mat(roiTemplate.size() * SCALE_FACTOR, CV_32F, Scalar(0));
	int i = 0;
	int iNumAveragedFrames = 0;
	//while (1)
	while (iNumAveragedFrames < 50)
	{
		cap >> frame_cam;
		if (frame_cam.empty())
			break;
		cvtColor(frame_cam, frame_cam, COLOR_BGR2GRAY);

		Mat imgRef = frame_cam(roiRef).clone();
		imgRef.convertTo(imgRef, CV_32F);
		filter.Process(imgRef, imgRef, SCALE_FACTOR);

		Point maxLoc;
		FindOffset(imgRef, imgTemplate, maxLoc);

		Point object_offset = offset - maxLoc;	// offset of detected object
		auto offset_norm = norm(object_offset);
		if (offset_norm < MAX_OBJ_OFFSET)
		{
			Mat img_obj = imgRef(roiRefTemplate - object_offset).clone();	// extracted object from the frame
			img_averaged += img_obj;
			normalize(img_obj, img_obj, 0, 255, NORM_MINMAX);
			img_obj.convertTo(img_obj, CV_8U);
			imshow("img_obj(stabilized)", img_obj);
			iNumAveragedFrames++;
		}
		else
			cout << "!!!MAX_OBJ_OFFSET!!!" << object_offset << " norm = " << offset_norm << endl;

		Mat img_obj_b = imgRef(roiRefTemplate).clone();	// extracted image without stabilization
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
	imwrite(srt_out_path + "imgTemplate(first frame).jpg", imgTemplate);

	normalize(img_averaged, img_averaged, 0, 255, NORM_MINMAX);
	img_averaged.convertTo(img_averaged, CV_8U);
	imwrite(srt_out_path + "img_averaged(stabilized).jpg", img_averaged);
	cout << "Number of averaged frames = " << iNumAveragedFrames << endl;

	waitKey(0);
	return 0;
}