// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "Kryptonite_i.h"
#include "dllmain.h"
#include "xdlldata.h"

CKryptoniteModule _AtlModule;

#ifndef DEBUGASEXE
// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
#else
#include "Crypto.h"
#undef DEBUG

class Crypto : CCrypto
{
public:
	Crypto()
	{
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 0;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(const IID &,void **)
	{
		return 0;
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}

	int _init(const TCHAR* name, const TCHAR* path)
	{
		BSTR bName = SysAllocString((const OLECHAR*)name);
		BSTR bPath = SysAllocString((const OLECHAR*)path);
		int nextAction;
		CCrypto::init(bName, bPath, &nextAction);
		return nextAction;
	}

	int _generatePrivateKey()
	{
		int nextAction;
		CCrypto::generatePrivateKey(&nextAction);
		return nextAction;
	}

	int _createFingerprintStore()
	{
		int nextAction;
		CCrypto::createFingerprintStore(&nextAction);
		return nextAction;
	}

	int _startSession(const TCHAR* partner)
	{
		int nextAction;
		BSTR bPartner = SysAllocString((const OLECHAR*)partner);
		CCrypto::startSession(bPartner, &nextAction);
		return nextAction;
	}

	int _getNextEvent(const TCHAR* partner)
	{
		BSTR bPartner = SysAllocString((const OLECHAR*)partner);
		int eventType, eventsLeft;
		BSTR eventData;
		CCrypto::getNextEvent(bPartner, &eventType, &eventData, &eventsLeft);
		return eventsLeft;
	}

	int _getNextMessage(const TCHAR* partner)
	{
		BSTR bPartner = SysAllocString((const OLECHAR*)partner);
		int messagesLeft;
		BSTR msg;
		CCrypto::getNextMessage(bPartner, &msg, &messagesLeft);
		return messagesLeft;
	}

	int _gotMessage(const TCHAR* partner, const TCHAR* message)
	{
		BSTR bPartner = SysAllocString((const OLECHAR*)partner);
		BSTR bMsg = SysAllocString((const OLECHAR*)message);
		BOOL deploy;
		BSTR plain;
		int nextAction;
		CCrypto::messageReceived(bPartner, bMsg, &plain, &deploy, &nextAction);
		return nextAction;
	}
};

//main for debugging
int main(int argc, _TCHAR* argv[])
{
	const TCHAR* msgFromSybille = L"?OTRv2?<b>sybille</b> has requested an <a href=\"http://otr.cypherpunks.ca/\">Off-the-Record private conversation</a>.  However, you do not have a plugin to support that. See <a href=\"http://otr.cypherpunks.ca/\">http://otr.cypherpunks.ca/</a> for more information";
	Crypto* crypto = new Crypto();
	crypto->_init(L"sundance",L"%AppData%\\watchasayin\\Kryptonite\\");
	//crypto->_generatePrivateKey();
	//crypto->_createFingerprintStore();
	//crypto->_startSession(L"sybille");
	//crypto->_getNextEvent(L"sybille");

	crypto->_gotMessage(L"sybille", msgFromSybille);

	/*
	int messagesLeft = 0;
	do
	{
		messagesLeft = crypto->_getNextMessage(L"sybille");
		printf ("%d messages left.", messagesLeft);

	} while(messagesLeft >0);
	*/
	return 0;
}
#endif
