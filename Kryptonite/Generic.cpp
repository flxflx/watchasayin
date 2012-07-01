// Generic.cpp : Implementation of CGeneric

#include "stdafx.h"
#include "Generic.h"
#include "Logger.h"

// CGeneric
STDMETHODIMP CGeneric::getLog(SAFEARRAY* * lines)
{
	*lines = Logger::toSafeArray();
	return S_OK;
}
