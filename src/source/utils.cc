#include <Windows.h>
#include <stdio.h>
#include "../headers/utils.h"
#include "../headers/font.h"

using namespace std;

string GbkToUtf8(const char *src_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
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


bool isIPAddressValid(const char* pszIPAddr) {
    if (!pszIPAddr) return false; //若pszIPAddr为空  
    char IP1[100],cIP[4];  
    int len = strlen(pszIPAddr);  
    int i = 0,j=len-1;  
    int k, m = 0,n=0,num=0;  
    //去除首尾空格(取出从i-1到j+1之间的字符):  
    while (pszIPAddr[i++] == ' ');  
    while (pszIPAddr[j--] == ' ');  
      
    for (k = i-1; k <= j+1; k++)  
    {  
        IP1[m++] = *(pszIPAddr + k);  
    }       
    IP1[m] = '\0';  
      
    char *p = IP1;  
  
    while (*p!= '\0')  
    {  
        if (*p == ' ' || *p<'0' || *p>'9') return false;  
        cIP[n++] = *p; //保存每个子段的第一个字符，用于之后判断该子段是否为0开头  
  
        int sum = 0;  //sum为每一子段的数值，应在0到255之间  
        while (*p != '.'&&*p != '\0')  
        {  
          if (*p == ' ' || *p<'0' || *p>'9') return false;  
          sum = sum * 10 + *p-48;  //每一子段字符串转化为整数  
          p++;  
        }  
        if (*p == '.') {  
            if ((*(p - 1) >= '0'&&*(p - 1) <= '9') && (*(p + 1) >= '0'&&*(p + 1) <= '9'))//判断"."前后是否有数字，若无，则为无效IP，如“1.1.127.”  
                num++;  //记录“.”出现的次数，不能大于3  
            else  
                return false;  
        };  
        if ((sum > 255) || (sum > 0 && cIP[0] =='0')||num>3) return false;//若子段的值>255或为0开头的非0子段或“.”的数目>3，则为无效IP  
  
        if (*p != '\0') p++;  
        n = 0;  
    }  
    if (num != 3) return false;  
    return true;  
}