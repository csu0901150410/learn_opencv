#include "frameprocessor.h"

GrayscaleProcessor g_grayscaleProcessor;
RecognizeProcessor g_recognizeProcessor;

void GrayscaleProcessor::process(cv::Mat& input, cv::Mat& output)
{
	cv::Mat grayImg;
	cv::cvtColor(input, grayImg, cv::COLOR_BGR2GRAY);
	cv::cvtColor(grayImg, output, cv::COLOR_GRAY2BGR);
}

struct global_cascade_classifier
{
	cv::CascadeClassifier rpalm;
	cv::CascadeClassifier fist;
	cv::CascadeClassifier right;

	bool initial{false};
};

global_cascade_classifier gCascadeClassifier;

void init_cascade_classifier()
{
	if (!gCascadeClassifier.rpalm.load("../resources/model/rpalm.xml"))
		printf("--(!)Error loading rpalm\n");
	if (!gCascadeClassifier.fist.load("../resources/model/fist.xml"))
		printf("--(!)Error loading fist\n");
	if (!gCascadeClassifier.right.load("../resources/model/right.xml"))
		printf("--(!)Error loading right\n");
	gCascadeClassifier.initial = true;
}

#include <omp.h>

void RecognizeProcessor::process(cv::Mat& input, cv::Mat& output)
{
	if (!gCascadeClassifier.initial)
		init_cascade_classifier();

	cv::TickMeter tm;
	tm.start();

	int time_count1 = 0;
	int time_count2 = 0;

	output = input;

	double scaleFactor = 1.2;
	int minNeighbors = 3;
	cv::Size minSize(60, 60);

	cv::Mat smallImg, grayImg;
	//cv::resize(input, smallImg, cv::Size(640, 360));
	cv::cvtColor(input, grayImg, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(grayImg, grayImg);

	std::vector<cv::Rect> rpalm, fists, rect_right;

	//detector for right palm
	cv::CascadeClassifier& rpalm_cascade = gCascadeClassifier.rpalm;
	rpalm_cascade.detectMultiScale(grayImg, rpalm, scaleFactor, minNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, minSize);

	//detector for fist
	cv::CascadeClassifier& fist_cascade = gCascadeClassifier.fist;
	fist_cascade.detectMultiScale(grayImg, fists, scaleFactor, minNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, minSize);

	//detecting largest contour for gesture detection
	cv::CascadeClassifier& right_cascade = gCascadeClassifier.right;
	right_cascade.detectMultiScale(grayImg, rect_right, scaleFactor, minNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, minSize);

//#pragma omp parallel sections
//	{
//#pragma omp section
//		{
//			//detector for right palm
//			cv::CascadeClassifier& rpalm_cascade = gCascadeClassifier.rpalm;
//			rpalm_cascade.detectMultiScale(grayImg, rpalm, scaleFactor, minNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, minSize);
//		}
//
//#pragma omp section
//		{
//			//detector for fist
//			cv::CascadeClassifier& fist_cascade = gCascadeClassifier.fist;
//			fist_cascade.detectMultiScale(grayImg, fists, scaleFactor, minNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, minSize);
//		}
//
//#pragma omp section
//		{
//			//detecting largest contour for gesture detection
//			cv::CascadeClassifier& right_cascade = gCascadeClassifier.right;
//			right_cascade.detectMultiScale(grayImg, rect_right, scaleFactor, minNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, minSize);
//		}
//	}

	// 绿色，手掌
	cv::Rect big_rpalm;
	if (rpalm.size() != NULL)
	{
		big_rpalm = rpalm[0];
	}
	for (int i = 0; i < rpalm.size(); i++)
	{
		if ((rpalm[i].width * rpalm[i].height) > (big_rpalm.width * big_rpalm.height))// detecting biggest rect
		{
			big_rpalm = rpalm[i];
		}
		cv::Point center(rpalm[i].x + rpalm[i].width * 0.5, rpalm[i].y + rpalm[i].height * 0.5);
		cv::rectangle(output, rpalm[i], cv::Scalar(0, 255, 0), 2);
	}

	// 红色，拳头
	cv::Rect big_fist;
	if (fists.size() != NULL)
	{
		big_fist = fists[0];
	}
	for (int i = 0; i < fists.size(); i++)
	{
		if ((fists[i].width * fists[i].height) > (big_fist.width * big_fist.height))//detecting biggest rect
		{
			big_fist = fists[i];
		}
		cv::Point center(fists[i].x + fists[i].width * 0.5, fists[i].y + fists[i].height * 0.5);
		cv::rectangle(output, fists[i], cv::Scalar(255, 0, 0), 2);
	}

	// 蓝色，向右
	cv::Rect big_right;
	if (rect_right.size() != NULL)
	{
		big_right = rect_right[0];
	}
	for (int i = 0; i < rect_right.size(); i++)
	{
		cv::Point center(rect_right[i].x + rect_right[i].width * 0.5, rect_right[i].y + rect_right[i].height * 0.5);
		cv::rectangle(output, rect_right[i], cv::Scalar(0, 0, 255), 2);
	}

	tm.stop();
	std::cout << "当前帧耗时 : " << tm.getTimeMilli() << " ms" << std::endl;
}
