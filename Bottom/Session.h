#pragma once
#include "Connection.h"
#include <vector>

//! Describes the key of a session.
class Key
{

public:
	BYTE data[32]; //max. 256 bits
	int length;

	Key()
	{
		length = -1;
	}

	/*! The public constructor.
		\param data The raw-data of the key.
		\param length The length of the raw-data in bytes.
	*/
	Key(const BYTE* data, const unsigned int length)
	{
		if (length > sizeof(this->data))
		{
			return;
		}
		
		this->length = length;
		memcpy(this->data, data, length);
	}
};

//! Describes a session consisting of multiple connections to be encrypted.
class Session
{
private:
	unsigned int id;
	Connections incomingConnections;
	Connections outgoingConnections;
	Key key;
	bool keyValid;
	bool expandDynamically;

public:
	Session()
	{
		Session(-1, false);
	}

	/*! The public constructor.
		\param id The id of the session.
		\param expandDynamically A flag indicating whether new connections which are "similar" to connections already belonging to 
		the session should be added to it on-the-fly when it is active.
	*/
	Session(const unsigned int id, const bool expandDynamically) : id(id), expandDynamically(expandDynamically)
	{
		keyValid = false;
	}

	//! Gets the id of the session.
	unsigned int getId() const 
	{
		return id;
	}

	//! Checks if the session is expandable.
	bool expand() const
	{
		return expandDynamically;
	}

	//! Adds an to be encrypted outgoing connection to the session.
	void addOutgoingConnection(const Connection& conn)
	{
		outgoingConnections.addConnection(conn);
	}

	//! Adds an to be encrypted incoming connection to the session.
	void addIncomingConnection(const Connection& conn)
	{
		incomingConnections.addConnection(conn);
	}

	//! Sets the cryptographic key of the session.
	void setKey(const Key& key)
	{
		keyValid = true;
		this->key = Key(key.data, key.length);
	}

	//! Gets the cryptographic key of the session.
	void getKey(Key& key) const
	{
		if (keyValid != true)
		{
			return;
		}
		key = Key(this->key.data, this->key.length);
	}

	//! Checks if a given incomging connection already belongs to the session.
	bool containsIncoming(const Connection& conn)
	{
		return incomingConnections.contains(conn);
	}

	//! Checks if a given outgoing connection already belongs to the session.
	bool containsOutgoing(const Connection& conn)
	{
		return outgoingConnections.contains(conn);
	}

	//! Searches for an incoming connection (finds similar connections).
	TSearchResult searchIncoming(const Connection& conn)
	{
		return incomingConnections.search(conn);
	}

	//! Searches for an outgoing connection (finds similar connections).
	TSearchResult searchOutgoing(const Connection& conn)
	{
		return outgoingConnections.search(conn);
	}

	//! Checks if the session already has a cryptographic key.
	bool hasKey() const
	{
		return keyValid;
	}
};

//! Factory/container class for Session objects.
class Sessions
{
private:
	unsigned int index;
	std::map<unsigned int,Session*> sessions;

public:
	Sessions()
	{
		index = 0;
	}

	~Sessions()
	{
		//clean-up
		std::map<unsigned int,Session*>::iterator iter;
		for (iter = sessions.begin(); iter != sessions.end(); iter++)
		{
			delete iter->second;
		}
	}

	//! Checks if the container has a session of the given id. 
	bool hasSession(const unsigned int sessionId)
	{
		return (sessions.find(sessionId) != sessions.end());
	}

	/*! Creates a new session and adds it to the container.
		\param expandableSession \sa Session
		\returns The id of the newly created session.
	*/
	unsigned int newSession(const bool expandableSession)
	{
		index++;
		sessions[index] = new Session(index, expandableSession);
		return index;
	}

	//! Removes the given session from the container.
	void deleteSession(unsigned int sessionId)
	{
		if (hasSession(sessionId) != true)
		{
			return;
		}

		delete sessions[sessionId];
		sessions.erase(sessionId);
	}

	//! Adds an incoming connection to the session with the given id.
	void addIncomingConnection(const unsigned int sessionId, const Connection& conn)
	{
		if (hasSession(sessionId) != true)
		{
			return;
		}

		sessions[sessionId]->addIncomingConnection(conn);
	}

	//! Adds an outgoing connection to the session with the given id.
	void addOutgoingConnection(const unsigned int sessionId, const Connection& conn)
	{
		if (hasSession(sessionId) != true)
		{
			return;
		}

		sessions[sessionId]->addOutgoingConnection(conn);
	}

	//! Sets the cryptographic key of the given session.
	void setKey(const unsigned int sessionId, const Key& key)
	{
		if (hasSession(sessionId) != true)
		{
			return;
		}

		//check if we already have a valid key
		if (sessions[sessionId]->hasKey() == true)
		{
			return;
		}

		sessions[sessionId]->setKey(key);
	}

	/*! Gets the session with the given id.
		\param sessionId The id of the desired session.
		\param session On success receives the desired session.
		\returns A flag indicating success.
	*/
	bool getSession(const unsigned int sessionId, Session& session)
	{
		if (hasSession(sessionId) != true)
		{
			return false;
		}

		session = *(sessions[sessionId]);
		return true;
	}

	/*! Gets the cryptographic key of the session wuth the given id.
		\param sessionId The id of the session to get the key from.
		\param key On success receives the desired key.
		\returns A flag indicating success.
	*/
	bool getKey(const unsigned int sessionId, Key& key)
	{
		if (hasSession(sessionId) != true)
		{
			return false;
		}

		if (sessions[sessionId]->hasKey() != true)
		{
			return false;
		}
		Key tempKey;
		sessions[sessionId]->getKey(tempKey);
		key = tempKey;

		return true;
	}

	/*! Checks if the given incoming connection belongs to one of the sessions in the container.
		\param conn The connection to be checked.
		\param sessionId Receives the id of the session the connection belongs to (only if the return value is true).
		\returns A flag indicating whether the given connection is part of a session.
	*/
	bool ownedBySessionIncoming(const Connection& conn, unsigned int& sessionId)
	{
		bool owned = false;
		std::map<unsigned int, Session*>::iterator iter;
		for (iter = sessions.begin(); iter != sessions.end(); iter++)
		{
			if (iter->second->containsIncoming(conn) == true)
			{
				owned = true;
				sessionId = iter->second->getId();
				break;
			}
		}
		return owned;
	}

	/*! Checks if the given outgoing connection belongs to one of the sessions in the container.
		\sa ownedBySessionIncoming
	*/
	bool ownedBySessionOutgoing(const Connection& conn, unsigned int& sessionId)
	{
		bool owned = false;
		std::map<unsigned int, Session*>::iterator iter;
		for (iter = sessions.begin(); iter != sessions.end(); iter++)
		{
			if (iter->second->containsOutgoing(conn) == true)
			{
				owned = true;
				sessionId = iter->second->getId();
				break;
			}
		}
		return owned;
	}
};