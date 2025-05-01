// ui-framework.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ui-framework.h"

#include "json/json.h"

#include "sciter_utils.h"
#include "Register.h"

#include <opencv2/opencv.hpp>
#include <vector>

#include "resources.cpp"

HINSTANCE ghInstance = 0;
sciter::image gMainImage;

#define WINDOW_CLASS_NAME L"sciter-ui-frame"			// the main window class name

//
//  FUNCTION: window::init()
//
//  PURPOSE: Registers the window class.
//
bool window::init_class()
{
	static ATOM cls = 0;
	if (cls)
		return true;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wnd_proc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = ghInstance;
	wcex.hIcon = LoadIcon(ghInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;//MAKEINTRESOURCE(IDC_PLAINWIN);
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	cls = RegisterClassEx(&wcex);
	return cls != 0;
}

window::window()
{
	init_class();
	_hwnd = CreateWindow(WINDOW_CLASS_NAME, L"demo", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, ghInstance, this);
	if (!_hwnd)
		return;

	init();

	ShowWindow(_hwnd, SW_SHOW);
	UpdateWindow(_hwnd);
}

window* window::ptr(HWND hwnd)
{
	return reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

bool window::init()
{
	SetWindowLongPtr(_hwnd, GWLP_USERDATA, LONG_PTR(this));
	setup_callback(); // callbacks
	sciter::attach_dom_event_handler(_hwnd, this); // dom events and callbacks

#ifdef _DEBUG
	// 调试模式下，直接加载文件
	// "home://main.htm"即exe路径下的main.htm
	// see https://sciter.com/forums/reply/87076/
	load_file(L"home://../resources/html/main.htm");
#else
	sciter::archive::instance().open(aux::elements_of(resources));
	load_file(L"this://app/main.htm");
#endif

	return true;
}

// 注册命令示例
bool command_register_sample(const Json::Value& params)
{
    // 解析命令参数
	int threshold = params["threshold_value"].asInt();

	if (!gMainImage.is_valid())
		return false;

    // 执行c++侧的一些处理，比方说把图像二值化
	sciter::bytes_writer bw;
	gMainImage.save(bw, SCITER_IMAGE_ENCODING_RAW);
	
	UINT width = 0, height = 0;
	gMainImage.dimensions(width, height);
	cv::Mat srcimage = cv::Mat(width * height * 4, 1, CV_8U, (unsigned*)bw.bb.data());

	if (srcimage.empty())
		return false;

	int srcc = srcimage.channels();

	// 转灰度
	cv::Mat grayimage = srcimage.reshape(4, 0);
	int gccc = grayimage.channels();
	// see https://sciter.com/forums/topic/himg-raw-image-data/
	cv::cvtColor(grayimage, grayimage, cv::COLOR_BGRA2RGB);
	gccc = grayimage.channels();
	cv::cvtColor(grayimage, grayimage, cv::COLOR_RGB2GRAY);
	gccc = grayimage.channels();

	// 灰度图二值化
	cv::Mat binimage;
	int maxvalue = 255;
	int type = cv::THRESH_BINARY;
	cv::threshold(grayimage, binimage, 175, 255, cv::THRESH_BINARY);

	
	// 再转回sciter::image
	int channels = binimage.channels();
	int rows = binimage.rows;
	int cols = binimage.cols;
 
	char* dp = (char*)binimage.data;
	std::vector<uchar> buffer;
	if (channels == 3)
	{
		for (int i = 0; i < rows * cols; i++)
		{
			buffer.push_back(dp[i * 3]);
			buffer.push_back(dp[i * 3 + 1]);
			buffer.push_back(dp[i * 3 + 2]);
		}
	}
	else if (channels == 1)
	{
		for (int i = 0; i < rows * cols; i++)
		{
			buffer.push_back(dp[i]);
		}
	}
	
	aux::bytes bytes(&buffer[0], buffer.size());
	gMainImage = sciter::image::create(width, height, false, &buffer[0]);

	// 此处可以在类外部调用脚本函数，其实现和在类外部注册被脚本调用的命令差不多，后续实现
	// todo - 调用脚本函数刷新图像
    
    return true;
}
REGISTER_FUNCTION("command_register_sample", command_register_sample);

bool window::custom_on_script_call(HELEMENT he, LPCSTR name, UINT argc, const sciter::value *argv, sciter::value &retval)
{
	aux::chars _name = aux::chars_of(name);
	
	if (const_chars("call_native") == _name)
	{
		// 脚本调用c++函数，统一用这个接口。脚本中语句view.call_native("funcname", params)
		// 此处将会根据字符串"funcname"找到对应的函数指针，执行c++函数调用
		// 注意此处被脚本调用的c++函数并非类的成员函数，而是定义在类外的函数，可称为命令

		std::string strName = to_std_string(argv[0].to_string());
		std::string strParams = to_std_string(argv[1].to_string(CVT_JSON_LITERAL));

		Json::Reader jsReader;
		Json::Value jsParams;
		if (!jsReader.parse(strParams, jsParams))
		{
			std::cerr << "json parse failed : " << strParams << std::endl;
			return false;
		}
		
		EXECUTE_REGISTER_FUNCTION(strName, jsParams);
		return true;// 此处处理完c++函数调用，外部重载的on_script_call就直接返回了
	}
	
    return false;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//

LRESULT CALLBACK window::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//SCITER integration starts
	BOOL handled = FALSE;
	LRESULT lr = SciterProcND(hWnd, message, wParam, lParam, &handled);
	if (handled)
		return lr;
	//SCITER integration ends

	window* self = ptr(hWnd);

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


// the WinMain

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	ghInstance = hInstance;

	// TODO: Place code here.
	MSG msg;

	// Perform application initialization:

	sciter::sync::gui_thread_ctx _; // instance of gui_thread_ctx
	// it should be created as a variable inside WinMain 
	// gui_thread_ctx is critical for GUI_CODE_START/END to work

	OleInitialize(NULL); // for shell interaction: drag-n-drop, etc.

	// See https://sciter.com/forums/topic/event_handler-trouble/
	sciter::om::hasset<window> pwin = new window();
	if (!pwin->is_valid())
		return FALSE;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	OleUninitialize();

	return (int)msg.wParam;
}
