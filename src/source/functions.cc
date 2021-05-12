#include <napi.h>
#include "../headers/font.h"
#include "../headers/utils.h"
#include "../headers/NodeAddonSocket.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    NodeAddonSocket::Init(env, exports);
    exports.Set("get_system_fonts", Napi::Function::New(env, get_system_fonts));
	return exports;
}

NODE_API_MODULE(addon, Init)