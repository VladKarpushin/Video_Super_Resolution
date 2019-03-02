#include "ImresizeInFreqFilter.hpp"

using namespace cv;
using namespace std;

void ImresizeInFreqFilter::filter2DFreq(const Mat& inputImg, Mat& outputImg, const Mat& H)
{
	Mat planes[2] = {Mat_<float>(inputImg.clone()), Mat::zeros(inputImg.size(), CV_32F)};
	Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI, DFT_SCALE);

	Mat planesH[2] = {Mat_<float>(H.clone()), Mat::zeros(H.size(), CV_32F)};
	Mat complexH;
	merge(planesH, 2, complexH);
	Mat complexIH;
	mulSpectrums(complexI, complexH, complexIH, 0);

	idft(complexIH, complexIH);
	split(complexIH, planes);
	outputImg = planes[0];
}

// 	iScaleFactor - scale factor along the horizontal axis
void ImresizeInFreqFilter::imresizeInFreq(const Mat& inputImg, Mat& outputImg, int iScaleFactor)
{
	Mat planes[2] = { Mat_<float>(inputImg.clone()), Mat::zeros(inputImg.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI, DFT_SCALE);

	Size dsize(inputImg.size()*iScaleFactor);
	outputImg = Mat(dsize, CV_32F, Scalar(0));

	int cx = inputImg.cols / 2;
	int cy = inputImg.rows / 2;
	Rect roi0 = Rect(0, 0, cx, cy);
	Rect roi1 = Rect(cx, 0, cx, cy);
	Rect roi2 = Rect(0, cy, cx, cy);
	Rect roi3 = Rect(cx, cy, cx, cy);

	Rect roi1New = Rect(dsize.width - cx, 0, cx, cy);
	Rect roi2New = Rect(0, dsize.height - cy, cx, cy);
	Rect roi3New = Rect(dsize.width - cx, dsize.height - cy, cx, cy);

	Mat complexINew = Mat(dsize, complexI.type(), Scalar(0));
	complexI(roi0).copyTo(complexINew(roi0));
	complexI(roi1).copyTo(complexINew(roi1New));
	complexI(roi2).copyTo(complexINew(roi2New));
	complexI(roi3).copyTo(complexINew(roi3New));

	idft(complexINew, complexINew);
	split(complexINew, planes);
	outputImg = planes[0];

	// filterrring check (start)
	Mat abs0 = abs(planes[0]);
	Mat abs1 = abs(planes[1]);
	double MaxReal, MinReal;
	minMaxLoc(abs0, &MinReal, &MaxReal, NULL, NULL);
	double MaxIm, MinIm;
	minMaxLoc(abs1, &MinIm, &MaxIm, NULL, NULL);
	//cout << "MaxReal = " << MaxReal << "; MinReal = " << MinReal << endl;
	//cout << "MaxIm = " << MaxIm << "; MinIm = " << MinIm << endl;
	// filterrring check (stop)
}
//! [imresizeInFreq]

int ImresizeInFreqFilter::ProcessMain(const Mat& inputImg, Mat& outputImg, int iScaleFactor)
{
	// it needs to process even image only
	Rect roi = Rect(0, 0, inputImg.cols & -2, inputImg.rows & -2);
	Mat imgIn = inputImg(roi);

	imgIn.convertTo(imgIn, CV_32F);
	imresizeInFreq(imgIn, outputImg, iScaleFactor);
	//outputImg.convertTo(outputImg, CV_8U);
	return 0;
}

// return value from Process: 0 - Success, !0 - Code of an error
int ImresizeInFreqFilter::Process(const Mat& inputImg, Mat& outputImg, int iScaleFactor)
{
	int flag = 0;
	try
	{
		flag = ProcessMain(inputImg, outputImg, iScaleFactor);
	}
	catch (const Exception& e)
	{
		return e.code;
	}
	return flag;
}