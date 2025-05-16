#include "lsMainFrame.h"

#include <wx/choicdlg.h>

#include "convertmattowxbmp.h"

// A frame was retrieved from WebCam or IP Camera.
wxDEFINE_EVENT(wxEVT_CAMERA_FRAME, wxThreadEvent);
// Could not retrieve a frame, consider connection to the camera lost.
wxDEFINE_EVENT(wxEVT_CAMERA_EMPTY, wxThreadEvent);
// An exception was thrown in the camera thread.
wxDEFINE_EVENT(wxEVT_CAMERA_EXCEPTION, wxThreadEvent);


class CameraThread : public wxThread
{
public:
	struct CameraFrame
	{
		cv::Mat matBitmap;
		long timeGet{ 0 };
	};

	CameraThread(wxEvtHandler* eventSink, cv::VideoCapture* camera);

protected:
	wxEvtHandler* m_eventSink{ nullptr };
	cv::VideoCapture* m_camera{ nullptr };

	ExitCode Entry() override;
};

CameraThread::CameraThread(wxEvtHandler* eventSink, cv::VideoCapture* camera)
	: wxThread(wxTHREAD_JOINABLE)
	, m_eventSink(eventSink)
	, m_camera(camera)
{
	wxASSERT(m_eventSink);
	wxASSERT(m_camera);
}

wxThread::ExitCode CameraThread::Entry()
{
	wxStopWatch stopWatch;
	while (!TestDestroy())
	{
		CameraFrame* frame = nullptr;

		try
		{
			frame = new CameraFrame;
			stopWatch.Start();
			(*m_camera) >> frame->matBitmap;
			frame->timeGet = stopWatch.Time();

			if (!frame->matBitmap.empty())
			{
				wxThreadEvent* evt = new wxThreadEvent(wxEVT_CAMERA_FRAME);
				evt->SetPayload(frame);
				m_eventSink->QueueEvent(evt);
				wxMilliSleep(30);
			}
			else
			{
				m_eventSink->QueueEvent(new wxThreadEvent(wxEVT_CAMERA_EMPTY));
				wxDELETE(frame);
				break;
			}
		}
		catch (const std::exception& e)
		{
			wxThreadEvent* evt = new wxThreadEvent(wxEVT_CAMERA_EXCEPTION);
			wxDELETE(frame);
			evt->SetString(e.what());
			m_eventSink->QueueEvent(evt);
			break;
		}
		catch (...)
		{
			wxThreadEvent* evt = new wxThreadEvent(wxEVT_CAMERA_EXCEPTION);
			wxDELETE(frame);
			evt->SetString("Unknown exception");
			m_eventSink->QueueEvent(evt);
			break;
		}
	}

	return static_cast<wxThread::ExitCode>(nullptr);
}

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

	Bind(wxEVT_MENU, &lsMainFrame::OnWebCam, this, ID_WebCam);

	Bind(wxEVT_MENU, &lsMainFrame::OnTestCallScript, this, ID_TestCallScript);

	Bind(wxEVT_CAMERA_FRAME, &lsMainFrame::OnCameraFrame, this);
	Bind(wxEVT_CAMERA_EMPTY, &lsMainFrame::OnCameraEmpty, this);
	Bind(wxEVT_CAMERA_EXCEPTION, &lsMainFrame::OnCameraException, this);
}

lsMainFrame::~lsMainFrame()
{
	DeleteCameraThread();
}

bool lsMainFrame::StartCameraCapture(const wxString& address, 
	const wxSize& resolution /*= wxSize()*/, 
	bool useMJPEG /*= false*/)
{
	const bool isDefaultWebCam = address.empty();
	cv::VideoCapture* cap = nullptr;

	Clear();

	{
		wxWindowDisabler disabler;
		wxBusyCursor busyCursor;

		if (isDefaultWebCam)
			cap = new cv::VideoCapture(0);
		else
			cap = new cv::VideoCapture(address.ToStdString());
	}

	if (!cap->isOpened())
	{
		delete cap;
		wxLogError("Could not connect to the camera.");
		return false;
	}

	m_videoCapture = cap;

	if (isDefaultWebCam)
	{
		m_videoCapture->set(cv::CAP_PROP_FRAME_WIDTH, resolution.GetWidth());
		m_videoCapture->set(cv::CAP_PROP_FRAME_HEIGHT, resolution.GetHeight());
		if (useMJPEG)
			m_videoCapture->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
	}

	if (!StartCameraThread())
	{
		Clear();
		return false;
	}

	return true;
}

bool lsMainFrame::StartCameraThread()
{
	DeleteCameraThread();

	m_cameraThread = new CameraThread(this, m_videoCapture);
	if (wxTHREAD_NO_ERROR != m_cameraThread->Run())
	{
		wxDELETE(m_cameraThread);
		wxLogError("Could not create the thread needed to retrieve the images from a camera.");
		return false;
	}

	return true;
}

void lsMainFrame::DeleteCameraThread()
{
	if (m_cameraThread)
	{
		m_cameraThread->Delete(nullptr, wxTHREAD_WAIT_BLOCK);
		wxDELETE(m_cameraThread);
	}
}

void lsMainFrame::Clear()
{
	DeleteCameraThread();

	if (m_videoCapture)
		wxDELETE(m_videoCapture);
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

void lsMainFrame::OnWebCam(wxCommandEvent& event)
{
	static const wxSize resolutions[] = {
		{320, 240},
		{640, 480},
		{800, 600},
		{1024, 576},
		{1280, 720},
		{1920, 1080}
	};
	static int resolutionIndex = 1;
	wxArrayString resolutionStrings;
	bool useMJPEG = false;

	for (const auto& r : resolutions)
		resolutionStrings.push_back(wxString::Format("%d x %d", r.GetWidth(), r.GetHeight()));

	resolutionIndex = wxGetSingleChoiceIndex("Select resolution", "WebCam",
		resolutionStrings, resolutionIndex, this);
	if (-1 == resolutionIndex)
		return;

	if (StartCameraCapture(wxEmptyString, resolutions[resolutionIndex], useMJPEG))
	{
		// 相机打开成功
		int a = 100;
	}
}

void lsMainFrame::OnCameraFrame(wxThreadEvent& event)
{
	CameraThread::CameraFrame* frame = event.GetPayload<CameraThread::CameraFrame*>();

	long timeConvert = 0;
	wxBitmap bitmap = ConvertMatToBitmap(frame->matBitmap, timeConvert);

	if (bitmap.IsOk())
	{
		// 更新所有wxOcvWindow为获取的位图
		m_sciter->update_ocvwindow(bitmap);
	}
	else
	{
		// 更新所有wxOcvWindow为空位图
		m_sciter->update_ocvwindow(wxBitmap());
	}

	delete frame;
}

void lsMainFrame::OnCameraEmpty(wxThreadEvent& event)
{
	wxLogError("Connection to the camera lost.");
	Clear();
}

void lsMainFrame::OnCameraException(wxThreadEvent& event)
{
	wxLogError("Exception in the camera thread: %s", event.GetString());
	Clear();
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

wxBitmap lsMainFrame::ConvertMatToBitmap(const cv::Mat& matBitmap, long& timeConvert)
{
	wxCHECK(!matBitmap.empty(), wxBitmap());

	wxBitmap bitmap(matBitmap.cols, matBitmap.rows, 24);
	bool converted = false;
	wxStopWatch stopWatch;
	long time = 0;

	stopWatch.Start();
	converted = ConvertMatBitmapTowxBitmap(matBitmap, bitmap);
	time = stopWatch.Time();

	if (!converted)
	{
		wxLogError("Could not convert Mat to wxBitmap.");
		return wxBitmap();
	}

	timeConvert = time;
	return bitmap;
}
