#pragma once

#include <functional>
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "sciter-x-threads.h"
#include "sciter-x-graphics.hpp"

#include "sciter_utils.h"

extern HINSTANCE ghInstance;
extern sciter::image gMainImage;

#define CUSTOM_FUNCTION_MAP() if (custom_on_script_call(he, name, argc, argv, retval)) {return true;}

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

    // 加载文件中的图像到全局图像
    sciter::value load_workarea_image()
    {
        aux::bytes bytes = sciter::archive::instance().get(TEXT("//images/opencv-logo.png"));
        gMainImage = sciter::image::load(bytes);
        return gMainImage.to_value();
    }

    // 将全局的图像刷新到界面
    sciter::value refresh_workarea_image()
    {
        if (gMainImage.is_valid())
            return gMainImage.to_value();
        return load_workarea_image();
    }

    // c++根据脚本函数名字调用脚本函数
    sciter::value call_script(const sciter::value& name, const sciter::value& params)
    {
        std::string _name = to_std_string(name.to_string());
        return this->call_function(_name.c_str(), params);
    }

    // BEGIN/END_FUNCTION_MAP宏其实是重载了on_script_call，这里先处理一部分情况
    bool custom_on_script_call(HELEMENT he, LPCSTR name, UINT argc, const sciter::value* argv, sciter::value& retval);

public:
// 此处通过FUNCTION_X确定字符串到成员函数指针的映射，脚本中通过view.funcname(...)可调用类的成员函数
BEGIN_FUNCTION_MAP
    CUSTOM_FUNCTION_MAP()
    FUNCTION_0("load_workarea_image", load_workarea_image);
    FUNCTION_0("refresh_workarea_image", refresh_workarea_image);
    FUNCTION_2("call_script", call_script);
END_FUNCTION_MAP
};
