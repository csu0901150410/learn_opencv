
#include "sciter-x.h"
#include "sciter-x-graphics.hpp"
#include "sciter-x-window.hpp"
#include "sciter-om.h"

#include <string>
#include <locale>
#include <codecvt>
#include <functional>

#include "json/json.h"

#include "Register.h"

// native API demo

// 字符串类型转换
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
bool hello_tis(const Json::Value& params)
{
    std::string str = params["func_name"].asCString();
    if ("on_dom_load" == str)
        return true;
    return false;
}
REGISTER_FUNCTION("hello_tis", hello_tis);

class frame : public sciter::window
{
public:
    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS /*| SW_TOOL*/ | SW_MAIN | SW_ENABLE_DEBUG) {}

    bool tis_call_native_function_by_name(std::string& name, Json::Value& params)
    {
        bool ret = GET_REGISTERER_FUNCTION(name)(params);
        return ret;
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
            std::string strName = to_std_string(argv[0].to_string());
            std::string strParams = to_std_string(argv[1].to_string(CVT_JSON_LITERAL));

            Json::Reader jsReader;
            Json::Value jsParams;
            if (!jsReader.parse(strParams, jsParams))
            {
                std::cerr << "json parse failed" << std::endl;
                return false;
            }

            return tis_call_native_function_by_name(strName, jsParams);
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
