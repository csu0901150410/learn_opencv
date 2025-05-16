
#include <string>

#include "lsRegister.h"

// 该函数可以在脚本中通过 view.call_native("script_call_demo", jsparams) 调用
Json::Value script_call_demo(const Json::Value& params)
{
	std::string stdstr = params.toStyledString();

	std::string text = params["text"].asString();

	Json::Value retjs;
	retjs["ret_text"] = text.c_str();
	return retjs;
}
REGISTER_FUNCTION("script_call_demo", script_call_demo);

Json::Value grayscale(const Json::Value& params)
{
	float r = params["red"].asFloat();
	float g = params["green"].asFloat();
	float b = params["blue"].asFloat();

	// 用ui获取到参数，执行一些后台指令
	// ...

	return true;
}
REGISTER_FUNCTION("grayscale", grayscale);