#pragma once

#include <string>
#include <unordered_map>

#include "json/json.h"

typedef Json::Value (*_command_func_ptr)(const Json::Value&);// 被注册的函数指针类型

// 注册器
// 实现思路 : 用一个单例类的全局唯一对象来保存注册的函数名字符串到函数指针的映射关系
class lsRegister
{
public:
	static lsRegister* instance();

	Json::Value register_function(const std::string& name, _command_func_ptr func);

	_command_func_ptr get_function(const std::string& name);

private:
	lsRegister() = default;
	std::unordered_map<std::string, _command_func_ptr> _registered_func;
};

// 函数注册宏。根据要注册的函数名拼接一个新的类名，在此类的构造函数中将函数注册到注册器单例对象
#define REGISTER_FUNCTION(name, func) \
class Register_##func { \
public: \
    Register_##func() { \
        lsRegister::instance()->register_function(name, func); \
    } \
}; \
Register_##func g_register_##func;

// 函数获取宏
#define GET_REGISTERER_FUNCTION(name) \
lsRegister::instance()->get_function(name)
