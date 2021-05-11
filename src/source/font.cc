#include "../headers/font.h"

using namespace std;

/*
	这个是每个字体的回调，对于每一个字体，都会调用一次这个回调函数
*/
BOOL CALLBACK EnumFontFamEx(LPLOGFONT lplf, LPTEXTMETRIC lptm, int iType, LPARAM lpData) {
	//获取从主函数传来的数组列表
	vector<string> *ary;
	ary = (vector<string> *)lpData;
	ary->push_back(lplf->lfFaceName);

	return true;
}

void GetAllFonts(vector<string> *rslp) {
	//获取当前环境
	HDC hdc;
	hdc = GetDC(NULL);

	//if为一个config，告诉EnumFontFamililesEx应该获取一些什么样的字体，就跟filter一样
	LOGFONT lf;

	//先将if置零
	memset(&lf, 0, sizeof(LOGFONT));
	//设置要枚举的字符集，其实就是一个字体里可能有很多不同的格式，这里如果设置为DEFAULT_CHARSET的话就会把它们全部没举出来，我们肯定不希望出现这样的事
	//lf.lfCharSet = DEFAULT_CHARSET;
	//设置枚举所有字体
	lf.lfFaceName[0] = NULL;

	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamEx, (LPARAM)rslp, 0);

	ReleaseDC(NULL, hdc);
}