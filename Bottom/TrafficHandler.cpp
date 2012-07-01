#include "StdAfx.h"
#include "TrafficHandler.h"
#include <string>
#include <sstream>
#include "Logger.h"

#define LOCALHOST 0x0100007F
#define WAIT_TIME_FIRST_ENC_PACKET 50

TrafficHandler::TrafficHandler(const TSecurityPolicy securityPolicy)
{
	InitializeCriticalSection(&csIncoming);
	InitializeCriticalSection(&csOutgoing);
	InitializeCriticalSection(&csGeneric);

	this->securityPolicy = securityPolicy;
	this->countConnections = false;
}

TrafficHandler::~TrafficHandler()
{
	DeleteCriticalSection(&csIncoming);
	DeleteCriticalSection(&csOutgoing);
	DeleteCriticalSection(&csGeneric);
}

bool TrafficHandler::packetToSend(const Connection &conn, const char* payload, const unsigned int lenPayload, char** newPayload, unsigned int& lenNewPayload, const unsigned int maximumPacketSize)
{
	//TODO: refactor
	//check if this packet is to be sent to localhost
	unsigned int ip = conn.getIp().S_un.S_addr;
	if (ip == LOCALHOST)
	{
		//just copy the buffer
		if (((*newPayload) = (char*)malloc(lenPayload)) == NULL)
		{
			return false;
		}
		memcpy(*newPayload, payload, lenPayload);
		lenNewPayload = lenPayload;
		return true;
	}

	EnterCriticalSection(&csGeneric);

	TSecurityPolicy sp = securityPolicy;
	int numActiveSessions = encSessions.size();

	updateConnectionCounts(ip);

	LeaveCriticalSection(&csGeneric);

	//save connection information
	EnterCriticalSection(&csOutgoing);

	//check if connection shall be dropped right away
	if ((openConnectionsSend.contains(conn) == false) && (numActiveSessions > 0) && ((int)sp >= (int)LOOSE))
	{
		//check if the connection is similar one belonging to an outgoing session
		LeaveCriticalSection(&csOutgoing);
		EnterCriticalSection(&csGeneric);

		bool similarConnection = false;
		std::map<unsigned int, Session>::iterator iter;
		for (iter = encSessions.begin(); iter != encSessions.end(); iter++)
		{
			//check if the session should be expanded dynamically
			if (iter->second.expand() == true)
			{
				if (iter->second.searchOutgoing(conn) == SIMILAR_FOUND)
				{
					iter->second.addOutgoingConnection(conn);
					similarConnection = true;
#ifdef DEBUG
					Logger::log(L"Info: Added outgoing connection to active session.");
#endif
				}
			}	
		}
		LeaveCriticalSection(&csGeneric);

		//if connection is not similar to any connection belonging to an active session, drop it!
		if (similarConnection == false)
		{
			return false;
		}
		EnterCriticalSection(&csOutgoing);
	}
	openConnectionsSend.addConnection(conn);

	LeaveCriticalSection(&csOutgoing);

	//check wether to encrypt packet before sending
	unsigned int sessionId;
	bool toEncrypt = ownedBySessionOutgoing(conn, sessionId); 

	//nothing to encrypt
	if (toEncrypt == false)
	{	
		//check security policy whether this packet has to be dropped or not
		if ((sp == TIGHT) && (numActiveSessions > 0))
		{
			return false;
		}

		//just copy the buffer
		if (((*newPayload) = (char*)malloc(lenPayload)) == NULL)
		{
			return false;
		}
		memcpy(*newPayload, payload, lenPayload);
		lenNewPayload = lenPayload;
		return true;
	}
	return encrypt(sessionId, (const BYTE*)payload, lenPayload, (BYTE**)newPayload, lenNewPayload, maximumPacketSize);
}

bool TrafficHandler::packetReceived(const Connection &conn, char* payload, unsigned int& lenPayload)
{
	EnterCriticalSection(&csIncoming);
	openConnectionsReceive.addConnection(conn);
	LeaveCriticalSection(&csIncoming);

	//check whether this connections needs to be decrypted
	unsigned int sessionId;
	bool toDecrypt = false;
	
	EnterCriticalSection(&csGeneric);
	updateConnectionCounts(conn.getIp().S_un.S_addr);

	std::map<unsigned int, Session>::iterator iter;
	for (iter = encSessions.begin(); iter != encSessions.end(); iter++)
	{
		//check whether the session needs to be expanded
		TSearchResult r = iter->second.searchIncoming(conn);
		if (r == SIMILAR_FOUND && iter->second.expand())
		{
			iter->second.addOutgoingConnection(conn);
#ifdef DEBUG
			Logger::log(L"Info: Added incoming connection to active session.");
#endif
			toDecrypt = true;
			sessionId = iter->second.getId();
			break;
		}

		if (r == MATCH_FOUND)
		{
			toDecrypt = true;
			sessionId = iter->second.getId();
			break;
		}
	}
	LeaveCriticalSection(&csGeneric);

	//nothing to decrypt
	if (toDecrypt == false || lenPayload == 0)
	{
		return true;
	}
	
	return decrypt(sessionId, (BYTE*)payload, lenPayload);
}

bool TrafficHandler::ownedBySessionIncoming(const Connection& conn, unsigned int& sessionId)
{
	EnterCriticalSection(&csGeneric);

	bool owned = false;
	std::map<unsigned int, Session>::iterator iter;
	for (iter = encSessions.begin(); iter != encSessions.end(); iter++)
	{
		if (iter->second.containsIncoming(conn) == true)
		{
			owned = true;
			sessionId = iter->second.getId();
			break;
		}
	}
	LeaveCriticalSection(&csGeneric);
	return owned;
}

bool TrafficHandler::ownedBySessionOutgoing(const Connection& conn, unsigned int& sessionId)
{
	EnterCriticalSection(&csGeneric);

	bool owned = false;
	std::map<unsigned int, Session>::iterator iter;
	for (iter = encSessions.begin(); iter != encSessions.end(); iter++)
	{
		if (iter->second.containsOutgoing(conn) == true)
		{
			owned = true;
			sessionId = iter->second.getId();
			break;
		}
	}
	LeaveCriticalSection(&csGeneric);
	return owned;
}

void TrafficHandler::getOutgoingConnections(map<unsigned int,IPConnection> &connections)
{
	EnterCriticalSection(&csOutgoing);
	//copy outgoing connections map
	openConnectionsSend.getConnections(connections);
	LeaveCriticalSection(&csOutgoing);
}

void TrafficHandler::getIncomingConnections(map<unsigned int,IPConnection> &connections)
{
	EnterCriticalSection(&csIncoming);
	//copy incoming connections map
	openConnectionsReceive.getConnections(connections);
	LeaveCriticalSection(&csIncoming);
}

void TrafficHandler::resetOugoingConnections()
{
	EnterCriticalSection(&csIncoming);
	openConnectionsReceive.clear();
	LeaveCriticalSection(&csIncoming);
}

void TrafficHandler::resetIncomingConnections()
{
	EnterCriticalSection(&csOutgoing);
	openConnectionsSend.clear();
	LeaveCriticalSection(&csOutgoing);
}

void TrafficHandler::addEncryptedSession(const Session& session)
{
	EnterCriticalSection(&csGeneric);
	encSessions[session.getId()] = session;
	setUpEncryptedSession(session);
	LeaveCriticalSection(&csGeneric);
}
	
void TrafficHandler::removeEncryptedSession(const Session& session)
{
	EnterCriticalSection(&csGeneric);
	encSessions.erase(session.getId());
	tearDownEncryptedSession(session);
	LeaveCriticalSection(&csGeneric);
}

TSecurityPolicy TrafficHandler::getSecurityPolicy() const
{
	return securityPolicy;
}

bool TrafficHandler::setSecurityPolicy(const TSecurityPolicy securityPolicy)
{
	EnterCriticalSection(&csGeneric);

	this->securityPolicy = securityPolicy;

	LeaveCriticalSection(&csGeneric);
	return true;
}

void TrafficHandler::updateConnectionCounts(const unsigned int ip)
{
	if (this->countConnections == true)
	{
		if (this->connectionCounts.find(ip) == this->connectionCounts.end())
		{
			this->connectionCounts[ip] = 0;
		}
		this->connectionCounts[ip]++;
	}
}

void TrafficHandler::setCountConnections(const bool enable)
{
	if (this->countConnections == false && enable == true)
	{
		EnterCriticalSection(&csGeneric);

		this->connectionCounts.clear();
		this->countConnections = true;

		LeaveCriticalSection(&csGeneric);
	}

	else if (this->countConnections == true && enable == false)
	{
		this->countConnections = false;
	}
}

void TrafficHandler::getConnectionCounts(map<unsigned int,unsigned int> &connectionCounts)
{
	EnterCriticalSection(&csGeneric);
	connectionCounts = this->connectionCounts;
	LeaveCriticalSection(&csGeneric);
}


TrafficHandlerMSCSP::TrafficHandlerMSCSP(const TSecurityPolicy securityPolicy) : TrafficHandler(securityPolicy)
{
	noErrors = false;
	InitializeCriticalSection(&csKeys);

	if (CryptAcquireContext(&hCSP, L"BOTTOM", MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_DELETEKEYSET) == 0)  
	{
		Logger::log(L"Error: Failed to delete crypto context.");
	}

	//get context
	if (CryptAcquireContext(&hCSP, L"BOTTOM", MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET | CRYPT_SILENT) == 0)  
	{
		Logger::log(L"Fatal Error: Could not acquire crypto context.");
		return;
	}

	noErrors = true;
}

TrafficHandlerMSCSP::~TrafficHandlerMSCSP()
{
	CryptReleaseContext(hCSP, 0);
	DeleteCriticalSection(&csKeys);
}

bool TrafficHandlerMSCSP::encrypt(const unsigned int sessionId, const BYTE* plain, const unsigned int lenPlain, BYTE** cipher, unsigned int& lenCipher, const unsigned int maximumPacketSize)
{
	if ((lenPlain&0x80000000) != 0)
	{
		Logger::log(L"Error: Could not encrypt message, because plaintext is too long.");
		return false;
	}

	EnterCriticalSection(&csKeys);
	if (keys.find(sessionId) == keys.end())
	{
		Logger::log(L"Error: Could not find a key for encryption.");
		LeaveCriticalSection(&csKeys);
		return false;
	}
	HCRYPTKEY hKey = keys[sessionId];
	LeaveCriticalSection(&csKeys); 
	
	unsigned int lenBuffer;
	unsigned int lenBlock;
	unsigned int tmpLen = sizeof(lenBlock);
	if (CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&lenBlock, (DWORD*)&tmpLen, 0) != TRUE)
	{
		Logger::log(L"Error: Could not get block length of cipher.");
		return false;
	}

	//round up to next block boundary
	lenBuffer = ((lenPlain / lenBlock) +2) * lenBlock;

	*cipher = (BYTE*)malloc(lenBuffer);
	if (*cipher == NULL)
	{
		Logger::log(L"Error: Failed to allocate enough memory for encrypting packet.");
		return false;
	}

	memcpy(*cipher, plain, lenPlain);
	lenCipher = lenPlain;

	if (CryptEncrypt(hKey, NULL, TRUE, 0, *cipher, (DWORD*)&lenCipher, lenBuffer) == false)
	{
#ifdef DEBUG
		std::wstringstream tmp;
		tmp << L"Error: Could not encrypt packet. Error: " << std::hex << GetLastError();
		Logger::log(tmp.str());
#else
		Logger::log(L"Error: Decryption of packet failed.");
#endif
		return false;
	}

#ifdef DEBUG
	std::wstringstream tmp;
	tmp << L"Info: Encrypted packet of size: " << lenCipher;
	Logger::log(tmp.str());
#endif

	return true;
}

bool TrafficHandlerMSCSP::decrypt(const unsigned int sessionId, BYTE* cipher, unsigned int& lenCipher)
{
	EnterCriticalSection(&csKeys);
	if (keys.find(sessionId) == keys.end())
	{
		Logger::log(L"Error: Could not find a key for decryption.");
		LeaveCriticalSection(&csKeys);
		return false;
	}
	HCRYPTKEY hKey = keys[sessionId];
	LeaveCriticalSection(&csKeys);

	unsigned int lenOrig = lenCipher;
	if (CryptDecrypt(hKey, NULL, TRUE, 0, cipher, (DWORD*)&lenCipher) == 0)
	{
		//this could be the case if the other side has not yet started to encrypt.
		//TODO: sound an alarm if this happens too often?
#ifdef DEBUG
		std::wstringstream tmp;
		tmp << L"Error: Could not decrypt packet. Error: " << std::hex << GetLastError() << " Details:\n";
		tmp << L"\tLen: " << lenCipher;
		Logger::log(tmp.str());
#else
		Logger::log(L"Error: Decryption of packet failed.");
#endif
		return true;
	}

#ifdef DEBUG
	std::wstringstream tmp;
	tmp << L"Info: Decrypted packet of size: " << lenCipher;
	Logger::log(tmp.str());
#endif
	return true;
}

bool TrafficHandlerMSCSP::setUpEncryptedSession(const Session& session)
{
	const int n128Bits = 16;
	Key key;

	session.getKey(key);

	if (key.length < n128Bits)
	{
		Logger::log(L"Fatal Error: Key is too short.");
		return false;
	}

	EnterCriticalSection(&csKeys);
	if (keys.find(session.getId()) != keys.end())
	{
		Logger::log(L"Fatal Error: Tried to import a key for a session that already got a key.");
		LeaveCriticalSection(&csKeys);
		return false;
	}

	HCRYPTKEY hKey;

	struct {
    PUBLICKEYSTRUC hdr;
	DWORD sizeData;
    BYTE rgbKeyData [n128Bits];
	} keyBlob;

	keyBlob.hdr.bType = PLAINTEXTKEYBLOB;
	keyBlob.hdr.bVersion = CUR_BLOB_VERSION;
	keyBlob.hdr.reserved = 0;
	
	//use AES 128
	keyBlob.hdr.aiKeyAlg = CALG_AES_128;
	keyBlob.sizeData = n128Bits;

	//copy key bytes
	memcpy(&(keyBlob.rgbKeyData), &(key.data), n128Bits);

	if (CryptImportKey(hCSP, (BYTE*)&keyBlob, sizeof(keyBlob), NULL , CRYPT_EXPORTABLE, &hKey) == false)
	{
		Logger::log(L"Fatal Error: Failed to load key into CSP.");
		LeaveCriticalSection(&csKeys);
		return false;
	}
	keys[session.getId()] = hKey;
	LeaveCriticalSection(&csKeys);

#ifdef DEBUG
		DWORD* pKey = (DWORD*)&(key.data);
		std::wstringstream tmp;
		tmp << L"Info: Successfully imported key: " << std::hex << pKey[0] << std::hex << pKey[1] << std::hex << pKey[2] << std::hex << pKey[3];
		Logger::log(tmp.str());
#endif

	return true;
}

bool TrafficHandlerMSCSP::tearDownEncryptedSession(const Session& session)
{
	EnterCriticalSection(&csKeys);
	if (keys.find(session.getId()) == keys.end())
	{
		Logger::log(L"Error: Tried to erase a key for a session that got no key.");
		LeaveCriticalSection(&csKeys);
		return false;
	}
	if (CryptDestroyKey(keys[session.getId()]) == 0)
	{
		Logger::log(L"Error: Something went worng while destroying a key.");
		LeaveCriticalSection(&csKeys);
		return false;
	}
	keys.erase(session.getId());
	LeaveCriticalSection(&csKeys);
	return true;
}

bool TrafficHandlerMSCSP::readyToEncrypt()
{
	return noErrors;
}