
#include <string>

#include "lsRegister.h"

// 这个命令返回值只能是bool有点局限，如果脚本调用c++需要返回json，还要改
// 命令定义及注册
extern void call_script(const std::string& name, const Json::Value& params);
bool script_call_demo(const Json::Value& params)
{
	std::string stdstr = params.toStyledString();

	std::string text = params["text"].asString();

	// 因为现在不支持返回json给脚本，所以调用另一个脚本函数设置文本
	call_script("native_set_text", text.c_str());

	return true;
}
REGISTER_FUNCTION("script_call_demo", script_call_demo);

bool grayscale(const Json::Value& params)
{
	float r = params["red"].asFloat();
	float g = params["green"].asFloat();
	float b = params["blue"].asFloat();

	// 用ui获取到参数，执行一些后台指令
	// ...

	return true;
}
REGISTER_FUNCTION("grayscale", grayscale);