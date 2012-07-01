// HookingEngine.cpp : Implementation of CHookingEngine
#include "stdafx.h"
#include "HookingEngine.h"

// CHookingEngine
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <Mswsock.h>
#include <WinNT.h>
#include "Logger.h"
#include "TrafficHandler.h"
#include "Session.h"
#include <sstream>
#include ".\NCodeHook\NCodeHook.cpp"


#pragma intrinsic(_ReturnAddress)

#define LENGTH_CONNECTION_DESCRIPTION 4 //ip, fsrcPort, dstPort, protocol
#define STOP_CLOCK_DELAY 10000

static HMODULE s_hDll;

TrafficHandler* trafficHandler;
Sessions* sessions;
NCodeHook<ArchitectureIA32>* nch;

//counter for active sessions
volatile unsigned int activeSessions = 0;
volatile unsigned int ticks = 0;
volatile bool clockStopped = false;

//hooked api pointers
int (PASCAL* pSendto) (
  __in  SOCKET s,
  __in  const char *buf,
  __in  int len,
  __in  int flags,
  __in  const struct sockaddr *to,
  __in  int tolen
);

int (PASCAL* pRecvfrom) (
  __in         SOCKET s,
  __out        char *buf,
  __in         int len,
  __in         int flags,
  __out        struct sockaddr *from,
  __inout_opt  int *fromlen
);

int (PASCAL* pSend)(
  __in  SOCKET s,
  __in  const char *buf,
  __in  int len,
  __in  int flags
);

int (PASCAL* pRecv)(
  __in   SOCKET s,
  __out  char *buf,
  __in   int len,
  __in   int flags
);

DWORD (WINAPI* pGetTickCount)(void);

int (WSAAPI* pWSAConnect)(
  __in   SOCKET s,
  __in   const struct sockaddr *name,
  __in   int namelen,
  __in   LPWSABUF lpCallerData,
  __out  LPWSABUF lpCalleeData,
  __in   LPQOS lpSQOS,
  __in   LPQOS lpGQOS
);

int (WSAAPI* pWSASend)(
  __in   SOCKET s,
  __in   LPWSABUF lpBuffers,
  __in   DWORD dwBufferCount,
  __out  LPDWORD lpNumberOfBytesSent,
  __in   DWORD dwFlags,
  __in   LPWSAOVERLAPPED lpOverlapped,
  __in   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int (WSAAPI* pWSASendTo)(
  __in   SOCKET s,
  __in   LPWSABUF lpBuffers,
  __in   DWORD dwBufferCount,
  __out  LPDWORD lpNumberOfBytesSent,
  __in   DWORD dwFlags,
  __in   const struct sockaddr *lpTo,
  __in   int iToLen,
  __in   LPWSAOVERLAPPED lpOverlapped,
  __in   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int (WSAAPI* pWSARecv)(
  __in     SOCKET s,
  __inout  LPWSABUF lpBuffers,
  __in     DWORD dwBufferCount,
  __out    LPDWORD lpNumberOfBytesRecvd,
  __inout  LPDWORD lpFlags,
  __in     LPWSAOVERLAPPED lpOverlapped,
  __in     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int (WSAAPI* pWSARecvFrom)(
  __in     SOCKET s,
  __inout  LPWSABUF lpBuffers,
  __in     DWORD dwBufferCount,
  __out    LPDWORD lpNumberOfBytesRecvd,
  __inout  LPDWORD lpFlags,
  __out    struct sockaddr *lpFrom,
  __inout  LPINT lpFromlen,
  __in     LPWSAOVERLAPPED lpOverlapped,
  __in     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

//to be blocked apis
int (WSAAPI* pWSASendMsg)(
  __in   SOCKET s,
  __in   LPWSAMSG lpMsg,
  __in   DWORD dwFlags,
  __out  LPDWORD lpNumberOfBytesSent,
  __in   LPWSAOVERLAPPED lpOverlapped,
  __in   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int (WSAAPI* pWSASendDisconnect)(
	__in SOCKET s,
	__in_opt LPWSABUF lpOutboundDisconnectData
);

LPFN_TRANSMITFILE pTransmitFile = TransmitFile;

//apis we need to get the pointers to dynamically 
LPFN_CONNECTEX pConnectEx;
LPFN_TRANSMITPACKETS pTransmitPackets;


//////////////////////////////////////////////////////////
//helper functions

void helpStopClock(unsigned int delay)
{
	Sleep(delay);
	if (activeSessions > 0)
	{
		ticks = pGetTickCount();
		clockStopped = true;
#ifdef DEBUG
		Logger::log(L"Info: Stopped GetTickCount() clock.");
#endif 
	}
}

//! makes GetTickCount() return always the same amount of ticks
//TODO: hacked together code, recode!
void stopClock(unsigned int delay)
{
	if (clockStopped == false)
	{
		if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)helpStopClock, (LPVOID)delay, 0, NULL) == NULL)
		{
			Logger::log(L"Error: Failed to create thread for stopping clock.");
		}
	}
}

//! makes GetTickCount() operate normally
void resumeClock()
{
#ifdef DEBUG
		Logger::log("Info: Resumed GetTickCount() clock.");
#endif 
	clockStopped = false;
}


//! gets pointers to target apis
bool getTargetAPIPointers()
{
	//static
	pSend = send;
	pSendto = sendto;
	pRecv = recv;
	pRecvfrom = recvfrom;
	pGetTickCount = GetTickCount;
	pWSAConnect = WSAConnect;
	pWSASendTo = WSASendTo;
	pWSASend = WSASend;
	pWSARecv = WSARecv;
	pWSARecvFrom = WSARecvFrom;
	pWSASendMsg = WSASendMsg;
	pWSASendDisconnect = WSASendDisconnect;
	pTransmitFile = TransmitFile;

	//dynamic
	//need to create a socket first
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
	{
		return false;
	}

	SOCKET sock;
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}

	GUID guidConnectEx = WSAID_CONNECTEX;
	DWORD bytesWritten;
	if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx),&pConnectEx, sizeof(pConnectEx), &bytesWritten, NULL, NULL) != 0)
	{
		closesocket(sock);
		WSACleanup();
		return false;
	}
	GUID guidTransmitPackets = WSAID_TRANSMITPACKETS;
	if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidTransmitPackets, sizeof(guidTransmitPackets),	&pTransmitPackets, sizeof(pTransmitPackets), &bytesWritten, NULL, NULL) != 0)
	{
		closesocket(sock);
		WSACleanup();
		return false;
	}

	closesocket(sock);
	WSACleanup();
	return true;
}

//for sendto/recvfrom, returns true on success
bool getPacketInfo(__in SOCKET s, __in const struct sockaddr_in* to, __out in_addr &ip, __out IPPROTO &proto, __out unsigned short &ownPort, __out unsigned short& peerPort, __out int& maximumPacketSize)
{
	//get maximum message size
	int sizeRet = sizeof(maximumPacketSize);
	if (getsockopt(s, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&maximumPacketSize, &sizeRet) != 0)
	{
		Logger::log(L"Error: Failed to get maximum packet size for given socket.");
		return false;
	}

	//get own port
	sockaddr_in name;
	int sizeName = sizeof(sockaddr);
	if (getsockname(s, (sockaddr*)&name, &sizeName) != 0)
	{
		Logger::log(L"Error: getsockname() failed.");
		//TODO: what should be done in such a case? Drop packed?
		return false;
	}
	ownPort = ntohs(name.sin_port);

	//get protocol, peer's ip and port 
	if (to != NULL) //unconnected socket
	{
		ip = to->sin_addr;
		peerPort = ntohs(to->sin_port);
		proto = IPPROTO_UDP;
	}
	else //connected socket
	{
		sockaddr_in name;
		int sizeName = sizeof(sockaddr);
		if (getpeername(s, (sockaddr*)&name, &sizeName) != 0)
		{
			Logger::log(L"Error: getpeername() failed.");
			//TODO: what should be done in such a case? Drop packed?
			return false;
		}

		ip = name.sin_addr;
		peerPort = ntohs(name.sin_port);
		proto = IPPROTO_TCP;
	}
	
	return true;
}

//////////////////////////////////////////////////////////
//hooked functions
int PASCAL mySendto(
  __in  SOCKET s,
  __in  const char* buf,
  __in  unsigned int len,
  __in  int flags,
  __in  const struct sockaddr_in* to,
  __in  int tolen
)
{
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;
	
	//note: since we're sending a packet: ownPort = srcPort and peerPort = dstPort
	if (getPacketInfo(s, to, ip, proto, srcPort, dstPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in mySendto(), dropping packet.");
		return len;
	}

	char* newBuf;
	unsigned int newLen;

	Connection conn(ip, srcPort, dstPort, proto);
	if (trafficHandler->packetToSend(conn, buf, len, &newBuf, newLen, maxPktSize) == false)
	{
#ifdef DEBUG
		std::wstringstream tmp;
		tmp << "Info: Dropped packet in sendto():\n";
		tmp << "\tip: " << ip.S_un.S_un_b.s_b1 << "." << ip.S_un.S_un_b.s_b2 << "." << ip.S_un.S_un_b.s_b3 << "." << ip.S_un.S_un_b.s_b4;
		tmp << "\n\tsrc: " << srcPort;
		tmp << "\n\tdst: " << dstPort;
		tmp << "\n\tproto: " << proto; 
		Logger::log(tmp.str());
#endif
		return len;
	}

	//send packet using the real sendto()
	int retVal = pSendto(s, (const char*)newBuf, newLen, flags, (sockaddr*)to, tolen);
	free(newBuf);

	return retVal;	
}

int PASCAL myRecvfrom(
  __in         SOCKET s,
  __out        char *buf,
  __in         int len,
  __in         int flags,
  __out        struct sockaddr *from,
  __inout_opt  int *fromlen
)
{
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;

	int retVal = pRecvfrom(s, buf, len, flags, from, fromlen);
	
	if (retVal == SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}
	
	//note: since we're receiving a packet: ownPort = dstPort and peerPort = srcPort
	if (getPacketInfo(s, (const sockaddr_in*)from, ip, proto, dstPort, srcPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in myRecvFrom(), dropping packet.");

		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}
	Connection conn(ip, srcPort, dstPort, proto);

	unsigned int newLen = retVal;
	if(trafficHandler->packetReceived(conn, buf, newLen) == false)
	{
		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}
	return newLen;
}

int PASCAL mySend(
  __in  SOCKET s,
  __in  const char *buf,
  __in  int len,
  __in  int flags
)
{
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;
	
	//note: since we're sending a packet: ownPort = srcPort and peerPort = dstPort
	if (getPacketInfo(s, NULL, ip, proto, srcPort, dstPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in mySend(), dropping packet.");
		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}

	char* newBuf;
	unsigned int newLen;
	Connection conn(ip, srcPort, dstPort, proto);
	if (trafficHandler->packetToSend(conn, buf, len, &newBuf, newLen, maxPktSize) == false)
	{
#ifdef DEBUG
		std::wstringstream tmp;
		tmp << "Info: Dropped packet in send():\n";
		tmp << "\tip: " << ip.S_un.S_un_b.s_b1 << "." << ip.S_un.S_un_b.s_b2 << "." << ip.S_un.S_un_b.s_b3 << "." << ip.S_un.S_un_b.s_b4;
		tmp << "\n\tsrc: " << srcPort;
		tmp << "\n\tdst: " << dstPort;
		tmp << "\n\tproto: " << proto; 
		Logger::log(tmp.str());
#endif
		return len;
	}

	//send packet using the real sendto()
	int retVal = pSend(s, newBuf, newLen, flags);
	free(newBuf);

	return retVal;
}

int PASCAL myRecv(
  __in   SOCKET s,
  __out  char *buf,
  __in   int len,
  __in   int flags
) 
{
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;

	int retVal = pRecv(s, buf, len, flags);
	if (retVal == SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}

	//note: since we're receiving a packet: ownPort = dstPort and peerPort = srcPort
	if (getPacketInfo(s, NULL, ip, proto, dstPort, srcPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in myRecv(), dropping packet.");
		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}
	Connection conn(ip, srcPort, dstPort, proto);

	unsigned int newLen = retVal;
	if(trafficHandler->packetReceived(conn, buf, newLen) == false)
	{
		Logger::log(L"Error: Failed to process received packet, droppping it.");
		WSASetLastError(WSAETIMEDOUT);

		return SOCKET_ERROR;
	}
	return newLen;
}

//WSA helper functions
bool processOutgoingWSABuffers(const Connection &conn,  const unsigned int maxPacketSize, const LPWSABUF buffers, const int countBuffers, LPWSABUF &encBuffers)
{
	//check count of buffers
	if (countBuffers < 0)
	{
		return false;
	}

	LPWSABUF tempBuffers = (LPWSABUF)malloc(sizeof(WSABUF)*countBuffers);
	if (tempBuffers == NULL)
	{
		return false;
	}

	//process all buffers 
	for (int i=0; i<countBuffers; i++)
	{
		if (trafficHandler->packetToSend(conn, buffers[i].buf, buffers[i].len, &(encBuffers[i].buf), (unsigned int&)(encBuffers[i].len), maxPacketSize) == false)
		{
			//if anything goes wrong, free all newly allocated buffers so far and exit
			for (int j=(i-1); j>=0; j--)
			{
				free(encBuffers[j].buf);
			}
			return false;
		}
	}
	return true;
}

bool processIncomingWSABuffers(const Connection &conn, LPWSABUF buffers, const int countBuffers)
{
	//check count of buffers
	if (countBuffers < 0)
	{
		return false;
	}

	//process all buffers 
	for (int i=0; i<countBuffers; i++)
	{
		if (trafficHandler->packetReceived(conn, buffers[i].buf, (unsigned int&)(buffers[i].len)) == false)
		{
			return false;
		}
	}
	return true;
}

bool freeWSABuffers(LPWSABUF buffers, const int countBuffers)
{
	//check count of buffers
	if (((int)countBuffers) < 0)
	{
		return false;
	}

	for (int i=0; i<countBuffers; i++)
	{
		free(buffers[i].buf);
	}
	free(buffers);
	return true;
}

int myWSASendTo(
  __in   SOCKET s,
  __in   LPWSABUF lpBuffers,
  __in   DWORD dwBufferCount,
  __out  LPDWORD lpNumberOfBytesSent,
  __in   DWORD dwFlags,
  __in   const struct sockaddr *lpTo,
  __in   int iToLen,
  __in   LPWSAOVERLAPPED lpOverlapped,
  __in   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
#ifdef DEBUG
	Logger::log(L"Info: Called WSASendTo().");
#endif
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;
	
	//note: since we're sending a packet: ownPort = srcPort and peerPort = dstPort
	if (getPacketInfo(s, (const struct sockaddr_in*)lpTo, ip, proto, srcPort, dstPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in mySendto(), dropping packet.");

		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}

	Connection conn(ip, srcPort, dstPort, proto);
	LPWSABUF lpEncBuffers;
	if (processOutgoingWSABuffers(conn, maxPktSize, lpBuffers, dwBufferCount, lpEncBuffers) != true)
	{
		#ifdef DEBUG
		std::wstringstream tmp;
		tmp << "Info: Dropped packets in WSASendTo():\n";
		tmp << "\tip: " << ip.S_un.S_un_b.s_b1 << "." << ip.S_un.S_un_b.s_b2 << "." << ip.S_un.S_un_b.s_b3 << "." << ip.S_un.S_un_b.s_b4;
		tmp << "\n\tsrc: " << srcPort;
		tmp << "\n\tdst: " << dstPort;
		tmp << "\n\tproto: " << proto; 
		Logger::log(tmp.str());
		#endif

		*lpNumberOfBytesSent = 0;
		for (unsigned int i=0; i<dwBufferCount; i++)
		{
			*lpNumberOfBytesSent += lpBuffers[i].len;
		}
		return 0;
	}
	//send packet using the real WSASendTo()
	int retVal = pWSASendTo(s, lpEncBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iToLen, lpOverlapped, lpCompletionRoutine);
	//clean-up
	freeWSABuffers(lpEncBuffers, dwBufferCount);
	return retVal;	
}

int myWSASend(
  __in   SOCKET s,
  __in   LPWSABUF lpBuffers,
  __in   DWORD dwBufferCount,
  __out  LPDWORD lpNumberOfBytesSent,
  __in   DWORD dwFlags,
  __in   LPWSAOVERLAPPED lpOverlapped,
  __in   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
#ifdef DEBUG
	Logger::log(L"Info: Called WSASend().");
#endif
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;
	
	//note: since we're sending a packet: ownPort = srcPort and peerPort = dstPort
	if (getPacketInfo(s, NULL, ip, proto, srcPort, dstPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in mySend(), dropping packet.");
		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}

	Connection conn(ip, srcPort, dstPort, proto);
	LPWSABUF lpEncBuffers;
	if (processOutgoingWSABuffers(conn, maxPktSize, lpBuffers, dwBufferCount, lpEncBuffers) != true)
	{
		#ifdef DEBUG
		std::wstringstream tmp;
		tmp << "Info: Dropped packets in WSASend():\n";
		tmp << "\tip: " << ip.S_un.S_un_b.s_b1 << "." << ip.S_un.S_un_b.s_b2 << "." << ip.S_un.S_un_b.s_b3 << "." << ip.S_un.S_un_b.s_b4;
		tmp << "\n\tsrc: " << srcPort;
		tmp << "\n\tdst: " << dstPort;
		tmp << "\n\tproto: " << proto; 
		Logger::log(tmp.str());
		#endif

		*lpNumberOfBytesSent = 0;
		for (unsigned int i=0; i<dwBufferCount; i++)
		{
			*lpNumberOfBytesSent += lpBuffers[i].len;
		}
		return 0;
	}

	//send packet using the real sendto()
	int retVal = pWSASend(s, lpEncBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
	freeWSABuffers(lpEncBuffers, dwBufferCount);

	return retVal;
}

int myWSARecvFrom(
  __in     SOCKET s,
  __inout  LPWSABUF lpBuffers,
  __in     DWORD dwBufferCount,
  __out    LPDWORD lpNumberOfBytesRecvd,
  __inout  LPDWORD lpFlags,
  __out    struct sockaddr *lpFrom,
  __inout  LPINT lpFromlen,
  __in     LPWSAOVERLAPPED lpOverlapped,
  __in     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;

	int retVal = pWSARecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);
	
	if (retVal != 0)
	{
		return retVal;
	}
	
	//note: since we're receiving a packet: ownPort = dstPort and peerPort = srcPort
	if (getPacketInfo(s, (const sockaddr_in*)lpFrom, ip, proto, dstPort, srcPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in myWSARecvFrom(), dropping it.");

		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}

	Connection conn(ip, srcPort, dstPort, proto);
	if (processIncomingWSABuffers(conn, lpBuffers, dwBufferCount) != true)
	{
		Logger::log(L"Error: Failed to process packets in myWSARecvFrom(), dropping it.");

		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}

	return 0;
}

int myWSARecv(
  __in     SOCKET s,
  __inout  LPWSABUF lpBuffers,
  __in     DWORD dwBufferCount,
  __out    LPDWORD lpNumberOfBytesRecvd,
  __inout  LPDWORD lpFlags,
  __in     LPWSAOVERLAPPED lpOverlapped,
  __in     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	//process packet
	in_addr ip;
	IPPROTO proto;
	unsigned short srcPort = 0, dstPort = 0;
	int maxPktSize;

	int retVal = pWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
	if (retVal != 0)
	{
		return retVal;
	}

	//note: since we're receiving a packet: ownPort = dstPort and peerPort = srcPort
	if (getPacketInfo(s, NULL, ip, proto, dstPort, srcPort, maxPktSize) == false)
	{
		Logger::log(L"Error: Failed to get packet information in myWSARecv(), dropping packet.");

		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}
	Connection conn(ip, srcPort, dstPort, proto);

	unsigned int newLen = retVal;
	if(processIncomingWSABuffers(conn, lpBuffers, dwBufferCount) != true)
	{
		Logger::log(L"Error: Failed to process packets in myWSARecv(), dropping it.");

		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}
	return 0;
}

DWORD WINAPI myGetTickCount(void)
{
	if (clockStopped == true)
	{
		return ticks;
	}
	return pGetTickCount();
}

//////////////////////////////////////////

int myWSAConnect(
  __in   SOCKET s,
  __in   const struct sockaddr *name,
  __in   int namelen,
  __in   LPWSABUF lpCallerData,
  __out  LPWSABUF lpCalleeData,
  __in   LPQOS lpSQOS,
  __in   LPQOS lpGQOS
)
{
	Logger::log(L"Info: Blocked call to WSAConnect().");
	return TRUE;
}

int myWSASendDisconnect(
  __in  SOCKET s,
  __in  LPWSABUF lpOutboundDisconnectData
)
{
	Logger::log(L"Info: Blocked call to WSASendDisconnect().");
	return 0;
}

int myWSASendMsg(
  __in   SOCKET s,
  __in   LPWSAMSG lpMsg,
  __in   DWORD dwFlags,
  __out  LPDWORD lpNumberOfBytesSent,
  __in   LPWSAOVERLAPPED lpOverlapped,
  __in   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	*lpNumberOfBytesSent = lpMsg->lpBuffers->len;
	Logger::log(L"Info: Blocked call to WSASendMsg().");
	return 0;
}


BOOL PASCAL myTransmitPackets(
  SOCKET hSocket,
  LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
  DWORD nElementCount,
  DWORD nSendSize,
  LPOVERLAPPED lpOverlapped,
  DWORD dwFlags
)
{
	Logger::log(L"Info: Blocked call to TransmitPackets().");
	return TRUE;
}

BOOL myTransmitFile(
  SOCKET hSocket,
  HANDLE hFile,
  DWORD nNumberOfBytesToWrite,
  DWORD nNumberOfBytesPerSend,
  LPOVERLAPPED lpOverlapped,
  LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
  DWORD dwFlags
)
{
	Logger::log(L"Info: Blocked call to TransmitFile().");
	return TRUE;
}

BOOL PASCAL myConnectEx(
  __in      SOCKET s,
  __in      const struct sockaddr *name,
  __in      int namelen,
  __in_opt  PVOID lpSendBuffer,
  __in      DWORD dwSendDataLength,
  __out     LPDWORD lpdwBytesSent,
  __in      LPOVERLAPPED lpOverlapped
)
{
	//do not allow ConnectEx if we an active session
	if (activeSessions > 0)
	{
		return TRUE;
	}
	return pConnectEx(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);
}
///////////////////////////////////////////////////////////

struct HOOK 
{
	PVOID api;
	PVOID detour;
	PCHAR apiName;
};

//table of to be hooked apis
#define TO_BE_HOOKED_APIS 15
HOOK apiHooks[TO_BE_HOOKED_APIS] =
{
	{(PVOID) pSendto, (PVOID) mySendto, "sendto"},
	{(PVOID) pSend, (PVOID) mySend, "send"},
	{(PVOID) pRecvfrom, (PVOID) myRecvfrom, "recvfrom"},
	{(PVOID) pRecv, (PVOID) myRecv, "recv"},
	{(PVOID) pWSAConnect, (PVOID) myWSAConnect, "WSAConnect"},
	{(PVOID) pWSASendTo, (PVOID) myWSASendTo, "WSASendTo"},
	{(PVOID) pWSASend, (PVOID) myWSASend, "WSASend"},
	{(PVOID) pWSARecvFrom, (PVOID) myWSARecvFrom, "WSARecvFrom"},
	{(PVOID) pWSARecv, (PVOID) myWSARecv, "WSARecv"},
	{(PVOID) pGetTickCount, (PVOID) myGetTickCount, "GetTickCount"},
	{(PVOID) pWSASendDisconnect, (PVOID) myWSASendDisconnect, "WSASendDisconnect"},
	{(PVOID) pWSASendMsg, (PVOID) myWSASendMsg, "WSASendMsg"},
	{(PVOID) pTransmitFile, (PVOID) myTransmitFile, "TransmitFile"},
	{(PVOID) pTransmitPackets, (PVOID) myTransmitPackets, "TransmitPackets"},
	{(PVOID) pConnectEx, (PVOID) myConnectEx, "ConnectEx"}
};

STDMETHODIMP CHookingEngine::hookNetworkIO(BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);

	*success = false;
	//create traffic handler
	trafficHandler = new TrafficHandlerMSCSP(TIGHT);

	//create sessions container
	sessions = new Sessions();

	//create hooker instance
	nch = new NCodeHook<ArchitectureIA32>();

	//get non-static api pointers
	if (getTargetAPIPointers() == false)
	{
		Logger::log(L"Error: Failed to get to be hooked api pointers dynamically.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_OK;
	}

	//install api-hooks
	for (int i=0; i<TO_BE_HOOKED_APIS; i++)
	{
		std::wstringstream tmp;
		if (nch->createHook<PVOID>(apiHooks[i].api, apiHooks[i].detour) == 0)
		{
			tmp << "Error: Failed to hook " << apiHooks[i].apiName << "().";
			Logger::log(tmp.str());
			Logger::logFunctionLeave(__FUNCTION__);
			return S_OK;
		}
		tmp << "Hooked " << apiHooks[i].apiName << "().";
		Logger::log(tmp.str());
	}

	*success = true;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}


STDMETHODIMP CHookingEngine::unhookNetworkIO(BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	*success = false;

	//uninstall api-hooks
	for (int i=0; i<TO_BE_HOOKED_APIS; i++)
	{
		std::wstringstream tmp;
		if (nch->removeHook<PVOID>(apiHooks[i].detour) == false)
		{
			tmp << "Error: Failed to unhook " << apiHooks[i].apiName << "().";
			Logger::log(tmp.str());
			Logger::logFunctionLeave(__FUNCTION__);
			return S_OK;
		}
		tmp << "Unhooked " << apiHooks[i].apiName << "().";
		Logger::log(tmp.str());
	}

	//clean-up
	delete trafficHandler;
	delete sessions;
	delete nch;

	*success = true;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}


SAFEARRAY* CHookingEngine::getOpenConnectionsAsSafeArray(TTrafficDirection direction)
{
	Logger::logFunctionEnter(__FUNCTION__);

	map<unsigned int, IPConnection> conns;

	if (direction == OUTGOING)
	{
		//get outgoing connections
		trafficHandler->getOutgoingConnections(conns);
	}
	else if (direction == INCOMING)
	{
		//get incoming connections
		trafficHandler->getIncomingConnections(conns);
	}
	else
	{
		Logger::logFunctionLeave(__FUNCTION__);
		return NULL;
	}

	//create two dimensional array
	SAFEARRAY * psa;
	SAFEARRAYBOUND rgsabound[2];

	int sizeDim1 = 0;

	//calc size of dimension 1 by counting connections
	map<unsigned int, IPConnection>::iterator iter;
	for (iter = conns.begin(); iter !=  conns.end(); ++iter)
	{
		sizeDim1 += iter->second.getNumberOfConnections();
	}

	//1st dimension, connections
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = sizeDim1;

	//2nd dimension, connections details
	rgsabound[1].lLbound = 0;
	rgsabound[1].cElements = LENGTH_CONNECTION_DESCRIPTION;

	//create safearray
	if ((psa = SafeArrayCreate(VT_UI4, 2, rgsabound)) == NULL)
	{
		Logger::log(L"Error: Failed to create 2-dimensional array in getOpenConnectionsOutgoing().");
		Logger::logFunctionLeave(__FUNCTION__);
		return NULL;
	}

	//fill safearray
	int connectionIndex = 0;
	for (iter = conns.begin(); iter !=  conns.end(); ++iter)
	{
		const map<IPPROTO,ProtocolConnection>* protocols = iter->second.getProtocols();
		map<IPPROTO,ProtocolConnection>::const_iterator iterProto;

		for (iterProto = protocols->begin(); iterProto != protocols->end(); ++iterProto)
		{
			ProtocolConnection pc = iterProto->second;
			LONG indices[2];

			//add a single connection for each port
			const map<unsigned short, SourcePort>* sourcePorts = pc.getSourcePorts();
			map<unsigned short, SourcePort>::const_iterator iterSrcPorts;

			for(iterSrcPorts = sourcePorts->begin(); iterSrcPorts != sourcePorts->end(); ++iterSrcPorts)
			{
				const set<unsigned short>* ports = iterSrcPorts->second.getDestinationPorts();
				set<unsigned short>::const_iterator iterDstPorts;

				for(iterDstPorts = ports->begin(); iterDstPorts != ports->end(); ++iterDstPorts, connectionIndex++)
				{
					indices[0] = connectionIndex;
					//2st dimension, connection details
				
					indices[1] = 0; //set ip
					unsigned int ip = (unsigned int)pc.getIp().S_un.S_addr;
					if (SafeArrayPutElement(psa, indices, (void*)&ip) != S_OK)
					{
						SafeArrayDestroy(psa);
						Logger::log(L"Error: Failed to add ip to 2-dimensional array in getOpenConnectionsOutgoing().");
						Logger::logFunctionLeave(__FUNCTION__);
						return NULL;
					}

					indices[1] = 1; //set src port
					unsigned int srcPort = iterSrcPorts->second.getPort();
					if (SafeArrayPutElement(psa, indices, (void*)&srcPort) != S_OK)
					{
						SafeArrayDestroy(psa);
						Logger::log(L"Error: Failed to add port to 2-dimensional array in getOpenConnectionsOutgoing().");
						Logger::logFunctionLeave(__FUNCTION__);
						return NULL;
					}

					indices[1] = 2; //set dst port
					unsigned int dstPort = *iterDstPorts;
					if (SafeArrayPutElement(psa, indices, (void*)&dstPort) != S_OK)
					{
						SafeArrayDestroy(psa);
						Logger::log(L"Error: Failed to add port to 2-dimensional array in getOpenConnectionsOutgoing().");
						Logger::logFunctionLeave(__FUNCTION__);
						return NULL;
					}

					indices[1] = 3; //set protocol
					unsigned int proto = (unsigned int) pc.getProtocol();
					if (SafeArrayPutElement(psa, indices, (void*)&proto) != S_OK)
					{
						SafeArrayDestroy(psa);
						Logger::log(L"Error: Failed to add protocol to 2-dimensional array in getOpenConnectionsOutgoing().");
						Logger::logFunctionLeave(__FUNCTION__);
						return NULL;
					}
				}
			}
		}
	}
	Logger::logFunctionLeave(__FUNCTION__);
	return psa;
}

STDMETHODIMP CHookingEngine::getOpenConnectionsOutgoing(SAFEARRAY* * connections)
{
	Logger::logFunctionEnter(__FUNCTION__);
	SAFEARRAY* out;
	if ((out = getOpenConnectionsAsSafeArray(OUTGOING)) == NULL)
	{
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	*connections = out;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}

STDMETHODIMP CHookingEngine::getOpenConnectionsIncoming(SAFEARRAY* * connections)
{
	Logger::logFunctionEnter(__FUNCTION__);
	SAFEARRAY* out;
	if ((out = getOpenConnectionsAsSafeArray(INCOMING)) == NULL)
	{
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	*connections = out;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}


STDMETHODIMP CHookingEngine::resetConnections(BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	trafficHandler->resetIncomingConnections();
	trafficHandler->resetOugoingConnections();
	*success = true;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}


STDMETHODIMP CHookingEngine::addConnectionOutgoing(unsigned int sessionId, unsigned int ip, unsigned short sourcePort, unsigned short destinationPort, unsigned int protocol, BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	*success = false;

	if (sessions->hasSession(sessionId) != true)
	{
		Logger::log(L"Error: Tried to add an outgoing connection to an unknown session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	if (protocol >= IPPROTO_RESERVED_MAX)
	{
		Logger::log(L"Error: Failed to add outgoing connection to session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	in_addr _ip;
	_ip.S_un.S_addr = ip;
	Connection conn(_ip, sourcePort, destinationPort, (IPPROTO) protocol);

	//check ongoing sessions
	unsigned int tempSessionId;
	if (trafficHandler->ownedBySessionOutgoing(conn, tempSessionId) == true)
	{
		Logger::log(L"Error: Connection already owned by other ongoing session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	//check not yet running sessions
	if (sessions->ownedBySessionOutgoing(conn, tempSessionId) == true)
	{
		Logger::log(L"Error: Connection already owned by other not yet started session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	sessions->addOutgoingConnection(sessionId, conn);

	*success = true;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}

STDMETHODIMP CHookingEngine::addConnectionIncoming(unsigned int sessionId, unsigned int ip, unsigned short sourcePort, unsigned short destinationPort, unsigned int protocol, BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	*success = false;

	if (sessions->hasSession(sessionId) != true)
	{
		Logger::log(L"Error: Tried to add an incoming connection to an unknown session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	if (protocol >= IPPROTO_RESERVED_MAX)
	{
		Logger::log(L"Error: Failed to add incoming connection to session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	in_addr _ip;
	_ip.S_un.S_addr = ip;
	Connection conn(_ip, sourcePort, destinationPort, (IPPROTO) protocol);

	//check ongoing sessions
	unsigned int tempSessionId;
	if (trafficHandler->ownedBySessionIncoming(conn, tempSessionId) == true)
	{
		Logger::log(L"Error: Connection already owned by other ongoing session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	//check not yet running sessions
	if (sessions->ownedBySessionIncoming(conn, tempSessionId) == true)
	{
		Logger::log(L"Error: Connection already owned by other not yet started session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	sessions->addIncomingConnection(sessionId, conn);

	*success = true;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}


STDMETHODIMP CHookingEngine::createSession(BOOL expandable, unsigned int* sessionId)
{
	Logger::logFunctionEnter(__FUNCTION__);
	*sessionId = sessions->newSession((expandable == 1));
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}


STDMETHODIMP CHookingEngine::startSession(unsigned int sessionId, SAFEARRAY * key, BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	*success = false;

	if (trafficHandler->readyToEncrypt() != true)
	{
		Logger::log(L"Fatal Error: Could not start secure session because of the cryptographic provider not being ready.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	if (sessions->hasSession(sessionId) != true)
	{
		Logger::log(L"Fatal Error: Tried to add an incoming connection to an unknown session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	if (key->cDims != 1)
	{
		Logger::log(L"Fatal Error: Key array is of wrong format.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	BYTE* data;
	if (SafeArrayAccessData(key,(void HUGEP**)&data) != S_OK)
	{
		Logger::log(L"Fatal Error: Failed to access key data.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}
	const unsigned int keyLength = key->cbElements * key->rgsabound[0].cElements;
	Key k(data, keyLength);
	SafeArrayUnaccessData(key);
	if (k.length != keyLength)
	{
		Logger::log(L"Fatal Error: Failed to import key.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	sessions->setKey(sessionId, k);
	
	Session session;
	if (sessions->getSession(sessionId, session) != true)
	{
		Logger::log(L"Fatal Error: Unknown error while destroying session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return false;
	}

	activeSessions++;
	stopClock(STOP_CLOCK_DELAY);
	trafficHandler->addEncryptedSession(session);
	*success = true;

	Logger::logFunctionLeave(__FUNCTION__);
	
	return S_OK;
}


STDMETHODIMP CHookingEngine::destroySession(unsigned int sessionId, BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	*success = false;
	if (sessions->hasSession(sessionId) != true)
	{
		Logger::log(L"Error: Could not destroy unknown session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return S_FALSE;
	}

	Session session;
	if (sessions->getSession(sessionId, session) != true)
	{
		Logger::log(L"Error: Unknown error while destroying session.");
		Logger::logFunctionLeave(__FUNCTION__);
		return false;
	}
	trafficHandler->removeEncryptedSession(session);
	activeSessions--;
	if (activeSessions == 0)
	{
		//HACK: clock needs to be resumed once there are no more TIGHT sessions
		resumeClock();
	}
	sessions->deleteSession(sessionId);
	*success = true;
	Logger::logFunctionLeave(__FUNCTION__);
	return S_OK;
}


STDMETHODIMP CHookingEngine::init(BSTR baseDir, BOOL* success)
{
	Logger::init(baseDir);
	Logger::reset();

	*success = true;
	return S_OK;
}


STDMETHODIMP CHookingEngine::setSecurityPolicy(int securityPolicy, BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	*success = trafficHandler->setSecurityPolicy((TSecurityPolicy)securityPolicy);
	return S_OK;
}


STDMETHODIMP CHookingEngine::startCountingConnections(BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	trafficHandler->setCountConnections(true);
	*success = true;
	return S_OK;
}


STDMETHODIMP CHookingEngine::stopCountingConnections(BOOL* success)
{
	Logger::logFunctionEnter(__FUNCTION__);
	trafficHandler->setCountConnections(false);
	*success = true;
	return S_OK;
}


STDMETHODIMP CHookingEngine::getMostUsedPeerIP(unsigned int* ip)
{
	Logger::logFunctionEnter(__FUNCTION__);
	map<unsigned int, unsigned int> counts;
	trafficHandler->getConnectionCounts(counts);

	unsigned int mostUsedIp = 0;
	unsigned int timesUsed = 0;

	map<unsigned int, unsigned int>::iterator iter;
	for (iter = counts.begin(); iter != counts.end(); ++iter)
	{
		if (iter->second > timesUsed)
		{
			timesUsed = iter->second;
			mostUsedIp = iter->first;
		}
	}

	*ip = mostUsedIp;
	return S_OK;
}
