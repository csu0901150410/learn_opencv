#pragma once

#include <opencv2/opencv.hpp>

void init_cascade_classifier();

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
};

extern GrayscaleProcessor g_grayscaleProcessor;
extern RecognizeProcessor g_recognizeProcessor;
