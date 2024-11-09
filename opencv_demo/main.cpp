
#include "sciter-x.h"
#include "sciter-x-graphics.hpp"
#include "sciter-x-window.hpp"
#include "sciter-om.h"

#include <functional>

// native API demo

class frame : public sciter::window
{
public:
    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS /*| SW_TOOL*/ | SW_MAIN | SW_ENABLE_DEBUG) {}

    sciter::string tis_call_native_function_by_name()
    {
        return WSTR("tis_call_native_function_by_name");
    }

    // BEGIN_FUNCTION_MAP
    //     FUNCTION_0("call_native", tis_call_native_function_by_name);
    // END_FUNCTION_MAP

    virtual bool on_script_call(HELEMENT he, LPCSTR name, UINT argc, const sciter::value* argv, sciter::value& retval)
    {
        aux::chars _name = aux::chars_of(name);

        // c++函数暴露给脚本，没有参数。其实就是字符串到函数指针的映射
        if (const_chars("call_native") == _name)
        {
            retval = tis_call_native_function_by_name();
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
