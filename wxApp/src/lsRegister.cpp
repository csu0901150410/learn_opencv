#include "lsRegister.h"

lsRegister* lsRegister::instance()
{
	static lsRegister* uniqueInstance = nullptr;
	if (nullptr == uniqueInstance)
		uniqueInstance = new lsRegister();
	return uniqueInstance;
}

bool lsRegister::register_function(const std::string& name, _command_func_ptr func)
{
	if (_registered_func.end() != _registered_func.find(name))
		return false;
	_registered_func[name] = func;
	return true;
}

_command_func_ptr lsRegister::get_function(const std::string& name)
{
	if (_registered_func.end() == _registered_func.find(name))
		return nullptr;
	return _registered_func.at(name);
}
