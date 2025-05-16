#pragma once

#include <wx/control.h>
#include <sciter-x-window.hpp>

#include <unordered_set>

#include "lsRegister.h"

class wxOcvWindow;

/**
 * native/script交互的方式
 * 
 * 1.script_call_native。UI和后台交互，UI调用后台函数，并接收调用的返回值，比如UI从后台获取数据
 * 2.native_call_script。后台调用脚本，并接收调用的返回值，比如后台处理过程中需要弹窗交互并获取UI输入的数据
 * 
 */

class wxSciterControl :
	public wxControl,
	public sciter::host<wxSciterControl>,
	protected sciter::event_handler_raw
{
	friend struct sciter::host<wxSciterControl>;
	
	DECLARE_DYNAMIC_CLASS(wxSciterControl);
public:

	// Constructors
	wxSciterControl();
	wxSciterControl(wxWindow* parent, wxWindowID id);
	~wxSciterControl();

	// Creation
	bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxSUNKEN_BORDER, const wxValidator& validator = wxDefaultValidator);

public:
	sciter::dom::element get_root() const;

protected:
	// Common initialization
	void Init();

	// Event handlers
	void OnSize(wxSizeEvent& evt);
	void OnShow(wxShowEvent& evt);
	void OnPaint(wxPaintEvent& event);
	void OnKeyDown(wxKeyEvent& event);

	// sciter:host
	HWINDOW get_hwnd() const { return m_hwnd; }
	HINSTANCE get_resource_instance() const { return NULL; }

	virtual bool handle_scripting_call(HELEMENT he, SCRIPTING_METHOD_PARAMS& params) override;

private:
	Json::Value script_call_native(std::string& name, const Json::Value& params);

protected:
	HWINDOW m_hwnd;
	mutable sciter::dom::element m_root;

public:
	void register_ocvwindow(wxOcvWindow* window);
	void unregister_ocvwindow(wxOcvWindow* window);
	void update_ocvwindow(const wxBitmap& bitmap);

private:
	std::unordered_set<wxOcvWindow*> m_ocvwindows;
};