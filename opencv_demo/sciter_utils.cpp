#include "sciter_utils.h"

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