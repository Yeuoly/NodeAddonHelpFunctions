#include "../headers/NodeAddonSocket.h"
#include "../headers/utils.h"
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

//定义处理线程，传进来的lparams是NodeAddonSocket的对象指针
DWORD WINAPI SocketListenerThreadProc(LPVOID lparmas) {
    //这个相当是数据中转，将Native数据转化成符合Napi标准的数据
    auto socket_callback = [](Napi::Env env, Napi::Function js_callback){
        js_callback.Call( {} );
    };
    auto message_callback = [](Napi::Env env, Napi::Function js_calllback, char *message){
        js_calllback.Call( { Napi::String::New(env, GbkToUtf8(message)) } );
    };
    //开启监听
    NodeAddonSocket *lp = (NodeAddonSocket *)lparmas;
    const char *ip = lp->ip;
    const uint32_t port = lp->port;
    //初始化套接字
    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2, 2), &wsadata) != 0){
        Napi::TypeError::New(*(lp->env), "Init Socket Failed - ERR001").ThrowAsJavaScriptException();
        return NAS_SOCKET_ERR;
    }
    lp->socket = socket(AF_INET, SOCK_STREAM, 0);
    //设置Socket信息
    SOCKADDR_IN addr_server;
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = port >= 1024 ? htons(port) : htonl(port);
    addr_server.sin_addr.S_un.S_addr = inet_addr(ip);

    if(SOCKET_ERROR == bind(lp->socket, (LPSOCKADDR)&addr_server, sizeof(SOCKADDR_IN))){
        Napi::TypeError::New(*(lp->env), "Bind Socket Failed - ERR002").ThrowAsJavaScriptException();
        return NAS_SOCKET_ERR;
    }
    //开始监听，监听长度为1
    if(SOCKET_ERROR == listen(lp->socket, 1)){
        Napi::TypeError::New(*(lp->env), "Start Socket Listen Failed - ERR003").ThrowAsJavaScriptException();
        return NAS_SOCKET_ERR;
    }
    SOCKADDR_IN addr_client;
    int len = sizeof(SOCKADDR);

    //等待连接
    SOCKET con = accept(lp->socket, (LPSOCKADDR)&addr_client, &len);
    if(con == SOCKET_ERROR){
        Napi::TypeError::New(*(lp->env), "Socket Connect Failed - ERR004").ThrowAsJavaScriptException();
        return NAS_SOCKET_ERR;
    }
    //接收到了连接，给一个Callback
    lp->socket_callback.BlockingCall();
    char buf[1401];
    //这里有个坑，如果两次recv很快的话，buf会被覆盖成最后一次的结果然后输出两次，所以我们需要拷贝一下，还没写这个逻辑，先记一下
    while(recv(con, buf, sizeof(buf), 0) != SOCKET_ERROR){
        lp->message_callback.BlockingCall(buf, message_callback);
    }
    return 0;
}

//定义处理线程，传进来的lparams是NodeAddonSocket的对象指针
DWORD WINAPI SocketSenderThreadProc(LPVOID lparmas) {
    //NodeAddonSocket *lp = (NodeAddonSocket *)lparmas;
}

//这个是其实就相当于Node里类的constructor函数，当一个类被初始化的时候，我们在C++里实现这个类的初始化，然后传递对象引用给Node
//而在JS中，类的作用域其实是函数作用域，所以这里建立的类引用本质上是函数引用
void NodeAddonSocket::Init(Napi::Env env, Napi::Object exports){
    //DefineClass可以返回当前类的地址和结构等信息，作为一个Function保存，Node拿到类的地址以后就可以利用这个类的信息创建对象了，这里还传入了函数地址
    Napi::Function func = DefineClass(env, "NodeAddonSocket", {});
    //当执行完DefineClass以后，C++内抽象了一个JS的类，并把这个类的信息作为一个Function实例保存了下来


    //这里给Node传递这个模块的NodeAddonSocket类的构造函数
    exports.Set("NodeAddonSocket", func);

    //将这个类的信息保存在了环境中，以便可以在C++内抽象地创建一个JS对象，如下，返回值一个Function/Value
    //return env.GetInstanceData<Napi::FunctionReference>()->New();

    //Napi::FunctionReference* constructor = new Napi::FunctionReference();
    //*constructor = Napi::Persistent(func);
    //env.SetInstanceData<Napi::FunctionReference>(constructor);
}

NodeAddonSocket::NodeAddonSocket(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeAddonSocket>(info) {
    //初始化参数，挂载回调函数
    //info[0] -> ip
    //info[1] -> port
    //info[2] -> get socket callback
    //info[3] -> get message callback
    this->alive = false;
    Napi::Env env = info.Env();
    this->env = &env;
    if(info.Length() < 4 
        || !info[0].IsString() 
        || !info[1].IsNumber() 
        || !info[2].IsFunction() 
        || !info[3].IsFunction() 
        // || !isIPAddressValid(info[0].As<Napi::String>().Utf8Value().c_str())
        ){
        Napi::TypeError::New(env, "wrong params type or invalid ip address").ThrowAsJavaScriptException();
        return;
    }
    //保存这些信息，并创建线程安全函数，这个线程安全函数其实是用来完成队列行为的，比如说我们这里创建了以NASCALLBACK_SOCKET为ResourceName的线程安全函数
    //并绑定了这个资源名对应的Hook，当我们调用callback.BlockingCall的时候，实际上就是在这个Hook的队列上添加要用来执行的数据，然后由
    //ThreadSafeFunction在主线程中调用Callback
    this->ip = (char *)info[0].As<Napi::String>().Utf8Value().c_str();
    this->port = info[1].As<Napi::Number>().Uint32Value();
    this->socket_callback = Napi::ThreadSafeFunction::New(env, info[2].As<Napi::Function>(), "NASCALLBACK_SOCKET", 0, 1);
    this->message_callback = Napi::ThreadSafeFunction::New(env, info[3].As<Napi::Function>(), "NASCALLBACK_MESSAGE", 0, 1);
    //开启新线程用于监听连接和收消息，并把当前对象指针传过去
    CreateThread(NULL, NULL, SocketListenerThreadProc, this, NULL, &this->thread_id);
}