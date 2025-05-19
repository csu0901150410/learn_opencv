#include "lsMainFrame.h"

#include <wx/choicdlg.h>

#include <opencv2/ml.hpp>

#include "convertmattowxbmp.h"
#include "camerathread.h"
#include "frameprocessor.h"

lsMainFrame::lsMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame*)NULL, -1, title, pos, size)
	, m_sciter()
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Open, "&Open...", "Load html to sciter window.");
	menuFile->Append(ID_About, "&About...");
	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit, "E&xit", "Quit application.");

	wxMenu* menuView = new wxMenu;
	menuView->Append(ID_WebCam, "&WebCam...", "Open WebCam.");

	wxMenu* menuTest = new wxMenu;
	menuTest->Append(ID_TestCallScript, "Test call script...");
	menuTest->Append(ID_TrainModel, "Train model ...");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuView, "&View");
	menuBar->Append(menuTest, "&Test");

	SetMenuBar(menuBar);


	//wxFontSelectorCtrl* ctl = new wxFontSelectorCtrl(this, this->NewControlId());
	m_sciter = new wxSciterControl(this, this->NewControlId());
	//m_sciter->load_file(L"http://httpbin.org/html");
	m_sciter->load_file(L"home://../resources/wxhtml/main.htm");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_sciter, 1, wxEXPAND);
	SetSizer(sizer);

	// 应用布局
	Layout();

	CreateStatusBar();
	SetStatusText("Welcome to wxWindows!");

	Centre();

	Bind(wxEVT_MENU, &lsMainFrame::OnQuit, this, ID_Quit);
	Bind(wxEVT_MENU, &lsMainFrame::OnOpen, this, ID_Open);
	Bind(wxEVT_MENU, &lsMainFrame::OnAbout, this, ID_About);
	Bind(wxEVT_PAINT, &lsMainFrame::OnPaint, this);

	Bind(wxEVT_MENU, [this](wxCommandEvent&) { AddCamera("0"); }, ID_WebCam);

	Bind(wxEVT_MENU, &lsMainFrame::OnTestCallScript, this, ID_TestCallScript);
	Bind(wxEVT_MENU, &lsMainFrame::OnTrainModel, this, ID_TrainModel);


	m_processNewCameraFrameDataTimer.Start(m_processNewCameraFrameDataInteval);
	m_processNewCameraFrameDataTimer.Bind(wxEVT_TIMER, &lsMainFrame::OnProcessNewCameraFrameData, this);

	// 相机事件
	Bind(EVT_CAMERA_CAPTURE_STARTED, &lsMainFrame::OnCameraCaptureStarted, this);
	Bind(EVT_CAMERA_COMMAND_RESULT, &lsMainFrame::OnCameraCommandResult, this);
	Bind(EVT_CAMERA_ERROR_OPEN, &lsMainFrame::OnCameraErrorEmpty, this);
	Bind(EVT_CAMERA_ERROR_EMPTY, &lsMainFrame::OnCameraErrorEmpty, this);
	Bind(EVT_CAMERA_ERROR_EXCEPTION, &lsMainFrame::OnCameraErrorException, this);

	wxLog::AddTraceMask(TRACE_WXOPENCVCAMERAS);
}

lsMainFrame::~lsMainFrame()
{
	RemoveAllCameras();
}

void lsMainFrame::AddCamera(const wxString& address)
{
	const wxSize thumbnailSize = wxSize(640, 480);
	static int newCameraId = 0;

	CameraView cameraView;
	wxString cameraName = wxString::Format("CAM #%d", 1);
	CameraSetupData cameraSetupData;

	cameraSetupData.name = cameraName;
	cameraSetupData.address = address;
	cameraSetupData.apiPreference = m_defaultCameraBackend;
	cameraSetupData.sleepDuration = m_defaultCameraThreadSleepDuration;
	cameraSetupData.frameSize = m_defaultCameraResolution;
	cameraSetupData.FPS = m_defaultCameraFPS;
	cameraSetupData.useMJPGFourCC = m_defaultUseMJPGFourCC;

	cameraSetupData.eventSink = this;
	cameraSetupData.frames = &m_newCameraFrameData;
	cameraSetupData.framesCS = &m_newCameraFrameDataCS;
	cameraSetupData.thumbnailSize = thumbnailSize;
	//cameraSetupData.frameProcessorPtr = &g_grayscaleProcessor;
	cameraSetupData.frameProcessorPtr = &g_recognizeProcessor;

	cameraSetupData.commands = new CameraCommandDatas;

	cameraView.thread = new CameraThread(cameraSetupData);
	cameraView.commandDatas = cameraSetupData.commands;

	m_cameras[cameraName] = cameraView;

	if (cameraView.thread->Run() != wxTHREAD_NO_ERROR)
		wxLogError("Could not create the worker thread needed to retrieve the images from camera '%s'.", cameraName);
}

void lsMainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void lsMainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("This is a wxWindows + Sciter child control sample", "About", wxOK | wxICON_INFORMATION, this);
}

void lsMainFrame::OnOpen(wxCommandEvent& event)
{
	wxFileDialog dlg(this);
	dlg.SetWildcard("HTML files|*.htm;*.html|All files|*.*");
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	const wxString path = "file:///" + dlg.GetPath();
	m_sciter->load_file(path.ToStdWstring().c_str());
	this->SetStatusText(dlg.GetPath());
}

void lsMainFrame::OnPaint(wxPaintEvent& event)
{
	int a = 100;
}

void lsMainFrame::OnCameraCaptureStarted(CameraEvent& event)
{
	wxLogTrace(TRACE_WXOPENCVCAMERAS, "Started capturing from camera '%s' (fps: %s, backend: %s)'.",
		event.GetCameraName(),
		event.GetInt() ? wxString::Format("%d", event.GetInt()) : "n/a",
		event.GetString());
}

void lsMainFrame::OnCameraCommandResult(CameraEvent& event)
{
	int a = 100;
}

void lsMainFrame::OnCameraErrorOpen(CameraEvent& event)
{
	int a = 100;
}

void lsMainFrame::OnCameraErrorEmpty(CameraEvent& event)
{
	int a = 100;
}

void lsMainFrame::OnCameraErrorException(CameraEvent& event)
{
	int a = 100;
}

void lsMainFrame::OnProcessNewCameraFrameData(wxTimerEvent& event)
{
	CameraFrameDataPtrs frameData;
	wxStopWatch stopWatch;

	stopWatch.Start();

	{
		wxCriticalSectionLocker locker(m_newCameraFrameDataCS);
		if (m_newCameraFrameData.empty())
			return;
		frameData = std::move(m_newCameraFrameData);
	}

	for (const auto& fd : frameData)
	{
		const wxString cameraName = fd->GetCameraName();
		auto it = m_cameras.find(cameraName);

		if (m_cameras.end() == it || !it->second.thread->IsCapturing())
			continue;

		//const wxBitmap* cameraFrame = fd->GetFrame();
		const wxBitmap* cameraFrameThumbnail = fd->GetThumbnail();

		if (cameraFrameThumbnail && cameraFrameThumbnail->IsOk())
		{
			// 更新所有wxOcvWindow为获取的位图
			m_sciter->update_ocvwindow(*cameraFrameThumbnail);
		}
		else
		{
			// 更新所有wxOcvWindow为空位图
			m_sciter->update_ocvwindow(wxBitmap());
		}
	}

	//wxLogTrace(TRACE_WXOPENCVCAMERAS, "Processed %zu new camera frames in %ld ms.", frameData.size(), stopWatch.Time());
	frameData.clear();
}

extern SCITER_VALUE call_script(const std::string& name, const Json::Value& params);

// C++调用脚本函数
Json::Value call_script_demo(const Json::Value& params = Json::Value::null)
{
	Json::Value jsparams;
	jsparams["text"] = "native call script success ...";
	call_script("script_function_for_native_call", jsparams);

	return Json::Value();
}
REGISTER_FUNCTION("call_script_demo", call_script_demo);

void lsMainFrame::OnTestCallScript(wxCommandEvent& event)
{
	call_script_demo();
}

void lsMainFrame::OnTrainModel(wxCommandEvent& event)
{
	auto extractHuFeature = [](const cv::Mat& src) {
		cv::Mat ycrcb, mask;
		cv::cvtColor(src, ycrcb, cv::COLOR_BGR2YCrCb);
		cv::inRange(ycrcb, cv::Scalar(0, 133, 77), cv::Scalar(255, 173, 127), mask);
		cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
		cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		double maxArea = 0;
		int maxIdx = -1;
		for (size_t i = 0; i < contours.size(); ++i) {
			double area = contourArea(contours[i]);
			if (area > maxArea) {
				maxArea = area;
				maxIdx = static_cast<int>(i);
			}
		}

		if (maxIdx == -1)
			return cv::Mat();

		cv::Moments m = cv::moments(contours[maxIdx]);
		double hu[7];
		cv::HuMoments(m, hu);
		cv::Mat feature(1, 7, CV_32F);
		for (int i = 0; i < 7; ++i) {
			feature.at<float>(0, i) = -1.0f * static_cast<float>(copysign(log10(abs(hu[i]) + 1e-10), hu[i]));
		}
		return feature;
	};

	auto loadImages = [extractHuFeature](const std::string& folderPath, int label, std::vector<cv::Mat>& features, std::vector<int>& labels) {
		std::vector<cv::String> filenames;
		cv::glob(folderPath, filenames, false);

		for (size_t i = 0; i < filenames.size(); ++i) {
			cv::Mat img = cv::imread(filenames[i]);
			if (img.empty())
				continue;

			cv::Mat feat = extractHuFeature(img);
			if (!feat.empty()) {
				features.push_back(feat);
				labels.push_back(label);
			}
		}
	};

	std::vector<cv::Mat> features;
    std::vector<int> labels;

    // 按实际路径修改这两个文件夹路径
    std::string palmPath = "../resources/dataset/palms/*.bmp";
    std::string fistPath = "../resources/dataset/fists/*.bmp";

    loadImages(palmPath, 0, features, labels);
    loadImages(fistPath, 1, features, labels);

    if (features.empty()) {
        return;
    }

    cv::Mat trainData;
    cv::vconcat(features, trainData);
    cv::Mat responses = cv::Mat(labels).reshape(1, labels.size());

    cv::Ptr<cv::ml::KNearest> knn = cv::ml::KNearest::create();
    knn->setDefaultK(3);
	knn->setIsClassifier(true);

	knn->train(trainData, cv::ml::ROW_SAMPLE, responses);
    knn->save("../resources/model/gesture_model.xml");
}

void lsMainFrame::RemoveCamera(const wxString& cameraName)
{
	auto it = m_cameras.find(cameraName);
	wxCHECK_RET(it != m_cameras.end(), wxString::Format("Camera '%s' not found, could not be deleted.", cameraName));

	wxLogTrace(TRACE_WXOPENCVCAMERAS, "Removing camera '%s'...", cameraName);
	it->second.thread->Delete(nullptr, wxTHREAD_WAIT_BLOCK);
	delete it->second.thread;
	delete it->second.commandDatas;
	m_cameras.erase(it);
	wxLogTrace(TRACE_WXOPENCVCAMERAS, "Removed camera '%s'.", cameraName);
}

void lsMainFrame::RemoveAllCameras()
{
	while (!m_cameras.empty())
		RemoveCamera(wxString(m_cameras.begin()->first));

	m_cameras.clear();
}
