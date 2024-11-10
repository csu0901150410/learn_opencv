
#include "main.h"

#include "resources.cpp"

// native API demo

// 命令定义及注册
bool hello_tis(const Json::Value& params)
{
    // 解析命令参数
    std::string name = params["script"].asCString();

    // 调用tis的脚本函数
    call_script(name);
    
    return true;
}
REGISTER_FUNCTION("hello_tis", hello_tis);

// 调用脚本函数的api
void call_script(const std::string& name)
{
    frame::instance()->call_function(name.c_str());
}

int uimain(std::function<int()> run)
{
    sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "resources.cpp") with the archive

    sciter::om::hasset<frame> pwin = frame::instance();

    // note: this:://app URL is dedicated to the sciter::archive content associated with the application
    pwin->load(WSTR("this://app/main.htm"));

    pwin->expand();

    return run();
}
