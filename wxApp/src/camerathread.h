#pragma once

#include <wx/wx.h>
#include <wx/msgqueue.h>
#include <wx/thread.h>
#include <opencv2/opencv.hpp>

class FrameProcessor;

// for wxLogTrace
#define TRACE_WXOPENCVCAMERAS "WXOPENCVCAMERAS"

// 相机命令数据，用于主线程和相机线程通讯
struct CameraCommandData
{
	// 用于cv::VideoCapture::get/set
	struct VCPropCommandParameter
	{
		int id{ 0 };
		double value{ 0 };
		bool succeeded{ false };// set返回值
	};
	typedef std::vector<VCPropCommandParameter> VCPropCommandParameters;

	struct CameraInfo
	{
		long threadSleepDuration{ 0 };
		wxLongLong captureStartedTime{ 0 };
		wxULongLong framesCapturedCount{ 0 };
		wxString cameraCaptureBackendName;
		wxString cameraAddress;
	};

	enum Commands
	{
		GetCameraInfo = 0,
		SetThreadSleepDuration,
		GetVCProp,
		SetVCProp,
	};

	Commands command;
	wxAny parameter;
};

typedef wxMessageQueue<CameraCommandData> CameraCommandDatas;


// 相机事件
class CameraEvent : public wxThreadEvent
{
public:
	CameraEvent(wxEventType eventType, const wxString& cameraName)
		: wxThreadEvent(eventType)
		, m_cameraName(cameraName)
	{
	}

	wxString GetCameraName() const { return m_cameraName; }

	CameraCommandData GetCommandResult() const { return GetPayload<CameraCommandData>(); }

	wxEvent* Clone() const override { return new CameraEvent(*this); }

protected:
	wxString m_cameraName;
};

// 声明事件
wxDECLARE_EVENT(EVT_CAMERA_CAPTURE_STARTED, CameraEvent);
wxDECLARE_EVENT(EVT_CAMERA_COMMAND_RESULT, CameraEvent);
wxDECLARE_EVENT(EVT_CAMERA_ERROR_OPEN, CameraEvent);
wxDECLARE_EVENT(EVT_CAMERA_ERROR_EMPTY, CameraEvent);
wxDECLARE_EVENT(EVT_CAMERA_ERROR_EXCEPTION, CameraEvent);



// 相机帧数据
class CameraFrameData
{
public:
	CameraFrameData(const wxString& cameraName, const wxULongLong frameNumber);
	~CameraFrameData();

	wxString GetCameraName() const { return m_cameraName; }

	wxBitmap* GetFrame() { return m_frame; }

	wxBitmap* GetThumbnail() { return m_thumbnail; }

	wxULongLong GetFrameNumber() const { return m_frameNumber; }

	long GetTimeToRetrieve() const { return m_timeToRetrieve; }

	long GetTimeToConvert() const { return m_timeToConvert; }

	long GetTimeToThumbnail() const { return m_timeToCreateThumbnail; }

	wxLongLong GetCapturedTime() const { return m_capturedTime; }


	void SetCameraName(const wxString& cameraName) { m_cameraName = cameraName; }

	void SetFrame(wxBitmap* frame) { m_frame = frame; }
	void SetThumbnail(wxBitmap* thumbnail) { m_thumbnail = thumbnail; }
	void SetFrameNumber(const wxULongLong number) { m_frameNumber = number; }
	
	void SetTimeToRetrieve(const long t) { m_timeToRetrieve = t; }
	void SetTimeToConvert(const long t) { m_timeToConvert = t; }
	void SetTimeToCreateThumbnail(const long t) { m_timeToCreateThumbnail = t; }
	void SetCapturedTime(const wxLongLong t) { m_capturedTime; }

private:
	wxString m_cameraName;
	wxBitmap* m_frame{nullptr};
	wxBitmap* m_thumbnail{nullptr};
	wxULongLong m_frameNumber{0};
	wxLongLong m_capturedTime{0};
	long m_timeToRetrieve{0};
	long m_timeToConvert{0};
	long m_timeToCreateThumbnail{0};
};

typedef std::unique_ptr<CameraFrameData> CameraFrameDataPtr;
typedef std::vector<CameraFrameDataPtr> CameraFrameDataPtrs;

// 相机配置数据
struct CameraSetupData
{
	// 相机线程在获取到一帧之后的睡眠时间枚举
	enum
	{
		SleepFromFPS = -1,// = (1000 / FPS) - 睡眠事件等于每帧处理的时间
		SleepNone = 0,
	};

	wxString name;
	wxString address;// 相机地址
	int apiPreference{cv::CAP_ANY};// see https://docs.opencv.org/4.10.0/d0/da7/videoio_overview.html
	long sleepDuration{SleepFromFPS};
	int FPS{0};
	int defaultFPS{25};
	bool useMJPGFourCC{false};

	wxEvtHandler* eventSink{nullptr};// 用于发送相机事件
	CameraFrameDataPtrs* frames{nullptr};// 相机帧数组
	wxCriticalSection* framesCS{nullptr};
	wxSize frameSize;
	wxSize thumbnailSize;

	CameraCommandDatas* commands{nullptr};

	FrameProcessor* frameProcessorPtr{nullptr};

	bool IsOK() const;
};

// 相机工作线程
class CameraThread : public wxThread
{
public:
	CameraThread(const CameraSetupData& cameraSetupData);

	wxString GetCameraAddress() const { return m_cameraSetupData.address; }
	wxString GetCameraName() const { return m_cameraSetupData.name; }
	bool IsCapturing() const { return m_isCapturing; }

protected:
	CameraSetupData m_cameraSetupData;

	std::unique_ptr<cv::VideoCapture> m_cameraCapture;
	std::atomic_bool m_isCapturing{false};
	wxLongLong m_captureStartedTime;
	wxULongLong m_frameCapturedCount{0};

	ExitCode Entry() override;

	bool InitCapture();
	void SetCameraResolution(const wxSize& resolution);
	void SetCameraUseMJPEG();
	void SetCameraFPS(const int FPS);

	void ProcessCameraCommand(const CameraCommandData& commandData);
};

