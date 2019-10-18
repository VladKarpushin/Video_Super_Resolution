// 2019-03-03 Video super resolution and camera stabilization
//

#include <iostream>
//#include <string>
#include "opencv2/opencv.hpp"
#include  "ImresizeInFreqFilter.hpp"
#include <windows.h>


using namespace cv;
using namespace std;

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

void GetFilesVec(LPCTSTR lpFileName, vector<string> & file_vec)
{
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	if ((hFind = FindFirstFile(lpFileName, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			printf("%s\n", FindFileData.cFileName);
			file_vec.push_back(FindFileData.cFileName);

		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}

int main() 
{
	string str_path_in = "D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\input\\new5\\";
	vector<string> file_vec;
	string str_mask = str_path_in + "*.jpg";
	GetFilesVec(str_mask.c_str(), file_vec);

	string srt_out_path = "D:\\home\\programming\\vc\\new\\6_My home projects\\11_video processing\\output\\";
	//VideoCapture cap(str_path_in + "DSC_0510.MOV");
	//if (!cap.isOpened())
	//{
	//	cout << "Error opening video stream or file" << endl;
	//	return -1;
	//}

	Mat img_frame;
	img_frame = imread(str_path_in + file_vec[0]);
	//cap >> img_frame;
	if (img_frame.empty())
	{
		cout << "img_frame.empty()" << endl;
		return -1;
	}
	cvtColor(img_frame, img_frame, COLOR_BGR2GRAY);

	// nikon siemens star
	const Rect roi_frame = Rect(Point2i(2000, 1100), Point2i(4379, 2972));
	img_frame = img_frame(roi_frame).clone();
	Rect roi_template = Rect(Point2i(870, 750), Point2i(1111, 1025));	// w and h should be even
	roi_template.width = roi_template.width & -2;
	roi_template.height = roi_template.height& -2;
	const int SCALE_FACTOR = 4;
	const int MAX_OBJ_OFFSET = 300 * SCALE_FACTOR; // max allowed radius of object offset MAX_OBJ_OFFSET = 100
	const int MAX_FRAMES = file_vec.size();	// max number of averaged frames

	const Rect roi_template_new = Rect(roi_template.tl()*SCALE_FACTOR, roi_template.size()*SCALE_FACTOR);
	Mat img_template = img_frame(roi_template).clone();
	ImresizeInFreqFilter filter;
	filter.Process(img_template, img_template, SCALE_FACTOR);
	img_template.convertTo(img_template, CV_32F);
	
	Mat img_averaged = img_template.clone();	// superresolution image
	//int i = 0;
	int num_avr_frames = 0;
	//while (num_avr_frames < MAX_FRAMES)
	for(int i = 1; i < MAX_FRAMES; i++)
	{
		//i++;
		//cap >> img_frame;
		img_frame = imread(str_path_in + file_vec[i]);
		if (img_frame.empty())
			break;
		
		//std::string s_frame = std::to_string(num_avr_frames);
		//imwrite(srt_out_path + "img_frame_" + s_frame + ".jpg", img_frame);

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
		cout << "frame number: " << i;
		cout << "\t maxLoc = " << maxLoc;
		cout << "\t maxLoc - offset = " << offset << endl;
	}
	//cap.release();

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