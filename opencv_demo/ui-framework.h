#pragma once

#include <functional>
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "sciter-x-threads.h"

extern HINSTANCE ghInstance;

class window : public sciter::host<window>,
    public sciter::event_handler
{
    HWND _hwnd;

    static LRESULT CALLBACK	wnd_proc(HWND, UINT, WPARAM, LPARAM);
    static window* ptr(HWND hwnd);
    static bool init_class();
public:
    // notification_handler traits:
    HWND      get_hwnd() { return _hwnd; }
    HINSTANCE get_resource_instance() { return ghInstance; }

    window();
    bool init(); // instance
    bool is_valid() const { return _hwnd != 0; }

private:
    // function exposed to script
    sciter::value expose_to_script()
    {
        return TEXT("hello tis");
    }

    // call script function
    sciter::value call_script()
    {
        return this->call_function("script_call_by_native");
    }

public:
// expose member methods to script
// in script: view.member_method(...)
BEGIN_FUNCTION_MAP
    FUNCTION_0("native_function_call_by_script", expose_to_script);
    FUNCTION_0("call_script", call_script);
END_FUNCTION_MAP
};
