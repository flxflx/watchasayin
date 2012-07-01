// HookingEngine.h : Declaration of the CHookingEngine

#pragma once
#include "resource.h"       // main symbols


#include "Bottom_i.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

enum TTrafficDirection {
	INCOMING,
	OUTGOING
};

// CHookingEngine
//! Class made available via COM exposing hooking and session encryption related functionality.
class ATL_NO_VTABLE CHookingEngine :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CHookingEngine, &CLSID_HookingEngine>,
	public IDispatchImpl<IHookingEngine, &IID_IHookingEngine, &LIBID_BottomLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CHookingEngine()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_HOOKINGENGINE)


BEGIN_COM_MAP(CHookingEngine)
	COM_INTERFACE_ENTRY(IHookingEngine)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

private:
	SAFEARRAY* getOpenConnectionsAsSafeArray(TTrafficDirection direction);

public:

	//! Hooks the network API functions of the process.
	STDMETHOD(hookNetworkIO)(BOOL* success);
	//! Unhooks the network API functions of the process.
	STDMETHOD(unhookNetworkIO)(BOOL* success);
	//! Gets a list of outgoing connections.
	STDMETHOD(getOpenConnectionsOutgoing)(SAFEARRAY* * connections);
	//! Gets a list of incoming connections.
	STDMETHOD(getOpenConnectionsIncoming)(SAFEARRAY* * connections);
	//! Resets the both incoming and outgoing connection lists (should not be called).
	STDMETHOD(resetConnections)(BOOL* success);
	//! Adds an outgoing connection to a certain session.
	STDMETHOD(addConnectionOutgoing)(unsigned int sessionId, unsigned int ip, unsigned short sourcePort, unsigned short destinationPort, unsigned int protocol, BOOL* success);
	//! Adds an incoming connection to a certain session.
	STDMETHOD(addConnectionIncoming)(unsigned int sessionId, unsigned int ip, unsigned short sourcePort, unsigned short destinationPort, unsigned int protocol, BOOL* success);
	/*! Creates a new session
		\param expandable A flag indicating whether new connections which are "similar" to connections already belonging to the to be 
		created session should be added to it on-the-fly once it is active.
		\returns The id of the newly created session.
	*/
	STDMETHOD(createSession)(BOOL expandable, unsigned int* sessionId);
	//! Starts a given session.
	STDMETHOD(startSession)(unsigned int sessionId, SAFEARRAY * key, BOOL* success);
	//! Destroys a given session.
	STDMETHOD(destroySession)(unsigned int sessionId, BOOL* success);
	//! Inits the module.
	STDMETHOD(init)(BSTR baseDir, BOOL* success);
	//! Sets the security policy of the module to either VERY_LOOSE, LOOSE or TIGHT.
	STDMETHOD(setSecurityPolicy)(int securityPolicy, BOOL* success);
	STDMETHOD(startCountingConnections)(BOOL* success);
	STDMETHOD(stopCountingConnections)(BOOL* success);
	STDMETHOD(getMostUsedPeerIP)(unsigned int* ip);
};

OBJECT_ENTRY_AUTO(__uuidof(HookingEngine), CHookingEngine)
