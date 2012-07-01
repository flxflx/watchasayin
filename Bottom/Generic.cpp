// Generic.cpp : Implementation of CGeneric

#include "stdafx.h"
#include "Generic.h"
#include <string>
#include <vector>
#include "Logger.h"


void unloadLibrary(HMODULE hSelf)
{
	Sleep(250);
	FreeLibraryAndExitThread(hSelf, 0);
}

// CGeneric
STDMETHODIMP CGeneric::getPid(int* pid)
{
	*pid = GetCurrentProcessId();
	return S_OK;
}

STDMETHODIMP CGeneric::getLog(SAFEARRAY* * lines)
{
	*lines = Logger::toSafeArray();
	return S_OK;
}


STDMETHODIMP CGeneric::unload(BSTR ownName)
{
	//get own handle
	HMODULE hSelf = GetModuleHandle(ownName);
	if (hSelf == NULL)
	{
		Logger::log(L"Error: Could not get handle to own module.");
		return S_FALSE;
	}

	//start thread to free self
	CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)unloadLibrary, hSelf, 0, 0);
	
	return S_OK;
}

