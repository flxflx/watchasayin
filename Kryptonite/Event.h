#pragma once
#include <string>

//! Possible types of conversation specific events.
typedef enum EventTypeSpecific {
	DLG_CONFIRM_NEW_FINGERPRINT = 20, /*!< A dialog asking for the confirmation of the fingerprint needs to be shown to the user.*/
	INFO_CONNECTION_SECURE = 21, /*!< The connection is now secure.*/
	INFO_CONNECTION_STILL_SECURE = 22, /*!< The connection is still secure.*/
	INFO_CONNECTION_INSECURE = 23 /*!< The connection is now insecure.*/
};

//! Possible types of global events
typedef enum EventTypeGeneric{
	UNKNOWN = 0, /*!< The unknown event.*/
	DLG_NO_PRIVATE_KEY_FOUND = 1, /*!< A dialog informing the user that no private key was found for needs to be displayed.*/
	DLG_PRIVATE_KEY_INVALID = 2, /*!< A dialog informing the user that their private key is invalid needs to be displayed.*/
	DLG_NO_FINGERPRINTS_FOUND = 3, /*!< A dialog informing the user that no fingerprint store was found needs to be displayed.*/
	DLG_FINGERPRINTS_INVALID = 4 /*!< A dialog informing the user that the fingeprint store is invalid needs to be displayed.*/
};

//! Describes an event.
template <class T> class Event
{
public:
	T type;
	std::string data;

	/*! The public constructor.
		\param type The type of the event.
		\param data the data associated with the event. */
	Event(T type, std::string data)
	{
		this->type = type;
		this->data = data;
	}
};