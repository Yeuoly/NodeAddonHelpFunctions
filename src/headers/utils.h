#include<string>
#include <napi.h>

std::string GbkToUtf8(const char *src_str);

Napi::Value get_system_fonts(const Napi::CallbackInfo& info);

bool isIPAddressValid(const char* pszIPAddr);