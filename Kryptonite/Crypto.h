// Crypto.h : Declaration of the CCrypto

#pragma once
#include "resource.h"       // main symbols



#include "Kryptonite_i.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CCrypto
//! Class made available via COM exposing all the functionality needed to secure a given conversation using an AKE and symmetric crypto.
class ATL_NO_VTABLE CCrypto :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCrypto, &CLSID_Crypto>,
	public IDispatchImpl<ICrypto, &IID_ICrypto, &LIBID_KryptoniteLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CCrypto()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CRYPTO)


BEGIN_COM_MAP(CCrypto)
	COM_INTERFACE_ENTRY(ICrypto)
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

public:
	/*! Inits the module.
		\param accountName The name of the principal account.
		\param baseDir The path of the working directory of the module (e.g. for storing/loading fingerprints).
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(init)(BSTR accountName, BSTR baseDir, int* nextAction);

	/*! Generates a new private key for the principal account.
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(generatePrivateKey)(int* nextAction);

	/*! Tells the module that a message was received from the conversation partner with the given name.
		\param partner The name of the conversation partner.
		\param message The received message.
		\param plain [returns] The message in decrypted form.
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(messageReceived)(BSTR partner, BSTR message, BSTR* plain, BOOL* deploy, int* nextAction);

	/*! Informs the module on a message that the is to be sent to the given conversation partner.
		\param partner The name of the conversation partner.
		\param message The message to be sent.
		\param cipher [returns] The message in encrypted form.
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(messageSending)(BSTR partner, BSTR message, BSTR* cipher, int* nextAction);

	//! Gets the next message to be sent to the given conversation partner.
	STDMETHOD(getNextMessage)(BSTR partner, BSTR* message, int* messagesLeft);

	//! Gets the next notification to be displayed in context with  the conversation with the given partner.
	STDMETHOD(getNextNotification)(BSTR partner, BSTR* notification, int* notificationsLeft);

	//! Gets the next event to be processed in the context of the conversation with the given partner.
	STDMETHOD(getNextEvent)(BSTR partner, int* eventType, BSTR* eventData, int* eventsLeft);

	//! Gets the next to be processed global event.
	STDMETHOD(getNextEventGeneric)(int* eventType, BSTR* eventData, int* eventsLeft);

	/*! Starts a new session with the given conversation partner (will init an AKE).
		\param partner The name of the conversation partner.
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(startSession)(BSTR partner, int* nextAction);

	/*! Ends a certain session.
		\param partner The name of the partner whose session is to be closed.
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(endSession)(BSTR partner, int* nextAction);

	/*! Create a new fingerprint store for the principal account.
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(createFingerprintStore)(int* nextAction);

	//! Gets the principal account's own fingerprint.
	STDMETHOD(getOwnFingerprint)(BSTR* fingerprint, BOOL* success);

	/*! Confirms the given fingerprint for the given conversation partner.
		\param partner The name of the conversation partner.
		\param fingerprint The fingerprint which is confirmed.
		\param nextAction [returns] The next action to be conducted. */
	STDMETHOD(confirmFingerprint)(BSTR partner, BSTR fingerprint, int* nextAction);

	/*! Gets a cryptographically secure symmetric key which was computed in an AKE with the given conversation partner.
		\param partner The name of the conversation partner.
		\param key [returns] The key as a stream of bytes.
		\param success [returns] A flag indicating success. */
	STDMETHOD(getSessionKey)(BSTR partner, SAFEARRAY* * key, BOOL* success);

	/*! Gets the fingerprint of the given conversation partner.
		\param partner The name of the conversation partner.
		\param fingerprint [returns] The fingerprint as a string.
		\param success [returns] A flag indicating success. */
	STDMETHOD(getFingerprint)(BSTR partner,BSTR* fingerprint, BOOL* success);
};

OBJECT_ENTRY_AUTO(__uuidof(Crypto), CCrypto)
