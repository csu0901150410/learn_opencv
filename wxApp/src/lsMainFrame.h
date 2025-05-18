#pragma once

#include <wx/wx.h>
#include <opencv2/opencv.hpp>

#include "wxSciterControl.h"
#include "camerathread.h"

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

	void AddCamera(const wxString& address);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& event);

	void OnCameraCaptureStarted(CameraEvent& event);
	void OnCameraCommandResult(CameraEvent& event);
	void OnCameraErrorOpen(CameraEvent& event);
	void OnCameraErrorEmpty(CameraEvent& event);
	void OnCameraErrorException(CameraEvent& event);

	void OnProcessNewCameraFrameData(wxTimerEvent& event);

	void OnTestCallScript(wxCommandEvent& event);

	enum
	{
		ID_Quit = 1,
		ID_About,
		ID_Open,

		ID_WebCam,

		ID_TestCallScript,
	};

private:

	struct CameraView
	{
		CameraThread* thread{nullptr};
		CameraCommandDatas* commandDatas{nullptr};
	};

	// 主窗口定时刷新相机画面
	static const long ms_defaultProcessNewCameraFrameDataInterval = 30;
	long m_processNewCameraFrameDataInteval{ms_defaultProcessNewCameraFrameDataInterval};
	wxTimer m_processNewCameraFrameDataTimer;

	std::map<wxString, CameraView> m_cameras;
	CameraFrameDataPtrs m_newCameraFrameData;
	wxCriticalSection m_newCameraFrameDataCS;

	long m_defaultCameraBackend{0};
	long m_defaultCameraThreadSleepDuration{CameraSetupData::SleepFromFPS};
	wxSize m_defaultCameraResolution;
	int m_defaultCameraFPS{0};
	bool m_defaultUseMJPGFourCC{false};

	void RemoveCamera(const wxString& cameraName);
	void RemoveAllCameras();
};
