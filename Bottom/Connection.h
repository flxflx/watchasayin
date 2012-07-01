#pragma once
#include "StdAfx.h"
#include <ws2def.h>
#include <set>
#include <map>

using namespace std;

//! Defines a single network connection over one specific ip protocol + one specific port.
class Connection
{
private:
	in_addr ip;
	unsigned short srcPort;
	unsigned short dstPort;
	IPPROTO protocol;

public:
	
	/*!
	The public constructor.
		\param ip The ip-address of the remote host.
		\param srcPort The source network-port.
		\param srcPort The destination network-port.
		\param protocol The used protocol (e.g. TCP).
	*/
	Connection(in_addr ip, unsigned short srcPort, unsigned short dstPort, IPPROTO protocol) : ip(ip), protocol(protocol), srcPort(srcPort), dstPort(dstPort)
	{
	}

	//!Gets the source port.
	unsigned short getSrcPort() const
	{
		return srcPort;
	}

	//!Gets the destination port.
	unsigned short getDstPort() const
	{
		return dstPort;
	}

	//! Gets the ip-address of the remote host.
	in_addr getIp() const
	{ 
		return ip;
	}

	//! Gets the used protocol.
	IPPROTO getProtocol() const
	{
		return protocol;
	}

	/*! Compares to connections.
		\param conn The connection to compare to.
		\returns A flag indicating whether both connections are the same or not.
	*/
	bool equals(Connection &conn) const
	{
		if (conn.getIp().S_un.S_addr != ip.S_un.S_addr)
		{
			return false;
		}

		if (conn.getSrcPort() != srcPort)
		{
			return false;
		}

		if (conn.getDstPort() != dstPort)
		{
			return false;
		}

		if (conn.getProtocol() != protocol)
		{
			return false;
		}

		return true;
	}
};

//! Defines a source port with multiple destination ports (at least one)
class SourcePort
{
private:
	set<unsigned short> dstPorts;
	unsigned short port;

public:

	SourcePort()
	{
	}
	
	/*! The public constructor.
		\param port The source port.
		\param dstPort A destination port.
	*/
	SourcePort(unsigned short port, unsigned short dstPort) :port(port)
	{
		addDestinationPort(dstPort);
	}

	/*! Adds a destination port.
		\param dstPort The destination port to be added.
	*/
	void addDestinationPort(unsigned short dstPort)
	{
		dstPorts.insert(dstPort);
	}

	//! Gets a set of all destination ports.
	set<unsigned short>* getDestinationPorts()
	{
		return &dstPorts;
	} 

	//! Gets a set of all destination ports.
	const set<unsigned short>* getDestinationPorts() const
	{
		return &dstPorts;
	} 

	//! Gets the source port.
	unsigned short getPort() const
	{
		return port;
	}

	//! Gets the number of destination ports.
	int getNumberOfConnections() const
	{
		return dstPorts.size();
	}
};


//! Defines a network connection over one specific ip protocol (multiple ports)
class ProtocolConnection
{
private:
	in_addr ip;
	map<unsigned short,SourcePort> sourcePorts;
	IPPROTO protocol;

public:

	ProtocolConnection()
	{
	}

	/*! A public constructor.
		\param connection The connection to start from.
	*/
	ProtocolConnection(const Connection &connection) 
	{
		ip = connection.getIp();
		protocol = connection.getProtocol();
		SourcePort sourcePort(connection.getSrcPort(), connection.getDstPort());
		addSourcePort(sourcePort);
	}
	
	/*! A public constructor.
		\param ip The ip-address of the remote host
		\param protocol The used protocol.
		\param srcPort The first source network-port.
		\param dstPort The first destination network-port.
	*/
	ProtocolConnection(const in_addr ip, const IPPROTO protocol, const unsigned short srcPort, const unsigned short dstPort) : ip(ip), protocol(protocol)
	{
		addSourcePort(srcPort, dstPort);
	}

	/*! A public constructor.
		\param ip The ip-address of the remote host
		\param protocol The used protocol.
		\param srcPort The first source network-port.
	*/
	ProtocolConnection(in_addr ip, IPPROTO protocol, SourcePort &sourcePort) : ip(ip), protocol(protocol)
	{
		addSourcePort(sourcePort);
	}
	
	//! Gets the set of destination network-ports.
	map<unsigned short,SourcePort>* getSourcePorts()
	{
		return &sourcePorts;
	}

	//! Gets a source port -> destination ports mapping.
	const map<unsigned short,SourcePort>* getSourcePorts() const
	{
		return &sourcePorts;
	}
	
	//! Adds a source port with a corresponding destination port (raw).
	void addSourcePort(const unsigned short srcPort, const unsigned short dstPort)
	{
		SourcePort sourcePort(srcPort, dstPort);
		addSourcePort(sourcePort);
	}

	//! Adds a source port with a corresponding destination port.
	void addSourcePort(const SourcePort &sourcePort)
	{
		
		unsigned short srcPort = sourcePort.getPort();
		//check if source port is unknown...
		if (sourcePorts.find(srcPort) == sourcePorts.end())
		{
			//...if so add it
			sourcePorts[srcPort] = sourcePort;
		}
		else
		{
			//... if not, only add the new destination ports
			const set<unsigned short>* dstPorts = sourcePort.getDestinationPorts();
			set<unsigned short>::iterator itDstPorts;

			for (itDstPorts = dstPorts->begin(); itDstPorts != dstPorts->end(); ++itDstPorts)
			{
				sourcePorts[srcPort].addDestinationPort(*itDstPorts);
			}
		}
	}

	//! Gets the ip-address of the rmeote host.
	in_addr getIp()
	{
		return ip;
	}

	//! Gets the protocol.
	IPPROTO getProtocol()
	{
		return protocol;
	}

	//! Gets the number of connections using the protocol.
	int getNumberOfConnections()
	{
		int n = 0;
		map<unsigned short,SourcePort>::iterator itPorts;
		for(itPorts = sourcePorts.begin(); itPorts != sourcePorts.end(); ++itPorts)
		{
			n += itPorts->second.getNumberOfConnections();
		}

		return n;
	}
};

//! Defines a network connection with one specific ip (multiple protocols)
class IPConnection
{
private:
	in_addr ip;
	map<IPPROTO,ProtocolConnection> protocols;

public:
	
	IPConnection()
	{
	}

	/*! A public constructor.
		\param ip The ip-address of the remote host.
	*/
	IPConnection(in_addr ip) : ip(ip)
	{
	}

	/*! A public constructor.
		\param ip The ip-address of the remote host.
		\param protocolConnection A first protocol connection.
	*/
	IPConnection(in_addr ip, ProtocolConnection &protocolConnection) : ip(ip)
	{
		addProtocol(protocolConnection);
	}

	//! Gets a protocol -> connections mapping.
	map<IPPROTO,ProtocolConnection>* getProtocols()
	{
		return &protocols;
	}

	//! Gets a protocol -> connections mapping.
	const map<IPPROTO,ProtocolConnection>* getProtocols() const
	{
		return &protocols;
	}

	//! Adds a set of connections over a given protocol.
	void addProtocol(ProtocolConnection &protocolConnection)
	{
		//check if protocol is unknown...
		IPPROTO protocol = protocolConnection.getProtocol();
		if (protocols.find(protocol) == protocols.end())
		{
			//...if, so just add it
			protocols[protocol] = protocolConnection;
		}
		else
		{
			//...if not, just add ports
			const map<unsigned short, SourcePort>* srcPorts = protocolConnection.getSourcePorts();
			map<unsigned short, SourcePort>::const_iterator itSrcPorts;
			for (itSrcPorts = srcPorts->begin(); itSrcPorts != srcPorts->end(); ++itSrcPorts)
			{
				protocols[protocol].addSourcePort(itSrcPorts->second);
			}	
		}
	}

	//! Gets the remote host's ip-address
	in_addr getIp()
	{
		return ip;
	}

	//! Gets the total number of single connections.
	int getNumberOfConnections()
	{
		int n = 0;
		map<IPPROTO,ProtocolConnection>::iterator itProtocols;
		for(itProtocols = protocols.begin(); itProtocols != protocols.end(); ++itProtocols)
		{
		n += itProtocols->second.getNumberOfConnections();
		}
		return n;
	}
};

//! Possible results of a search for a connection. 
enum TSearchResult
{
	MATCH_FOUND = 0, /*!< An exact match was found.*/
	SIMILAR_FOUND = 1, /*!< A similar connection was found with at least the ip, proto and one port (src or dst) being the same.*/
	NOTHING_FOUND = 2 /*!< No match was found.*/
};

//! Container for ip connections. Tried to optimize speed and storage.
class Connections
{
private:
	std::map<unsigned int,IPConnection> connections;

public:
	Connections()
	{
	}


	/*! Adds a connection.
		\param conn The connection to be added
		\returns A flag indicating whether the given connection was new or not.
	*/
	bool addConnection(const Connection &conn)
	{
		bool newConnection = false;
		unsigned int ip = (unsigned int)(conn.getIp().S_un.S_addr);
	
		//check if ip is unknown
		if (connections.find(ip) == connections.end())
		{
			newConnection = true;

			//if so, add ip 
			IPConnection ipc(conn.getIp());
			connections[ip] = ipc;
		}
	
		ProtocolConnection pc(conn);
		connections[ip].addProtocol(pc);

		return newConnection;
	}

	/*! Checks for a given connection.
		\param conn The connection to be checked.
		\return A flag indicating whether the given connection was found.
	*/
	bool contains(const Connection &conn)
	{
		return (search(conn) == MATCH_FOUND);
	}

	/*! Looks for a given connection.
		\param conn The connection to be checked.
		\return A flag indicating whether a exact match, similar match or no macht at all was found.
	*/
	TSearchResult search(const Connection &conn)
	{
		unsigned int ip = conn.getIp().S_un.S_addr;
		if (connections.find(ip) == connections.end())
		{
			return NOTHING_FOUND;
		}
		
		map<IPPROTO,ProtocolConnection>* protoConns = connections[ip].getProtocols();
		if (protoConns->find(conn.getProtocol()) == protoConns->end())
		{
			return NOTHING_FOUND;
		}

		//look for exact mactch
		map<unsigned short,SourcePort>* srcPorts = (*protoConns)[conn.getProtocol()].getSourcePorts();
		if (srcPorts->find(conn.getSrcPort()) != srcPorts->end())
		{
			set<unsigned short>* dstPorts = (*srcPorts)[conn.getSrcPort()].getDestinationPorts();
			if (dstPorts->find(conn.getDstPort()) != dstPorts->end())
			{
				//both ports are the same -> exact match
				return MATCH_FOUND;
			}

			//only the src port is the same -> similar match
			return SIMILAR_FOUND;
		}

		//look for connections with same ip, protocol and dst port
		map<IPPROTO,ProtocolConnection>::iterator itProtoConns;
		for (itProtoConns = protoConns->begin(); itProtoConns != protoConns->end(); ++itProtoConns)
		{
			//for each connection, check if at least one port is the same
			map<unsigned short,SourcePort>* sourcePorts = itProtoConns->second.getSourcePorts();
			map<unsigned short,SourcePort>::iterator itSourcePorts;
			for (itSourcePorts = sourcePorts->begin(); itSourcePorts != sourcePorts->end(); ++itSourcePorts)
			{
				set<unsigned short>* dstPorts = itSourcePorts->second.getDestinationPorts();
				if (dstPorts->find(conn.getDstPort()) != dstPorts->end())
				{
					//only dst port is the same -> similar match
					return SIMILAR_FOUND;
				}
			}
		}
		
		//no port match -> no match
		return NOTHING_FOUND;
	}

	//! Gets a ip-address -> connections mapping.
	void getConnections(std::map<unsigned int,IPConnection>& connections)
	{
		connections = this->connections;
	}

	//! Clears the container.
	void clear()
	{
		connections.clear();
	}
};