#include "Register.h"

Register *Register::instance()
{
    static Register *uniqueInstance = nullptr;
    if (nullptr == uniqueInstance)
        uniqueInstance = new Register();
    return uniqueInstance;
}

bool Register::register_function(const std::string &name, _register_func_ptr func)
{
    if (_registered_func.end() != _registered_func.find(name))
        return false;
    _registered_func[name] = func;
    return true;
}

_register_func_ptr Register::get_function(const std::string &name)
{
    if (_registered_func.end() == _registered_func.find(name))
        return nullptr;
    return _registered_func.at(name);
}
