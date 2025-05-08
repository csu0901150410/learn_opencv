#pragma once

#include <wx/control.h>
#include <sciter-x-window.hpp>

#include <unordered_set>

class wxOcvWindow;

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