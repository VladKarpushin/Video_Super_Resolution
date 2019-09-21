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
	VideoCapture cap("D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\new2\\one_book.MTS");
	
	String srt_out_path = "D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\output\\";

	if (!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	Mat img_frame;
	cap >> img_frame;
	if (img_frame.empty())
	{
		cout << "img_frame.empty()" << endl;
		return -1;
	}
	cvtColor(img_frame, img_frame, COLOR_BGR2GRAY);

	////for video_man_cut.avi
	//const Rect roi_frame = Rect(Point2i(300, 200), Point2i(1800, 800));
	//img_frame = img_frame(roi_frame).clone();
	//const Rect roi_template = Rect(Point2i(146, 133), Point2i(264, 223));
	//const int SCALE_FACTOR = 1;
	//const int MAX_OBJ_OFFSET = 300 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
	//const Rect roi_template_new = Rect(roi_template.tl()*SCALE_FACTOR, roi_template.size()*SCALE_FACTOR);

	// for plane3.MOV
	//const Rect roi_frame = Rect(Point2i(820, 312), Point2i(1200, 640));
	//const Rect roi_template = Rect(Point2i(954, 445), Point2i(1043, 498));
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 300 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100 for screen1.avi

	// for plane1.MOV
	//const Rect roi_frame = Rect(Point2i(878, 435), Point2i(1225, 753));
	//const Rect roi_template = Rect(Point2i(942, 524), Point2i(1092, 619));
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 200 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100

	// for plane2.MOV
	//const Rect roi_frame = Rect(Point2i(845, 304), Point2i(1300, 663));
	//const Rect roi_template = Rect(Point2i(955, 415), Point2i(1115, 528));
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 200 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
												   
	//for !moon_zoom_2.MOV
	//const Rect roi_frame = Rect(Point2i(800, 350), Point2i(1300, 800));
	//img_frame = img_frame(roi_frame).clone();
	//const Rect roi_template = Rect(Point2i(172, 125), Point2i(264, 223));
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 100 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
	//const Rect roi_template_new = Rect(roi_template.tl()*SCALE_FACTOR, roi_template.size()*SCALE_FACTOR);

	// for bookshelf
	//const Rect roi_frame = Rect(Point2i(680, 141), Point2i(1343, 684));
	//img_frame = img_frame(roi_frame).clone();
	//const Rect roi_template = Rect(Point2i(347, 170), Point2i(407, 514));	// w and h should be even
	//const int SCALE_FACTOR = 5;
	//const int MAX_OBJ_OFFSET = 100 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
	//const Rect roi_template_new = Rect(roi_template.tl()*SCALE_FACTOR, roi_template.size()*SCALE_FACTOR);

	// for !license_plate3
	//const Rect roi_frame = Rect(Point2i(853, 378), Point2i(1278, 657));
	//img_frame = img_frame(roi_frame).clone();
	//Rect roi_template = Rect(Point2i(134, 84), Point2i(257, 140));	// w and h should be even
	//roi_template.width = roi_template.width & -2;
	//roi_template.height = roi_template.height& -2;
	//const int SCALE_FACTOR = 3;
	//const int MAX_OBJ_OFFSET = 100 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
	//const Rect roi_template_new = Rect(roi_template.tl()*SCALE_FACTOR, roi_template.size()*SCALE_FACTOR);

	// for one_book
	const Rect roi_frame = Rect(Point2i(647, 136), Point2i(1747, 949));
	img_frame = img_frame(roi_frame).clone();
	Rect roi_template = Rect(Point2i(560, 237), Point2i(922, 442));	// w and h should be even
	roi_template.width = roi_template.width & -2;
	roi_template.height = roi_template.height& -2;
	const int SCALE_FACTOR = 4;
	const int MAX_OBJ_OFFSET = 100 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
	const Rect roi_template_new = Rect(roi_template.tl()*SCALE_FACTOR, roi_template.size()*SCALE_FACTOR);

	Mat img_template = img_frame(roi_template).clone();
	ImresizeInFreqFilter filter;
	filter.Process(img_template, img_template, SCALE_FACTOR);
	img_template.convertTo(img_template, CV_32F);
	
	Mat img_averaged = img_template.clone();	// superresolution image
	int i = 0;
	int num_avr_frames = 0;
	//while (1)
	while (num_avr_frames < 100)
	{
		cap >> img_frame;
		if (img_frame.empty())
			break;
		cvtColor(img_frame, img_frame, COLOR_BGR2GRAY);

		img_frame = img_frame(roi_frame).clone();
		img_frame.convertTo(img_frame, CV_32F);
		filter.Process(img_frame, img_frame, SCALE_FACTOR);

		Point maxLoc;
		FindOffset(img_frame, img_template, maxLoc);

		Point offset = roi_template_new.tl() - maxLoc;	// offset of detected object
		auto offset_norm = norm(offset);
		if (offset_norm < MAX_OBJ_OFFSET)
		{
			Mat img_obj = img_frame(roi_template_new - offset).clone();	// extracted object from the frame
			img_averaged += img_obj;
			normalize(img_obj, img_obj, 0, 255, NORM_MINMAX);
			img_obj.convertTo(img_obj, CV_8U);
			imshow("img_obj(stabilized)", img_obj);
			num_avr_frames++;
		}
		else
			cout << "!!!MAX_OBJ_OFFSET!!!" << offset << " norm = " << offset_norm << endl;

		Mat img_obj_b = img_frame(roi_template_new).clone();	// extracted image without stabilization
		normalize(img_obj_b, img_obj_b, 0, 255, NORM_MINMAX);
		img_obj_b.convertTo(img_obj_b, CV_8U);
		imshow("img_obj_b", img_obj_b);

		Mat img_avr;
		normalize(img_averaged, img_avr, 0, 255, NORM_MINMAX);
		img_avr.convertTo(img_avr, CV_8U);
		imshow("img_avr", img_avr);

		if (waitKey(1) >= 0)
			break;
		cout << "frame number: " << i++;
		cout << "\t maxLoc = " << maxLoc;
		cout << "\t maxLoc - offset = " << offset << endl;
	}
	cap.release();

	normalize(img_template, img_template, 0, 255, NORM_MINMAX);
	img_template.convertTo(img_template, CV_8U);
	imwrite(srt_out_path + "img_template(first frame).jpg", img_template);

	normalize(img_averaged, img_averaged, 0, 255, NORM_MINMAX);
	img_averaged.convertTo(img_averaged, CV_8U);
	imwrite(srt_out_path + "img_averaged(stabilized).jpg", img_averaged);
	cout << "Number of averaged frames = " << num_avr_frames << endl;

	waitKey(0);
	return 0;
}