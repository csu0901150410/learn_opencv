#include <wx/wx.h>
#include <wx/control.h>
#include <wx/gdicmn.h>

#include "wxSciterControl.h"
#include <sciter-x-debug.h>

#include "wxOcvWindow.h"
#include "lsRegister.h"

IMPLEMENT_DYNAMIC_CLASS(wxSciterControl, wxControl);

wxSciterControl::wxSciterControl(wxWindow* parent, wxWindowID id)
	:m_hwnd()
{
	Init();
	Create(parent, id);
}

wxSciterControl::wxSciterControl()
	:m_hwnd()
{
	Init();
}

wxSciterControl::~wxSciterControl()
{
#ifdef __WINDOWS__
	::DestroyWindow(m_hwnd);
#endif
}

bool wxSciterControl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/, long style /*= wxSUNKEN_BORDER*/, const wxValidator& validator /*= wxDefaultValidator*/)
{
	if (!wxControl::Create(parent, id, pos, size, style, validator))
		return false;

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	SetInitialSize(size);

	// Create Sciter child window with size exactly as our control.
	const auto crc = this->GetClientRect();
	RECT rc = { crc.GetLeft(), crc.GetTop(), crc.GetRight(), crc.GetBottom() };
	m_hwnd = SciterCreateWindow(SW_CHILD, &rc, nullptr, nullptr, this->GetHandle());

	if (m_hwnd) {
		static volatile sciter::debug_output setup_dbg;

		// Setup sciter callbacks
		setup_callback();
		sciter::attach_dom_event_handler(m_hwnd, this);

		// wxSciterControl窗口指针存到sciter窗口中，以便后续注册挂载窗口
		::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)(this));

		// Adjust our window style to eliminate double edge 
		if (this->HasFlag(wxBORDER_MASK)) {
			auto style = this->GetWindowStyleFlag();
			style &= ~wxBORDER_MASK;
			style |= wxBORDER_NONE;
			this->SetWindowStyleFlag(style);
		}

		// Show Sciter window
#ifdef __WINDOWS__
		::ShowWindow(m_hwnd, this->IsShown() ? SW_SHOW : SW_HIDE);
#endif

		this->Refresh();
	}

	return true;
}

void wxSciterControl::Init()
{
	Bind(wxEVT_SIZE, &wxSciterControl::OnSize, this);
	Bind(wxEVT_SHOW, &wxSciterControl::OnShow, this);
	Bind(wxEVT_PAINT, &wxSciterControl::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &wxSciterControl::OnKeyDown, this);
}

void wxSciterControl::OnSize(wxSizeEvent& evt)
{
 	wxRect rc = this->GetClientRect();
 	rc.SetSize(evt.GetSize());
 #ifdef __WINDOWS__
 	SetWindowPos(m_hwnd, nullptr, rc.GetX(), rc.GetY(), rc.GetWidth(), rc.GetHeight(), SWP_NOZORDER);
 #endif
}

void wxSciterControl::OnShow(wxShowEvent& evt)
{
#ifdef __WINDOWS__
	ShowWindow(m_hwnd, evt.IsShown() ? SW_SHOW : SW_HIDE);
#endif
}

void wxSciterControl::OnPaint(wxPaintEvent& event)
{
	int a = 100;
}

void wxSciterControl::OnKeyDown(wxKeyEvent& event)
{
	// 获取原始虚拟键码
	WPARAM vkCode = static_cast<WPARAM>(event.GetRawKeyCode());
	// 构造简化的lParam
	LPARAM lParam = (0x00000001 | (MapVirtualKey(vkCode, MAPVK_VK_TO_VSC) << 16));

	::PostMessage(m_hwnd, WM_KEYDOWN, vkCode, lParam);
	event.Skip();
}

std::string to_std_string(const sciter::string& str)
{
	aux::w2a auxstr(str.c_str());
	return auxstr.c_str();
}

sciter::string to_sciter_string(const std::string& str)
{
	aux::a2w auxstr(str.c_str());
	return auxstr.c_str();
}

bool wxSciterControl::handle_scripting_call(HELEMENT he, SCRIPTING_METHOD_PARAMS& params)
{
	aux::chars _name = aux::chars_of(params.name);

	// 暴露给脚本的接口，脚本通过这个接口根据c++命令名字调用命令
	if (const_chars("call_native") == _name)
	{
		std::string strName = to_std_string(params.argv[0].to_string());
		std::string strParams = to_std_string(params.argv[1].to_string(CVT_JSON_LITERAL));

		Json::Reader jsReader;
		Json::Value jsParams;
		if (!jsReader.parse(strParams, jsParams))
		{
			std::cerr << "json parse failed" << std::endl;
			return false;
		}

		return script_call_native(strName, jsParams);
	}

	return true;
}

bool wxSciterControl::script_call_native(std::string& name, const Json::Value& params)
{
	bool ret = GET_REGISTERER_FUNCTION(name)(params);
	return ret;
}

void wxSciterControl::register_ocvwindow(wxOcvWindow* window)
{
	m_ocvwindows.insert(window);
}

void wxSciterControl::unregister_ocvwindow(wxOcvWindow* window)
{
	if (!m_ocvwindows.count(window))
		return;
	m_ocvwindows.erase(window);
}

void wxSciterControl::update_ocvwindow(const wxBitmap& bitmap)
{
	for (auto wnd : m_ocvwindows)
	{
		if (wnd && wnd->IsShownOnScreen())
			wnd->set_bitmap(bitmap);
	}
}

sciter::dom::element wxSciterControl::get_root() const
{
	if (!m_root) {
		// 子类get_root是const的，父类get_root是非const的，所以要将this指针的const属性去掉再调用父类方法
		sciter::host<wxSciterControl>* base = const_cast<wxSciterControl*>(this);
		m_root = base->get_root();
	}
	return m_root;
}
