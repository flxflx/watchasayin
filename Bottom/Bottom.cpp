// Bottom.cpp : Implementation of WinMain
#include "stdafx.h"
#include "resource.h"
#include "Bottom_i.h"

#ifndef DEBUGASEXE
class CBottomModule : public ATL::CAtlExeModuleT< CBottomModule >
	{
public :
	DECLARE_LIBID(LIBID_BottomLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_BOTTOM, "{E06E4C4A-D2C0-4E3B-B39E-43A6A3878AFF}")
	HRESULT setUpCOMAsUser();
	HRESULT tearDownCOMAsUser();
	};

CBottomModule _AtlModule;

/**
* Wrapper for _AtlModule.setUpCOMAsUser. This needs to be started as a seperated thread, since RegisterClassObjects
* appearently must not be called directly from DllMain().
**/
void initCOMThread()
{
	_AtlModule.setUpCOMAsUser();
}

#define TEAR_DOWN_EVENTNAME L"BTMTEARDOWN"



/**
* The entry-point of this dll. Starts initCOMThread() in a new thread in case of an DLL_PROCESS_ATTACH (first event after injection) event.
**/
extern "C" int __stdcall DllMain(HINSTANCE hInstance, int dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//start com server in new thread
		CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)initCOMThread, NULL, 0, 0);
		break;

	case DLL_PROCESS_DETACH:
		break;
	default:
		break;

	}
	return TRUE;
}

/**
* Slightly modified derivate of CAtlExeModuleT::WinMain from atlbase.h
**/
HRESULT CBottomModule::setUpCOMAsUser() throw()
{
		HRESULT hr = S_OK;

#if !defined(_ATL_NO_COM_SUPPORT)
		hr = this->InitializeCom();
		if (FAILED(hr))
		{
			// Ignore RPC_E_CHANGED_MODE if CLR is loaded. Error is due to CLR initializing
			// COM and InitializeCOM trying to initialize COM with different flags.
			if (hr != RPC_E_CHANGED_MODE || GetModuleHandle(_T("Mscoree.dll")) == NULL)
			{
				ATLASSERT(0);
				return hr;
			}
		}
		else
		{
			m_bComInitialized = true;
		}
#endif	//  !defined(_ATL_NO_COM_SUPPORT)

		//register server as user
		this->ParseCommandLine(L"/RegServerPerUser", &hr);
		this->Run(SW_SHOWNORMAL);

#ifdef _DEBUG
		// Prevent false memory leak reporting. ~CAtlWinModule may be too late.
		this->Term();
#endif	// _DEBUG

#ifndef _ATL_NO_COM_SUPPORT
		if (m_bComInitialized)
			this->UninitializeCom();
#endif // _ATL_NO_COM_SUPPORT

		if (hr != S_OK)
		{
			MessageBoxA(NULL, "Something went wrong","Fail", MB_ICONINFORMATION);
		}

		return hr;
}

HRESULT CBottomModule::tearDownCOMAsUser() throw()
{
	HRESULT hr;
	this->ParseCommandLine(L"/UnregServerPerUser", &hr);
	return hr;
}

#else
#include "HookingEngine.h"
#undef DEBUG

class HookEngine : CHookingEngine
{
public:
	HookEngine()
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

	bool _init()
	{
		BSTR bPath = SysAllocString((const OLECHAR*)L"\\");
		BOOL success;
		CHookingEngine::init(bPath, &success);
		return (success==1);
	}
	
	bool _hookNetworkIO()
	{
		BOOL success;
		CHookingEngine::hookNetworkIO(&success);
		return (success==1);
	}

	int _createSession()
	{
		unsigned int si;
		CHookingEngine::createSession(&si);
		return si;
	}

	bool _addConnectionOutgoing(unsigned int sessionId, unsigned int ip, unsigned short sourcePort, unsigned short destinationPort, unsigned int protocol)
	{
		BOOL success;
		CHookingEngine::addConnectionOutgoing(sessionId, ip, sourcePort, destinationPort, protocol, &success);
		return (success==1);
	}

	bool _startSession(unsigned int sessionId, char* key, int len)
	{
		SAFEARRAY * saKey;
		SAFEARRAYBOUND rgsabound[1];

		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = len;
		saKey = SafeArrayCreate(VT_UI1, 1, rgsabound);

		for (LONG i = 0; i < len; i++)
		{
			//TODO: make sure we're not leaking any memory here! -> Use SysFreeString()
			SafeArrayPutElement(saKey, &i, &(key[i]));
		}

		BOOL success;
		CHookingEngine::startSession(sessionId, saKey, &success);
		return (success==1);
	}
};

SOCKET sock;
sockaddr_in localSa;

void initNetwork()
{
	WSADATA wsaData = {0};
    int iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wprintf(L"WSAStartup failed: %d\n", iResult);
        return;
    }

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	localSa.sin_addr.S_un.S_addr = 0x0100007F;
	localSa.sin_family = AF_INET;
	localSa.sin_port = htons(2204);
	bind(sock, (sockaddr*)&localSa, sizeof(localSa)); 
}

const char* buff = "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde";

void threadSendRandomPackets()
{
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	memset(sa.sin_zero, 0, sizeof(sa.sin_zero));

	//create 50 fake connections
	for (int i=0; i<10; i++)
	{
		sa.sin_addr.S_un.S_addr = rand()&0xFFFF | (rand()&0xFFFF)<<16;
		sa.sin_port = htons(rand()&0xFFFF);
		sendto(sock, buff, sizeof(buff), 0, (sockaddr*)&sa, sizeof(sa));
		printf("Thread sent packet.\n");
	}
	printf("Thread done.\n");
}

//main for debugging
int main(int argc, _TCHAR* argv[])
{
	initNetwork();
	HookEngine* he = new HookEngine();
	he->_init();
	he->_hookNetworkIO();

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	memset(sa.sin_zero, 0, sizeof(sa.sin_zero));

	//create 50 fake connections
	for (int i=0; i<50; i++)
	{
		sa.sin_addr.S_un.S_addr = rand()&0xFFFF | (rand()&0xFFFF)<<16;
		sa.sin_port = htons(rand()&0xFFFF);
		sendto(sock, buff, sizeof(buff), 0, (sockaddr*)&sa, sizeof(sa));
	}

	int sessionId = he->_createSession();
	//add last connection to session
	he->_addConnectionOutgoing(sessionId, sa.sin_addr.S_un.S_addr, ntohs(localSa.sin_port), ntohs(sa.sin_port), IPPROTO_UDP);
	char key[16];
	he->_startSession(sessionId,key,(sizeof(key)));

	for (int i = 0; i<10; i++)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadSendRandomPackets, NULL, 0, NULL);
	}

	for (int i=0; i<50; i++)
	{
		DWORD pre = GetTickCount();
		sendto(sock, buff, sizeof(buff), 0, (sockaddr*)&sa, sizeof(sa));
		DWORD post = GetTickCount();

		printf("Sending encrypted packet took %dms.\n", post-pre);
	}
	printf("Done.\n");
	getchar();
	return 0;
}
#endif


