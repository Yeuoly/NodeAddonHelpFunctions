#include <napi.h>
#include "../headers/font.h"
#include "../headers/utils.h"

/**
 * 数组求和
 **/
Napi::Number sum_array(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    double sum = 0;
    if(info.Length() <= 0 || !info[0].IsArray()){
        Napi::TypeError::New(env, "the param input is not a array").ThrowAsJavaScriptException();
        return Napi::Number::New(env, sum);
    }
    Napi::Array ary = info[0].As<Napi::Array>();
    uint32_t len = ary.Length();
    for(uint32_t i = 0; i < len; i++){
        sum += static_cast<Napi::Value>(ary[i]).ToNumber().DoubleValue();
    }
    return Napi::Number::New(env, sum);
}

/**
 * 向量点乘
 * */
Napi::Number vector_dot(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    //检查参数
    if(info.Length() <= 1 || !info[0].IsArray() || !info[1].IsArray()){
        Napi::TypeError::New(env, "params exception").ThrowAsJavaScriptException();
        return Napi::Number::New(env, 0);
    }
    Napi::Array vec1 = info[0].As<Napi::Array>();
    Napi::Array vec2 = info[1].As<Napi::Array>();

    if(vec2.Length() != vec1.Length()){
        Napi::TypeError::New(env, "params exception").ThrowAsJavaScriptException();
        return Napi::Number::New(env, 0);
    }
    double res = 0;
    uint32_t len = vec2.Length();
    for(uint32_t i = 0; i < len; i++){
        res += static_cast<Napi::Value>(vec1[i]).ToNumber().DoubleValue() *
            static_cast<Napi::Value>(vec2[i]).ToNumber().DoubleValue();
    }
    return Napi::Number::New(env, res);
}

/**
 * 获取系统字体
 * */
Napi::Value get_system_fonts(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::vector<std::string> fonts;
    GetAllFonts(&fonts);
    size_t len = fonts.size();
    Napi::Array ary = Napi::Array::New(env, len);
    for(size_t i = 0; i < len; i++){
        std::string utf8str = GbkToUtf8(fonts[i].c_str());
        ary[i] = Napi::String::New(env, utf8str);
    }
    return ary;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("sum_array", Napi::Function::New(env, sum_array));
    exports.Set("vector_dot", Napi::Function::New(env, vector_dot));
    exports.Set("get_system_fonts", Napi::Function::New(env, get_system_fonts));
	return exports;
}

NODE_API_MODULE(addon, Init)