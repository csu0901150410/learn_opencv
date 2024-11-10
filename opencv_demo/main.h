#include "sciter-x.h"
#include "sciter-x-graphics.hpp"
#include "sciter-x-window.hpp"
#include "sciter-om.h"

#include <string>
#include <functional>

#include "json/json.h"

#include "Register.h"

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

void call_script(const std::string& name);

class frame : public sciter::window
{
public:
    static frame* instance()
    {
        static frame* uniqueInstance = nullptr;
        if (nullptr == uniqueInstance)
            uniqueInstance = new frame();
        return uniqueInstance;
    }

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

private:
    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS /*| SW_TOOL*/ | SW_MAIN | SW_ENABLE_DEBUG) {}
};