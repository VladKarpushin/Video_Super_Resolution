#include <opencv2/opencv.hpp>

class ImresizeInFreqFilter
{
public:
	// input point of the filter
	// return value from Process: 0 - Success, !0 - Code of an error
	int Process(const cv::Mat& inputImg, cv::Mat& outputImg, int iScaleFactor);

private:
	// input point of the filter
	int ProcessMain(const cv::Mat& inputImg, cv::Mat& outputImg, int iScaleFactor);
	void filter2DFreq(const cv::Mat& inputImg, cv::Mat& outputImg, const cv::Mat& H);
	void imresizeInFreq(const cv::Mat& inputImg, cv::Mat& outputImg, int iScaleFactor);
};