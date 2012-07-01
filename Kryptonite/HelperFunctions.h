#pragma once
#include "stdlib.h"
#include <string>

//! Static class with various helper functions
class HelperFunctions
{

public:
	
	//! Conversion of std::wstring to std::string
	static std::string wstringToString(const std::wstring &wstr)
	{
		char* mb = (char*)malloc(wstr.length()+1);
	
		wcstombs(mb, wstr.c_str(), wstr.length());
		mb[wstr.length()] = 0x00;
		
		std::string str = mb;
		free(mb);
		return str;
	}

	//! Conversion of std::string to std::wstring
	static std::wstring stringToWstring(const std::string &str)
	{
		wchar_t* wc = (wchar_t*)malloc((str.length()+1)*sizeof(wchar_t));

		mbstowcs(wc, str.c_str(), str.length());
		wc[str.length()] = (wchar_t)0x00;

		std::wstring wstr = wc;
		free(wc);
		return wstr;
	}
};