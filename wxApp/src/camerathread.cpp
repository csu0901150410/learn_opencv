#include "camerathread.h"

#include "convertmattowxbmp.h"
#include "frameprocessor.h"

// 定义事件
wxDEFINE_EVENT(EVT_CAMERA_CAPTURE_STARTED, CameraEvent);
wxDEFINE_EVENT(EVT_CAMERA_COMMAND_RESULT, CameraEvent);
wxDEFINE_EVENT(EVT_CAMERA_ERROR_OPEN, CameraEvent);
wxDEFINE_EVENT(EVT_CAMERA_ERROR_EMPTY, CameraEvent);
wxDEFINE_EVENT(EVT_CAMERA_ERROR_EXCEPTION, CameraEvent);

CameraFrameData::CameraFrameData(const wxString& cameraName, const wxULongLong frameNumber)
	: m_cameraName(cameraName)
	, m_frameNumber(frameNumber)
{
}

CameraFrameData::~CameraFrameData()
{
	// 释放位图
	if (m_frame)
		delete m_frame;
	if (m_thumbnail)
		delete m_thumbnail;
}

bool CameraSetupData::IsOK() const
{
	return !name.empty()
		&& !address.empty()
		&& defaultFPS > 0
		&& eventSink
		&& frames
		&& framesCS
		&& frameSize.GetWidth() >= 0
		&& frameSize.GetHeight() >= 0
		&& commands;
}

CameraThread::CameraThread(const CameraSetupData& cameraSetupData)
	: wxThread(wxTHREAD_JOINABLE)
	, m_cameraSetupData(cameraSetupData)
{
	wxCHECK_RET(m_cameraSetupData.IsOK(), "相机配置数据非法");
}

wxThread::ExitCode CameraThread::Entry()
{
	SetName(wxString::Format("CameraThread %s", GetCameraName()));

	wxLogTrace(TRACE_WXOPENCVCAMERAS, "Entered CameraThread for camera '%s', attempting to start capture...", GetCameraName());

	if (!InitCapture())
	{
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Failed to start capture for camera '%s'", GetCameraName());
		m_cameraSetupData.eventSink->QueueEvent(new CameraEvent(EVT_CAMERA_ERROR_OPEN, GetCameraName()));
		return static_cast<wxThread::ExitCode>(nullptr);
	}

	const bool createThumbnail = m_cameraSetupData.thumbnailSize.GetWidth() > 0 && m_cameraSetupData.thumbnailSize.GetHeight() > 0;
	
	CameraEvent* evt{nullptr};
	cv::Mat matFrame;
	wxStopWatch stopWatch;
	long msPerFrame;

	m_captureStartedTime = wxGetUTCTimeMillis();
	m_isCapturing = true;

	if (m_cameraSetupData.frameSize.GetWidth() > 0)
		SetCameraResolution(m_cameraSetupData.frameSize);
	if (m_cameraSetupData.useMJPGFourCC)
		SetCameraUseMJPEG();
	if (m_cameraSetupData.FPS > 0)
		SetCameraFPS(m_cameraSetupData.FPS);

	m_cameraSetupData.FPS = m_cameraCapture->get(static_cast<int>(cv::CAP_PROP_FPS));

	evt = new CameraEvent(EVT_CAMERA_CAPTURE_STARTED, GetCameraName());
	evt->SetString(wxString(m_cameraCapture->getBackendName()));
	evt->SetInt(m_cameraSetupData.FPS);
	m_cameraSetupData.eventSink->QueueEvent(evt);

	while (!TestDestroy())
	{
		try
		{
			CameraFrameDataPtr frameData(new CameraFrameData(GetCameraName(), m_frameCapturedCount++));
			wxLongLong frameCaptureStartedTime;
			CameraCommandData commandData;

			frameCaptureStartedTime = wxGetUTCTimeMillis();

			// 从命令消息队列取一条消息出来处理
			if (m_cameraSetupData.commands->ReceiveTimeout(0, commandData) == wxMSGQUEUE_NO_ERROR)
				ProcessCameraCommand(commandData);

			// 计算每帧时间
			if (m_cameraSetupData.FPS > 0)
				msPerFrame = 1000 / m_cameraSetupData.FPS;
			else
				msPerFrame = 1000 / m_cameraSetupData.defaultFPS;

			stopWatch.Start();
			(*m_cameraCapture) >> matFrame;
			frameData->SetTimeToRetrieve(stopWatch.Time());
			frameData->SetCapturedTime(wxGetUTCTimeMillis());

			if (!matFrame.empty())
			{
				// 转换成bitmap，相机帧处理可在此处进行
				stopWatch.Start();
				frameData->SetFrame(new wxBitmap(matFrame.cols, matFrame.rows, 24));
				ConvertMatBitmapTowxBitmap(matFrame, *frameData->GetFrame());
				frameData->SetTimeToConvert(stopWatch.Time());

				if (createThumbnail)
				{
					cv::Mat matThumbnail;

					stopWatch.Start();
					cv::Size size(m_cameraSetupData.thumbnailSize.GetWidth(), m_cameraSetupData.thumbnailSize.GetHeight());
					cv::resize(matFrame, matThumbnail, size);

					if (m_cameraSetupData.frameProcessorPtr)
						m_cameraSetupData.frameProcessorPtr->process(matThumbnail, matThumbnail);

					frameData->SetThumbnail(new wxBitmap(m_cameraSetupData.thumbnailSize, 24));
					ConvertMatBitmapTowxBitmap(matThumbnail, *frameData->GetThumbnail());
					frameData->SetTimeToCreateThumbnail(stopWatch.Time());
				}

				// 相机线程获取到的帧存到数组
				{
					wxCriticalSectionLocker locker(*m_cameraSetupData.framesCS);
					m_cameraSetupData.frames->push_back(std::move(frameData));
				}

				if (m_cameraSetupData.sleepDuration == CameraSetupData::SleepFromFPS)
				{
					// 根据fps决定睡眠时间
					const wxLongLong elapsed = wxGetUTCTimeMillis() - frameCaptureStartedTime;
					const long timeToSleep = msPerFrame - elapsed.GetLo();

					// 预期睡眠时间 - 帧处理时间 = 睡眠时间
					if (timeToSleep > 0)
						Sleep(timeToSleep);
				}
				else if (m_cameraSetupData.sleepDuration == CameraSetupData::SleepNone)
				{
					continue;
				}
				else if (m_cameraSetupData.sleepDuration > 0)
				{
					// 指定了其他的睡眠时间
					Sleep(m_cameraSetupData.sleepDuration);
				}
				else
				{
					wxLogDebug("Invalid sleep duration %d", m_cameraSetupData.sleepDuration);
				}
			}
			else
			{
				// 相机断开连接
				m_isCapturing = false;
				m_cameraSetupData.eventSink->QueueEvent(new CameraEvent(EVT_CAMERA_ERROR_EMPTY, GetCameraName()));
			}
		}
		catch (const std::exception& e)
		{
			m_isCapturing = false;
			evt = new CameraEvent(EVT_CAMERA_ERROR_EXCEPTION, GetCameraName());
			evt->SetString(e.what());
			m_cameraSetupData.eventSink->QueueEvent(evt);
			break;
		}
		catch (...)
		{
			m_isCapturing = false;
			evt = new CameraEvent(EVT_CAMERA_ERROR_EXCEPTION, GetCameraName());
			evt->SetString("Unknown exception");
			m_cameraSetupData.eventSink->QueueEvent(evt);
			break;
		}
	}

	wxLogTrace(TRACE_WXOPENCVCAMERAS, "Exiting CameraThread for camera '%s'...", GetCameraName());
	return static_cast<wxThread::ExitCode>(nullptr);
}

bool CameraThread::InitCapture()
{
	unsigned long cameraIndex = 0;

	if (m_cameraSetupData.address.ToCULong(&cameraIndex))
		m_cameraCapture.reset(new cv::VideoCapture(cameraIndex, m_cameraSetupData.apiPreference));
	else
		m_cameraCapture.reset(new cv::VideoCapture(m_cameraSetupData.address.ToStdString(), m_cameraSetupData.apiPreference));

	return m_cameraCapture->isOpened();
}

void CameraThread::SetCameraResolution(const wxSize& resolution)
{
	if (m_cameraCapture->set(cv::CAP_PROP_FRAME_WIDTH, resolution.GetWidth()))
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Set frame width to %d for camera '%s'", resolution.GetWidth(), GetCameraName());
	else
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Could not set frame width to %d for camera '%s'", resolution.GetWidth(), GetCameraName());

	if (m_cameraCapture->set(cv::CAP_PROP_FRAME_HEIGHT, resolution.GetHeight()))
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Set frame height to %d for camera '%s'", resolution.GetHeight(), GetCameraName());
	else
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Could not set frame height to %d for camera '%s'", resolution.GetHeight(), GetCameraName());
}

void CameraThread::SetCameraUseMJPEG()
{
	if (m_cameraCapture->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')))
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Set FourCC to 'MJPG' for camera '%s'", GetCameraName());
	else
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Could not set FourCC to 'MJPG' for camera '%s'", GetCameraName());
}

void CameraThread::SetCameraFPS(const int FPS)
{
	if (m_cameraCapture->set(cv::CAP_PROP_FPS, FPS))
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Set FPS to %d for camera '%s'", FPS, GetCameraName());
	else
		wxLogTrace(TRACE_WXOPENCVCAMERAS, "Could not set FPS to %d for camera '%s'", FPS, GetCameraName());
}

void CameraThread::ProcessCameraCommand(const CameraCommandData& commandData)
{
	CameraEvent* evt = new CameraEvent(EVT_CAMERA_COMMAND_RESULT, GetCameraName());
	CameraCommandData evtCommandData;

	evtCommandData.command = commandData.command;

	// ui发过来的命令，分支处理
	if (CameraCommandData::GetCameraInfo == commandData.command)
	{
		CameraCommandData::CameraInfo cameraInfo;

		cameraInfo.threadSleepDuration = m_cameraSetupData.sleepDuration;
		cameraInfo.captureStartedTime = m_captureStartedTime;
		cameraInfo.framesCapturedCount = m_frameCapturedCount;
		cameraInfo.cameraCaptureBackendName = m_cameraCapture->getBackendName();
		cameraInfo.cameraAddress = m_cameraSetupData.address;

		evtCommandData.parameter = cameraInfo;
	}
	//// 可以在这里处理其他命令
	//else if (xxx == commandData.command)
	//{
	//}
	else
	{
		delete evt;
		return;
	}

	evt->SetPayload(evtCommandData);
	m_cameraSetupData.eventSink->QueueEvent(evt);
}
