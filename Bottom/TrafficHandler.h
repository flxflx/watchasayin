#pragma once
#include "Connection.h"
#include "Session.h"
#include <map>

using namespace std;


//! The different security policies of a Bottom module.
enum TSecurityPolicy
{
    VERY_LOOSE = 1, /*!< No connections are blocked at any time.*/

	
    LOOSE = 2, /*!< All new outgoing connections are blocked as long as a secure session is active.
			   TODO: Unsupported at the current state of development */

    TIGHT = 3, /*!< All unencrypted outgoing connections are blocked as long as a secure session is active. */ 

    unknown = -1 /*!< An unknown security policy.*/
};

//! Abstract class for handling network-io and sessions.
class TrafficHandler
{
private:
	CRITICAL_SECTION csOutgoing;
	CRITICAL_SECTION csIncoming;
	CRITICAL_SECTION csGeneric;

	Connections openConnectionsReceive;
	Connections openConnectionsSend;
	std::map<unsigned int, Session> encSessions;
	TSecurityPolicy securityPolicy;

	bool countConnections;
	std::map<unsigned int, unsigned int> connectionCounts;

	void updateConnectionCounts(const unsigned int ip);

protected:

	//cipher must be freed after using
	virtual bool encrypt(const unsigned int sessionId, const BYTE* plain, const unsigned int lenPlain, BYTE** cipher, unsigned int& lenCipher, const unsigned int maximumPacketSize) = 0; 
	//plain must be freed after using
	virtual bool decrypt(const unsigned int sessionId, BYTE* cipher, unsigned int& lenCipher) = 0;
	
	virtual bool setUpEncryptedSession(const Session& session) = 0;
	virtual bool tearDownEncryptedSession(const Session& session) = 0;

public:

	TrafficHandler(const TSecurityPolicy securityPolicy);
	~TrafficHandler();

	/*! Evaluates an outgoing network packet and encrypts it if needed. This should be called for every outgoing network packet. 
		\param conn The connection associated with the packet.
		\param payload The payload of the to be sent packet (never actually send this, always only newPayload).
		\param lenPayload The length in bytes of the payload.
		\param newPayload Receives the payload that has to be sent. The caller is responsible for freeing.
		\param lenNewPayload Receives the length in bytes of the new payload.
		\param maximumPacketSize The maximum size newPayload could possibly have.
		\returns A flag indicating success. On "false" nothing should be sent!
	*/
	bool packetToSend(const Connection &conn, const char* payload, const unsigned int lenPayload, char** newPayload, unsigned int& lenNewPayload, const unsigned int maximumPacketSize);

	/*! Evaluates an incoming network packet and decrypts it if needed. This should be called for every incoming network packet.
		\param conn The connection associated with the packet.
		\param payload The received payload. Gets decrypted in place.
		\param lenPayload The length of the received payload. Gets updated in case any data is decrypted.
		\returns A flag indicating success.
	*/
	bool packetReceived(const Connection &conn, char* payload, unsigned int& lenPayload);
	
	//! Gets a complete ip-address -> outgoing connections mapping.
	void getOutgoingConnections(map<unsigned int,IPConnection>& connections);
	//! Gets a complete ip-address -> incoming connections mapping.
	void getIncomingConnections(map<unsigned int,IPConnection>& connections);

	//! Clears the list of outgoing connections.
	void resetOugoingConnections();
	//! Clears the list of incoming connections.
	void resetIncomingConnections();

	//! Adds a to be encrypted session.
	void addEncryptedSession(const Session& session);
	//! Removes a session.
	void removeEncryptedSession(const Session& session);

	/*! Checks if a given incoming connection belongs to an active session.
		\param conn The connection to check.
		\param sessionId If found, Receives the id of the session the given connection belongs to.
		\returns A flag indicating whether the given connection belongs to any session at all.
	*/
	bool ownedBySessionIncoming(const Connection& conn, unsigned int& sessionId);

	/*! Checks if a given outgoing connection belongs to an active session.
		\param conn The connection to check.
		\param sessionId If found, Receives the id of the session the given connection belongs to.
		\returns A flag indicating whether the given connection belongs to any session at all.
	*/
	bool ownedBySessionOutgoing(const Connection& conn, unsigned int& sessionId);

	//! Gets the applied security policy.
	TSecurityPolicy getSecurityPolicy() const;

	//! Sets the security policy
	bool setSecurityPolicy(const TSecurityPolicy securityPolicy);

	//! Enables or disables to counting of connections
	void setCountConnections(const bool enable); 

	//! Gets an ip<->[packets send to/from] map
	void getConnectionCounts(map<unsigned int,unsigned int>& connectionCounts);

	//! Checks if the traffic handler is ready to encrypt sessions.
	virtual bool readyToEncrypt() = 0;
};

//! Implementation of the abstract TrafficHandler class using the Windows Cryptography API (PROV_RSA_AES)
class TrafficHandlerMSCSP : public TrafficHandler
{
private:
	//mapping of session-id and csp
	std::map<unsigned int, HCRYPTKEY> keys;
	HCRYPTPROV hCSP;
	CRITICAL_SECTION csKeys;

	bool noErrors;

protected:
	bool encrypt(const unsigned int sessionId, const BYTE* plain, const unsigned int lenPlain, BYTE** cipher, unsigned int& lenCipher, const unsigned int maximumPacketSize); 
	bool decrypt(const unsigned int sessionId, BYTE* cipher, unsigned int& lenCipher);

	bool setUpEncryptedSession(const Session& session);
	bool tearDownEncryptedSession(const Session& session);

public:
	TrafficHandlerMSCSP(const TSecurityPolicy securityPolicy);
	~TrafficHandlerMSCSP();

	bool readyToEncrypt();
};

