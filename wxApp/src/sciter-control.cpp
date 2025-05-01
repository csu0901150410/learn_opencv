#include <wx/wx.h>
#include <wx/control.h>
#include <wx/gdicmn.h>

#include "sciter-control.h"
#include <sciter-x-debug.h>

BEGIN_EVENT_TABLE(wxSciterControl, wxControl)
EVT_SIZE(wxSciterControl::OnSize)
EVT_SHOW(wxSciterControl::OnShow)
END_EVENT_TABLE();

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

}

void wxSciterControl::AdjustToParent()
{
	if (!GetParent() || !m_hwnd)
        return;
            
	wxSize parentClientSize = GetParent()->GetClientSize();
	
	// 先隐藏窗口
	ShowWindow(m_hwnd, SW_HIDE);
	
	// 调整大小和位置
	SetSize(0, 0, parentClientSize.GetWidth(), parentClientSize.GetHeight());
        
#ifdef __WINDOWS__
	RECT rc = { 0, 0, parentClientSize.GetWidth(), parentClientSize.GetHeight() };
	SetWindowPos(m_hwnd, nullptr, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER);
	
	// 重新显示窗口
	ShowWindow(m_hwnd, SW_SHOW);
#endif
}

wxSize wxSciterControl::DoGetBestSize() const
{
	return this->GetClientSize();
}

void wxSciterControl::OnSize(wxSizeEvent& evt)
{
 	wxRect rc = this->GetClientRect();
 	rc.SetSize(evt.GetSize());
 #ifdef __WINDOWS__
 	SetWindowPos(m_hwnd, nullptr, rc.GetX(), rc.GetY(), rc.GetWidth(), rc.GetHeight(), SWP_NOZORDER);
 #endif

	/*AdjustToParent();
    evt.Skip();*/
}

void wxSciterControl::OnShow(wxShowEvent& evt)
{
#ifdef __WINDOWS__
	ShowWindow(m_hwnd, evt.IsShown() ? SW_SHOW : SW_HIDE);
#endif
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