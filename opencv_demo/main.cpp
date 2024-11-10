
#include "sciter-x.h"
#include "sciter-x-graphics.hpp"
#include "sciter-x-window.hpp"
#include "sciter-om.h"

#include <string>
#include <locale>
#include <codecvt>
#include <functional>

#include "Register.h"

// native API demo

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

// 命令定义及注册
std::string hello_tis(const std::string& param)
{
    return param;
}
REGISTER_FUNCTION("hello_tis", hello_tis);

class frame : public sciter::window
{
public:
    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS /*| SW_TOOL*/ | SW_MAIN | SW_ENABLE_DEBUG) {}

    sciter::string tis_call_native_function_by_name(std::string& name, std::string& param)
    {
        _register_func_ptr ptr = GET_REGISTERER_FUNCTION(name);
        _register_func_ptr target = hello_tis;
        std::string ret = ptr(param);
        return to_sciter_string(ret);
    }

    // BEGIN_FUNCTION_MAP
    //     FUNCTION_0("call_native", tis_call_native_function_by_name);
    // END_FUNCTION_MAP

    virtual bool on_script_call(HELEMENT he, LPCSTR name, UINT argc, const sciter::value* argv, sciter::value& retval)
    {
        aux::chars _name = aux::chars_of(name);

        // 暴露给脚本的接口，脚本通过这个接口根据c++命令名字调用命令
        if (const_chars("call_native") == _name)
        {
            // todo - 统一将第二个参数解析为json，如果解析失败，则参数无效
            std::string str_name = to_std_string(argv[0].to_string());
            std::string str_param = to_std_string(argv[1].to_string());
            retval = tis_call_native_function_by_name(str_name, str_param);
            return true;
        }

        return false;
    }
};

#include "resources.cpp"

int uimain(std::function<int()> run)
{
    sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "resources.cpp") with the archive

    sciter::om::hasset<frame> pwin = new frame();

    // note: this:://app URL is dedicated to the sciter::archive content associated with the application
    pwin->load(WSTR("this://app/main.htm"));

    pwin->expand();

    return run();
}
