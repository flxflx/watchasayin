#pragma once

#include "stdafx.h"
#include <string>
#include <vector>
#include <map>
#include <set>

#include "Event.h"

//! Defines a conversation with one specific partner (thread-safe). 
/*! Purpose is to store to be processed events, to be sent messages, to be displayed user notifications and to be confirmed fingerprints. */
class Conversation
{
private:
	std::string partner;

	//cache of outgoing message for different recepients
	std::vector<std::string> outgoingMessageCache;
	CRITICAL_SECTION csMessageCache;

	//cache of notification messages for the user
	std::vector<std::string> notificationMessageCache;
	CRITICAL_SECTION csNotificationCache;

	//cache of events to perform by GUI
	std::vector<Event<EventTypeSpecific>> eventCache;
	CRITICAL_SECTION csEventCache;

	//cache of unconfirmed fingerprints
	std::set<std::string> unconfirmedFingerprints;
	CRITICAL_SECTION csUnconfirmedFingerprints;

	void initCriticalSections()
	{
		InitializeCriticalSection(&csMessageCache);
		InitializeCriticalSection(&csNotificationCache);
		InitializeCriticalSection(&csEventCache);
		InitializeCriticalSection(&csUnconfirmedFingerprints);
	}

public:

	Conversation()
	{
		initCriticalSections();
	}

	/*! The public constructor.
		\param partner The handle of the partner of the conversation. */
	Conversation(const std::string &partner)
	{
		initCriticalSections();
		this->partner = partner;
	}

	~Conversation()
	{
		DeleteCriticalSection(&csMessageCache);
		DeleteCriticalSection(&csNotificationCache);
		DeleteCriticalSection(&csEventCache);
		DeleteCriticalSection(&csUnconfirmedFingerprints);
	}

	//! Adds a new event.
	void addEvent(Event<EventTypeSpecific> &newEvent)
	{
		EnterCriticalSection(&csEventCache);

		eventCache.insert(eventCache.begin(),newEvent);

		EnterCriticalSection(&csEventCache);
	}

	//! Gets the latest event and deletes it.
	Event<EventTypeSpecific> getLastEvent()
	{
		EnterCriticalSection(&csEventCache);

		Event<EventTypeSpecific> lastEvent = eventCache.back();
		eventCache.pop_back();

		LeaveCriticalSection(&csEventCache);
		return lastEvent;
	}

	//! Gets the number of yet to be processed events.
	int getSizeEventCache()
	{
		EnterCriticalSection(&csEventCache);

		int size = eventCache.size();

		LeaveCriticalSection(&csEventCache);

		return size;
	}

	//! Adds a to be sent message.
	void addOutgoingMessage(const std::string &message)
	{
		EnterCriticalSection(&csMessageCache);

		outgoingMessageCache.insert(outgoingMessageCache.begin(), message);

		LeaveCriticalSection(&csMessageCache);
	}

	//! Gets the latest message to be sent and deletes it.
	std::string getLastOutgoingMessage()
	{
		EnterCriticalSection(&csMessageCache);

		std::string message = outgoingMessageCache.back();
		outgoingMessageCache.pop_back();

		LeaveCriticalSection(&csMessageCache);
		return message;
	}

	//! Gets the number of yet to be sent messages.
	int getSizeOutgoingMessageCache()
	{
		EnterCriticalSection(&csMessageCache);

		int size = outgoingMessageCache.size();

		LeaveCriticalSection(&csMessageCache);

		return size;
	}

	//! Adds a to be display notification.
	void addNotification(const std::string &notification)
	{
		EnterCriticalSection(&csNotificationCache);

		notificationMessageCache.insert(notificationMessageCache.begin(), notification);

		LeaveCriticalSection(&csNotificationCache);
	}

	//! Gets and deletes the last notification.
	std::string getLastNotification()
	{
		EnterCriticalSection(&csNotificationCache);

		std::string notification = notificationMessageCache.back();
		notificationMessageCache.pop_back();

		LeaveCriticalSection(&csNotificationCache);
		return notification;
	}

	//! Gets the number of yet to be displayed notifications.
	int getSizeNotificationCache()
	{
		EnterCriticalSection(&csNotificationCache);

		int size = notificationMessageCache.size();

		LeaveCriticalSection(&csNotificationCache);

		return size;
	}

	//! Adds an unconfirmed fingerprint
	void addUnconfirmedFingerprint(const std::string &fingerprint)
	{
		//add fingerprint to set of unconfirmed fingerprints
		EnterCriticalSection(&csUnconfirmedFingerprints);
		unconfirmedFingerprints.insert(fingerprint);
		LeaveCriticalSection(&csUnconfirmedFingerprints);

		//add corresponding event
		Event<EventTypeSpecific> newEvent(DLG_CONFIRM_NEW_FINGERPRINT, fingerprint);
		addEvent(newEvent);
	}

	//! Gets the number of fingerprints still waiting to be confirmed.
	int getNumberUnconfirmedFingerprints()
	{
		int n;
		EnterCriticalSection(&csUnconfirmedFingerprints);
		n = unconfirmedFingerprints.size();
		LeaveCriticalSection(&csUnconfirmedFingerprints);
		return n;
	}

	//! Confirms the given fingperprint.
	void confirmFingerprint(const std::string &fingerprint)
	{
		//remove fingerprint to set of unconfirmed fingerprints
		EnterCriticalSection(&csUnconfirmedFingerprints);
		unconfirmedFingerprints.erase(fingerprint);
		LeaveCriticalSection(&csUnconfirmedFingerprints);
	}
};
