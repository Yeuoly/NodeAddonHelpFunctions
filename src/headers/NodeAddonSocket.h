#include <napi.h>
#include <Windows.h>

#define NAS_SOCKET_ERR -1

class NodeAddonSocket : public Napi::ObjectWrap<NodeAddonSocket> {
    public:
        static void Init(Napi::Env env, Napi::Object exports);
        NodeAddonSocket(const Napi::CallbackInfo& info);

        Napi::Env *env;
        Napi::ThreadSafeFunction socket_callback;
        Napi::ThreadSafeFunction message_callback;

        SOCKET socket;
        SOCKET client_socket;
        DWORD thread_id;
        bool alive;
        uint32_t port;
        char *ip;
};