#include "frameprocessor.h"

GrayscaleProcessor g_grayscaleProcessor;
RecognizeProcessor g_recognizeProcessor("../resources/model/gesture_model.xml");

void GrayscaleProcessor::process(cv::Mat& input, cv::Mat& output)
{
	cv::Mat grayImg;
	cv::cvtColor(input, grayImg, cv::COLOR_BGR2GRAY);
	cv::cvtColor(grayImg, output, cv::COLOR_GRAY2BGR);
}

void RecognizeProcessor::process(cv::Mat& input, cv::Mat& output)
{
	output = input.clone();

	cv::TickMeter tm;
	tm.start();

	cv::Mat compensated = compensate_lighting(input);
	cv::Mat skinMask = extract_skin(compensated);

	std::vector<cv::Point> handContour;
	if (!extract_hand_contour(skinMask, handContour))
		return;

	cv::Mat colorOverlay(input.size(), input.type(), cv::Scalar(0, 0, 255));
	colorOverlay.copyTo(output, skinMask);

	cv::drawContours(output, std::vector<std::vector<cv::Point>>{handContour}, -1, cv::Scalar(0, 255, 0), 2);


	/*
	cv::Mat compensated = compensate_lighting(input);
	cv::Mat skinMask = extract_skin(compensated);
	std::vector<cv::Point> handContour;
	if (!extract_hand_contour(skinMask, handContour))
		return;
	cv::Mat huFeature;
	if (!extract_hu_moments(handContour, huFeature))
		return;

	int result;
	if (!classify(huFeature, result))
		return;
	*/

	/*cv::Mat feature = extract_hu_feature(input);
	if (!feature.empty())
	{
		double response = knn->findNearest(feature, 3, cv::noArray());
		int result = static_cast<int>(response);
		int a = 100;
	}*/

	tm.stop();
	std::cout << "当前帧耗时 : " << tm.getTimeMilli() << " ms" << std::endl;
}

RecognizeProcessor::RecognizeProcessor(const std::string& modelPath)
{
	knn = cv::ml::KNearest::load(modelPath);
}

cv::Mat RecognizeProcessor::compensate_lighting(const cv::Mat& input)
{
	// 光照补偿，对Y通道进行直方图均衡化，增强亮度对比
	cv::Mat ycrcb;
	cv::cvtColor(input, ycrcb, cv::COLOR_BGR2YCrCb);
	std::vector<cv::Mat> channels;
	cv::split(ycrcb, channels);
	cv::equalizeHist(channels[0], channels[0]);
	cv::merge(channels, ycrcb);
	cv::Mat output;
	cv::cvtColor(ycrcb, output, cv::COLOR_YCrCb2BGR);
	return output;
}

cv::Mat RecognizeProcessor::extract_skin(const cv::Mat& input)
{
	cv::Mat ycrcb, mask;
	cv::cvtColor(input, ycrcb, cv::COLOR_BGR2YCrCb);
	cv::inRange(ycrcb, cv::Scalar(0, 133, 77), cv::Scalar(255, 173, 127), mask);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
	return mask;
}

bool RecognizeProcessor::extract_hand_contour(const cv::Mat& mask, std::vector<cv::Point>& contour)
{
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	double maxArea = 0;
	int maxIndex = -1;
	for (int i = 0; i < contours.size(); ++i)
	{
		double area = cv::contourArea(contours[i]);
		if (area > maxArea)
		{
			maxArea = area;
			maxIndex = i;
		}
	}
	if (maxIndex >= 0)
	{
		contour = contours[maxIndex];
		return true;
	}
	return false;
}

bool RecognizeProcessor::extract_hu_moments(const std::vector<cv::Point>& contour, cv::Mat& feature)
{
	if (contour.empty())
		return false;
	cv::Moments m = cv::moments(contour);
	double hu[7];
	cv::HuMoments(m, hu);
	feature = cv::Mat(1, 7, CV_32F);
	for (int i = 0; i < 7; ++i)
	{
		feature.at<float>(0, i) = -1.0f * static_cast<float>(copysign(log10(std::abs(hu[i]) + 1e-10), hu[i]));
	}
	return true;
}

bool RecognizeProcessor::classify(const cv::Mat& feature, int& result)
{
	if (feature.empty())
		return false;
	result = static_cast<int>(knn->predict(feature));
	return true;
}

cv::Mat RecognizeProcessor::extract_hu_feature(const cv::Mat& input)
{
	using namespace cv;
	using namespace std;

	Mat ycrcb, mask;
	cvtColor(input, ycrcb, COLOR_BGR2YCrCb);
	inRange(ycrcb, Scalar(0, 133, 77), Scalar(255, 173, 127), mask);
	morphologyEx(mask, mask, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	morphologyEx(mask, mask, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	vector<vector<Point>> contours;
	findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	double maxArea = 0;
	int maxIdx = -1;
	for (size_t i = 0; i < contours.size(); ++i) {
		double area = contourArea(contours[i]);
		if (area > maxArea) {
			maxArea = area;
			maxIdx = static_cast<int>(i);
		}
	}

	if (maxIdx == -1) return Mat();

	Moments m = moments(contours[maxIdx]);
	double hu[7];
	HuMoments(m, hu);
	Mat feature(1, 7, CV_32F);
	for (int i = 0; i < 7; ++i) {
		feature.at<float>(0, i) = -1.0f * static_cast<float>(copysign(log10(abs(hu[i]) + 1e-10), hu[i]));
	}
	return feature;
}
