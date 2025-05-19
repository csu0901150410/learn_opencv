#pragma once

#include <opencv2/opencv.hpp>

// 帧处理接口
class FrameProcessor
{
public:
	virtual void process(cv::Mat& input, cv::Mat& output) = 0;
};

// 灰度处理
class GrayscaleProcessor : public FrameProcessor
{
public:
	virtual void process(cv::Mat& input, cv::Mat& output) override;
};

// 简单的手势识别处理
class RecognizeProcessor : public FrameProcessor
{
public:
	virtual void process(cv::Mat& input, cv::Mat& output) override;

	RecognizeProcessor(const std::string& modelPath);

private:
	cv::Mat compensate_lighting(const cv::Mat& input);
	cv::Mat extract_skin(const cv::Mat& input);
	bool extract_hand_contour(const cv::Mat& mask, std::vector<cv::Point>& contour);
	bool extract_hu_moments(const std::vector<cv::Point>& contour, cv::Mat& feature);
	bool classify(const cv::Mat& feature, int& result);

	cv::Mat extract_hu_feature(const cv::Mat& input);

private:
	cv::Ptr<cv::ml::KNearest> knn;
};

extern GrayscaleProcessor g_grayscaleProcessor;
extern RecognizeProcessor g_recognizeProcessor;
