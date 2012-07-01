// Crypto.cpp : Implementation of CCrypto

#include "stdafx.h"

#include "Crypto.h"

#include <stdio.h>
#include <string>
#include <sstream>
#include "stdlib.h"

#include "libOTR.h"
#include "Logger.h"
#include "User.h"
#include "HelperFunctions.h"
#include "OTRCompaFileIO.h"
#include "OTRCallbacks.h"
#include "Event.h"

enum ActionType {
	FATAL_ERROR = 0,
	NOTHING = 1,
	POLL_GENERIC = 2,
	POLL_SPECIFIC = 4
};

const char KEY_TOKEN[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// CCrypto
OTRCallbacks* otr;
LocalUser* user;
std::vector<Event<EventTypeGeneric>> genericEvents;

//file access functions
errno_t (__cdecl* OTRCompaFileIO::_wfopen_s) (FILE** pFile, const wchar_t *filename,  const wchar_t *mode);
int (__cdecl* OTRCompaFileIO::fclose)(FILE *stream);

//libgcrypt functions (needed due to strange vs2010 linker behaviour)
#define PATH_LIBGCRYPT "libgcrypt-11.dll"
int (__cdecl* gcry_mpi_cmp) (const gcry_mpi_t u, const gcry_mpi_t v);
gcry_error_t (__cdecl* gcry_cipher_decrypt) (gcry_cipher_hd_t h,
                                  void *out, size_t outsize,
                                  const void *in, size_t inlen);

STDMETHODIMP CCrypto::init(BSTR accountName, BSTR baseDir, int* nextAction)
{
	*nextAction = FATAL_ERROR;

	Logger::init(baseDir);
	Logger::reset();

	//load functions pointers from libOTR's runtime lib
	if (OTRCompaFileIO::init() != true)
	{
		Logger::log(L"Fatal Error: Failed to import file-io APIs.");
		return S_FALSE;
	}

	//load function pointers to libgcrypt functions
	HMODULE hLibgcrypt = LoadLibraryA(PATH_LIBGCRYPT);
	if (hLibgcrypt == NULL)
	{
		Logger::log(L"Fatal Error: Failed to load libgcrypt-11.dll.");
		return S_FALSE;
	}

	gcry_mpi_cmp = (int(__cdecl*)(const gcry_mpi_t u, const gcry_mpi_t v))GetProcAddress(hLibgcrypt, "gcry_mpi_cmp");
	if (gcry_mpi_cmp == NULL)
	{
		Logger::log(L"Fatal Error: Failed to load function from libgcrypt-11.dll.");
		return S_FALSE;
	}

	gcry_cipher_decrypt = (gcry_error_t(__cdecl*)(gcry_cipher_hd_t h, void *out, size_t outsize, const void *in, size_t inlen))GetProcAddress(hLibgcrypt, "gcry_cipher_decrypt");
	if (gcry_cipher_decrypt == NULL)
	{
		Logger::log(L"Fatal Error: Failed to load function from libgcrypt-11.dll.");
		return S_FALSE;
	}

	std::wstring name = accountName;
	std::wstring tmpLog = L"Initing OTR for user: ";
	tmpLog += name;
	Logger::log(tmpLog);

	
	//check for backslashes in name - don't want any directory traversal!
	if ((name.find(L"\\") != std::wstring::npos) || (name.find(L"/") != std::wstring::npos))
	{
		Logger::log(L"Fatal Error: Name is of wrong format.");
		return S_FALSE;
	}
	
	OTRL_INIT;

	OtrlUserState userState = otrl_userstate_create();
	if (userState == NULL)
	{
		Logger::log(L"Fatal Error: Failed to create OTR userstate. This should never happen.");
		return S_FALSE;
	}

	user = new LocalUser(HelperFunctions::wstringToString(name), userState, baseDir);
	otr = new OTRCallbacks(user);

	//try to open key-file
	FILE* privKeyFile;
	errno_t fileError = OTRCompaFileIO::_wfopen_s(&privKeyFile, user->getPathPrivKeyFile().c_str(), L"rb");
	if (fileError != 0)
	{
		switch (fileError)
		{
			case ENOENT:
			{
				//no key-file present, ask user to create one!
				Logger::log(L"Info: No private key-file found for the given user. Need to create one...");

				Event<EventTypeGeneric> e(DLG_NO_PRIVATE_KEY_FOUND, "");
				genericEvents.insert(genericEvents.begin(), e);

				*nextAction = POLL_GENERIC;
				return S_OK;
			}
			
			default:
				//looks like the file is opened by another process or we got no access rights
				Logger::log(L"Error: Can not access the private key file.");
				return S_FALSE;
		}
		
	}

	//key-file is present, give to otr...
	gcry_error_t error = otrl_privkey_read_FILEp(user->getState(), privKeyFile);
	OTRCompaFileIO::fclose(privKeyFile);
	if (error != GPG_ERR_NO_ERROR)
	{
		Logger::log(L"Error: The given private key-file appears to be invalid. Need to create a new one...");

		Event<EventTypeGeneric> e(DLG_PRIVATE_KEY_INVALID, "");
		genericEvents.insert(genericEvents.begin(), e);

		*nextAction = POLL_GENERIC;
		return S_OK;
	}

	//the key-file was successfully read - now read the corresponding fingerprint file...
	FILE* fingerprintsFile;
	fileError = OTRCompaFileIO::_wfopen_s(&fingerprintsFile, user->getPathFingerprintsFile().c_str(), L"rb");
	if (fileError != 0)
	{
		switch (fileError)
		{
			case ENOENT:
			{
				//no fingerprints available
				Logger::log(L"Info: No fingerprint-file found. Need to create a new one...");

				*nextAction = POLL_GENERIC;
				Event<EventTypeGeneric> e(DLG_NO_FINGERPRINTS_FOUND, "");
				genericEvents.insert(genericEvents.begin(), e);
				return S_OK;
			}
			
			default:
				//looks like the file is opened by another process or we got no access rights
				Logger::log(L"Error: Can not access the fingerprints file.");
				return S_FALSE;
		}
		
	}
	else
	{
		//fingerprints file available
		error = otrl_privkey_read_fingerprints_FILEp(user->getState(), fingerprintsFile, NULL, NULL);
		OTRCompaFileIO::fclose(fingerprintsFile);
		if (error != GPG_ERR_NO_ERROR)
		{
			Logger::log(L"Error: The given fingerprints-file appears to be invalid. Need to create a new one...");
			*nextAction = DLG_FINGERPRINTS_INVALID;

			Event<EventTypeGeneric> e(DLG_FINGERPRINTS_INVALID, "");
			genericEvents.insert(genericEvents.begin(), e);

			*nextAction = POLL_GENERIC;
			return S_OK;
		}
	}
	
	*nextAction = NOTHING;
	return S_OK;
}


STDMETHODIMP CCrypto::generatePrivateKey(int* nextAction)
{
	//BREAKPOINT
	*nextAction = FATAL_ERROR;

	std::wstring tmpLog = L"Info: Going to write private key-file to: ";
	tmpLog += user->getPathPrivKeyFile();
	Logger::log(tmpLog.c_str());

	//open keyfile
	FILE* privKeyFile;
	errno_t fileError = OTRCompaFileIO::_wfopen_s(&privKeyFile, user->getPathPrivKeyFile().c_str(), L"w+");
	if (fileError != 0)
	{
		std::wstring tmpLog = L"Fatal Error: Could not create new private key-file in dir: ";
		wchar_t dir[512];
		GetCurrentDirectory(512,dir);
		tmpLog += dir;

		Logger::log(tmpLog.c_str());
		return S_FALSE;
	}

	//write key
	std::string userName = user->getName();

	gcry_error_t error = otrl_privkey_generate_FILEp(user->getState(), privKeyFile, userName.c_str(), OTR_PROTOCOL_NAME);
	OTRCompaFileIO::fclose(privKeyFile);
	if (error != GPG_ERR_NO_ERROR)
	{
		Logger::log(L"Fatal Error: Could not create a new private key.");
		return S_FALSE;
	}

	//reopen keyfile
	fileError = OTRCompaFileIO::_wfopen_s(&privKeyFile, user->getPathPrivKeyFile().c_str(), L"rb");
	if (fileError != 0)
	{
		std::wstring tmpLog = L"Fatal Error: Failed to reopen just created private key file. This should never happen.";
		return S_FALSE;
	}

	//read keyfile
	error = otrl_privkey_read_FILEp(user->getState(), privKeyFile);
	OTRCompaFileIO::fclose(privKeyFile);
	if (error != GPG_ERR_NO_ERROR)
	{
		Logger::log(L"Fatal Error: The just created private key-file appears to be invalid. This should never happen.");
		return S_FALSE;
	}

	//the key-file was successfully read - now read the corresponding fingerprint file...
	FILE* fingerprintsFile;
	fileError = OTRCompaFileIO::_wfopen_s(&fingerprintsFile, user->getPathFingerprintsFile().c_str(), L"rb");
	if (fileError != 0)
	{
		switch (fileError)
		{
			case ENOENT:
			{
				//no fingerprints available
				Logger::log(L"Info: No fingerprint-file found. Need to create a new one...");

				*nextAction = POLL_GENERIC;
				Event<EventTypeGeneric> e(DLG_NO_FINGERPRINTS_FOUND, "");
				genericEvents.insert(genericEvents.begin(), e);
				return S_OK;
			}
			
			default:
				//looks like the file is opened by another process or we got no access rights
				Logger::log(L"Error: Can not access the fingerprints file.");
				return S_FALSE;
		}
	}
	else
	{
		//fingerprints file available
		error = otrl_privkey_read_fingerprints_FILEp(user->getState(), fingerprintsFile, NULL, NULL);
		OTRCompaFileIO::fclose(fingerprintsFile);
		if (error != GPG_ERR_NO_ERROR)
		{
			Logger::log(L"Error: The given fingerprints-file appears to be invalid. Need to create a new one...");
			*nextAction = DLG_FINGERPRINTS_INVALID;
			return S_OK;
		}
	}

	*nextAction = NOTHING;
	return S_OK;
}


STDMETHODIMP CCrypto::messageReceived(BSTR partner, BSTR message, BSTR* plain, BOOL* deploy, int* nextAction)
{
	*nextAction = FATAL_ERROR;
	std::string tempPlain;
	bool tempDeploy;

	std::string sPartner = HelperFunctions::wstringToString((wstring)partner);
	std::string sMessage = HelperFunctions::wstringToString((wstring)message);

	//check if partner is already known
	Conversation* conv;
	if (otr->getConversation(sPartner, &conv) == false)
	{
		//... if not, open a new conversation
		otr->openConversation(sPartner);
	}

	bool success = otr->decryptMessage(sPartner, sMessage, tempDeploy);
	
	if (!success)
	{
		return S_FALSE;
	}

	*deploy = (BOOL) tempDeploy;
	*nextAction = POLL_SPECIFIC;

	*plain = SysAllocString((const OLECHAR*)HelperFunctions::stringToWstring(sMessage).c_str());

	return S_OK;
}


STDMETHODIMP CCrypto::messageSending(BSTR partner, BSTR message, BSTR* cipher, int* nextAction)
{
	*nextAction = FATAL_ERROR;

	std::string sMessage = HelperFunctions::wstringToString((wstring)message);
	std::string sPartner = HelperFunctions::wstringToString((wstring)partner);

	//check if partner is already known
	Conversation* conv;
	if (otr->getConversation(sPartner, &conv) == false)
	{
		//...if not, open a new conversation
		otr->openConversation(sPartner);
	}

	bool success = otr->encryptMessage(sPartner, sMessage);
	
	if (!success)
	{
		return S_FALSE;
	}

	*nextAction = POLL_SPECIFIC;
	*cipher = SysAllocString((const OLECHAR*)HelperFunctions::stringToWstring(sMessage).c_str());

	return S_OK;
}

/*****************************
* Specific polling functions *
******************************/

STDMETHODIMP CCrypto::getNextMessage(BSTR partner, BSTR* message, int* messagesLeft)
{
	Conversation* conv;
	if (otr->getConversation(HelperFunctions::wstringToString((wstring)partner), &conv) == false)
	{
		return S_FALSE;
	}
	*messagesLeft = conv->getSizeOutgoingMessageCache(); 
	
	if (*messagesLeft >= 1)
	{
		std::wstring wsMessage = HelperFunctions::stringToWstring(conv->getLastOutgoingMessage());
		*message = SysAllocString((const OLECHAR*)wsMessage.c_str());
	}

	return S_OK;
}


STDMETHODIMP CCrypto::getNextNotification(BSTR partner, BSTR* notification, int* notificationsLeft)
{
	Conversation* conv;
	if (otr->getConversation(HelperFunctions::wstringToString((wstring)partner), &conv) == false)
	{
		return S_FALSE;
	}
	*notificationsLeft = conv->getSizeNotificationCache();

	if (*notificationsLeft >= 1)
	{
		std::wstring wsNotification = HelperFunctions::stringToWstring(conv->getLastNotification());
		*notification = SysAllocString((const OLECHAR*)wsNotification.c_str());
	}

	return S_OK;
}


STDMETHODIMP CCrypto::getNextEvent(BSTR partner, int* eventType, BSTR* eventData, int* eventsLeft)
{
	Conversation* conv;
	if (otr->getConversation(HelperFunctions::wstringToString((wstring)partner), &conv) == false)
	{
		return S_FALSE;
	}
	*eventsLeft = conv->getSizeEventCache();

	if (*eventsLeft >= 1)
	{
		Event<EventTypeSpecific> e = conv->getLastEvent();
		*eventType = e.type;
		std::wstring wsEventData = HelperFunctions::stringToWstring(e.data);
		*eventData = SysAllocString((const OLECHAR*)wsEventData.c_str());
	}

	return S_OK;
}

/****************************
* Generic polling functions *
*****************************/

STDMETHODIMP CCrypto::getNextEventGeneric(int* eventType, BSTR* eventData, int* eventsLeft)
{
	*eventsLeft = genericEvents.size();

	if (*eventsLeft >= 1)
	{
		Event<EventTypeGeneric> e = genericEvents.back();
		genericEvents.pop_back();

		*eventType = e.type;
		*eventData = SysAllocString((const OLECHAR*)e.data.c_str());
	}

	return S_OK;
}


STDMETHODIMP CCrypto::startSession(BSTR partner, int* nextAction)
{
	*nextAction = FATAL_ERROR;
	std::string sPartner = HelperFunctions::wstringToString((wstring)partner);
	if (otr->openConversation(sPartner) != true)
	{
		return S_FALSE;
	}

	if (otr->startAKE(HelperFunctions::wstringToString((wstring)partner)) != true)
	{
		otr->closeConversation(sPartner);
		return S_FALSE;
	}

	*nextAction = POLL_SPECIFIC;
	return S_OK;
}


STDMETHODIMP CCrypto::endSession(BSTR partner, int* nextAction)
{
	*nextAction = FATAL_ERROR;
	if (otr->closeConversation(HelperFunctions::wstringToString((wstring)partner)) == false)
	{
		return S_FALSE;
	}

	*nextAction = POLL_SPECIFIC;
	return S_OK;
}


STDMETHODIMP CCrypto::createFingerprintStore(int* nextAction)
{
	*nextAction = FATAL_ERROR;

	FILE* fingerprintsFile;
	errno_t fileError = OTRCompaFileIO::_wfopen_s(&fingerprintsFile, user->getPathFingerprintsFile().c_str(), L"w+");
	if (fileError != 0)
	{
		//TODO: Give feedback to user
		Logger::log(L"Fatal Error: Could neither read, nor create a fingerprint file.");
		return S_FALSE;
	}

	gcry_error_t error = otrl_privkey_write_fingerprints_FILEp(user->getState(), fingerprintsFile);
	OTRCompaFileIO::fclose(fingerprintsFile);
	if (error != GPG_ERR_NO_ERROR)
	{
		//TODO: Give feedback to user
		Logger::log(L"Fatal Error: OTR failed to create new fingerprints file.");
		return S_FALSE;
	}

	*nextAction = NOTHING;
	return S_OK;
}


STDMETHODIMP CCrypto::getOwnFingerprint(BSTR* fingerprint, BOOL* success)
{
	*success = false;

	char fp[45];
	if (otrl_privkey_fingerprint(user->getState(), fp, user->getName().c_str(), OTR_PROTOCOL_NAME) == NULL)
	{
		Logger::log(L"Error: Failed to get fingerprint of own private key.");
		return S_FALSE;
	}

	*success = true;
	std::wstring wsFp = HelperFunctions::stringToWstring((std::string)fp);
	*fingerprint = SysAllocString((const OLECHAR*)wsFp.c_str());

	return S_OK;
}


STDMETHODIMP CCrypto::confirmFingerprint(BSTR partner, BSTR fingerprint, int* nextAction)
{
	*nextAction = FATAL_ERROR;
	std::string sFp = HelperFunctions::wstringToString((wstring) fingerprint);
	std::string sPartner = HelperFunctions::wstringToString((wstring) partner);
	if (otr->confirmFingerprint(sPartner, sFp) != true)
	{
		Logger::log(L"Fatal Error: Failed to confirm fingerprint.");
		return S_FALSE;
	}

	*nextAction = NOTHING;
	return S_OK;
}


STDMETHODIMP CCrypto::getSessionKey(BSTR partner, SAFEARRAY* * saKey, BOOL* success)
{
	*success = false;
	std::string sPartner = HelperFunctions::wstringToString((wstring) partner);

	//get corresponding context
	ConnContext* cc = otrl_context_find(user->getState(), sPartner.c_str(), user->getName().c_str(), OTR_PROTOCOL_NAME, 0, NULL, NULL, NULL);
	if (cc == NULL)
	{
		Logger::log(L"Error: Could not get user-context.");
		return S_FALSE;
	}

#ifdef DEBUG
	std::wstringstream tmp;
	tmp << "Info: Going to derive a session key from OTR short-term keys. Our key-id: " << cc->our_keyid << ", their key-id: " << cc->their_keyid;
	Logger::log(tmp.str());
#endif

	char key[sizeof(KEY_TOKEN)]; //128 bits

	/**
	* NOTE: We're using their_y but our_old_dh_key.pub (and not our_dh_key.pub), because key-ids
	* are out of sync at this point (ours: 2, theirs: 1).
	* 
	* TODO: Write code that compares the key-ids.
	**/

	//determine which key to use
	gcry_error_t error;
	int cmp = gcry_mpi_cmp(cc->our_old_dh_key.pub, cc->their_y);

	if (cmp > 0)
	{
#ifdef DEBUG
		Logger::log("Info: This is the high-end for key derivation.");
#endif
		error = gcry_cipher_decrypt(cc->sesskeys[1][0].sendenc, key, sizeof(key), KEY_TOKEN, sizeof(KEY_TOKEN));
	}
	else
	{
#ifdef DEBUG
		Logger::log("Info: This is the low-end for key derivation.");
#endif
		error = gcry_cipher_decrypt(cc->sesskeys[1][0].rcvenc, key, sizeof(key), KEY_TOKEN, sizeof(KEY_TOKEN));
	}

	if (error != GPG_ERR_NO_ERROR)
	{
		std::wstringstream tmp;
		tmp << L"Error: Could not generate session key. Error: " << std::dec << error;
		Logger::log(tmp.str());
		return S_FALSE;
	}

#ifdef DEBUG
	Logger::log(L"Info: Successfully called libgcrypt imports.");

	DWORD* pKey = (DWORD*)key;
	std::wstringstream tmp1;
	tmp1 << L"Info: The generated key: " << std::hex << pKey[0] << std::hex << pKey[1] << std::hex << pKey[2] << std::hex << pKey[3];
	Logger::log(tmp1.str());
#endif

	//translate key into safearray
	SAFEARRAY * psa;
	SAFEARRAYBOUND rgsabound[1];

	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = sizeof(key);
	psa = SafeArrayCreate(VT_UI1, 1, rgsabound);

	for (int i = 0; i < sizeof(key); i++)
	{
		SafeArrayPutElement(psa, (LONG*)&i, &key[i]);
	}

	*saKey = psa;
	*success = true;
	return S_OK;
}


STDMETHODIMP CCrypto::getFingerprint(BSTR partner, BSTR* fingerprint, BOOL* success)
{
	*success = false;
	std::string sPartner = HelperFunctions::wstringToString((wstring) partner);

	//get corresponding context
	ConnContext* cc = otrl_context_find(user->getState(), sPartner.c_str(), user->getName().c_str(), OTR_PROTOCOL_NAME, 0, NULL, NULL, NULL);
	if (cc == NULL)
	{
		Logger::log(L"Error: Could not get user-context.");
		return S_FALSE;
	}

	char fp[45];
	otrl_privkey_hash_to_human(fp, cc->active_fingerprint->fingerprint);

	*success = true;
	std::wstring wsFp = HelperFunctions::stringToWstring((std::string)fp);
	*fingerprint = SysAllocString((const OLECHAR*)wsFp.c_str());

	return S_OK;
}
