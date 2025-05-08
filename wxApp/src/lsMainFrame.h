#pragma once

#include <wx/wx.h>
#include <opencv2/opencv.hpp>

#include "wxSciterControl.h"

namespace cv
{
	class Mat;
	class VideoCapture;
}

class CameraThread;

class lsMainFrame : public wxFrame
{
public:
	wxSciterControl* m_sciter;

	lsMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~lsMainFrame();

	// If address is empty, the default webcam is used.
	// resolution and useMJPEG are used only for webcam.
	bool StartCameraCapture(const wxString& address,
		const wxSize& resolution = wxSize(),
		bool useMJPEG = false);
	bool StartCameraThread();
	void DeleteCameraThread();

	void Clear();

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& event);

	void OnWebCam(wxCommandEvent& event);

	void OnCameraFrame(wxThreadEvent& event);
	void OnCameraEmpty(wxThreadEvent& event);
	void OnCameraException(wxThreadEvent& event);

	static wxBitmap ConvertMatToBitmap(const cv::Mat& matBitmap, long& timeConvert);

	enum
	{
		ID_Quit = 1,
		ID_About,
		ID_Open,

		ID_WebCam,
	};

private:
	cv::VideoCapture* m_videoCapture{nullptr};
	CameraThread* m_cameraThread{nullptr};
};
