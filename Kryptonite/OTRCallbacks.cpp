#include "StdAfx.h"
#include "OTRCallbacks.h"
#include "Logger.h"
#include "HelperFunctions.h"
#include "OTRCompaFileIO.h"

//static members init
LocalUser* OTRCallbacks::user = NULL;
std::map<std::string, Conversation*>* OTRCallbacks::conversations = NULL;

OtrlMessageAppOps OTRCallbacks::callbacks = {
	OTRCallbacks::policy,
	OTRCallbacks::create_privkey,
	OTRCallbacks::is_logged_in,
	OTRCallbacks::inject_message,
    OTRCallbacks::notify,
	OTRCallbacks::display_otr_message,
	OTRCallbacks::update_context_list,
	OTRCallbacks::protocol_name,
	OTRCallbacks::protocol_name_free,
	OTRCallbacks::new_fingerprint,
	OTRCallbacks::write_fingerprints,
	OTRCallbacks::gone_secure,
	OTRCallbacks::gone_insecure,
	OTRCallbacks::still_secure,
	OTRCallbacks::log_message,
	OTRCallbacks::max_message_size,
	OTRCallbacks::account_name,
	OTRCallbacks::account_name_free
};
/////////////////////

OTRCallbacks::OTRCallbacks(LocalUser* user)
{
	this->user = user;
	this->conversations = new std::map<std::string, Conversation*>();
}

OTRCallbacks::~OTRCallbacks()
{
	std::map<std::string, Conversation*>::iterator iter;

	//delete conversation objects
	for (iter = conversations->begin(); iter != conversations->end(); iter++)
	{
		delete iter->second;
	}
	delete this->conversations;
}

bool OTRCallbacks::openConversation(const std::string &partner)
{
	if (conversations->find(partner) != conversations->end())
	{
#ifdef DEBUG
		Logger::log(L"Error: Tried to open a new conversation for a name that is already associated with ongoing conversation.");
#endif
		return false;
	}

	Conversation* con = new Conversation(partner);
	(*conversations)[partner] = con;
	return true;
}

bool OTRCallbacks::closeConversation(const std::string &partner)
{
	Conversation* con;
	if (conversations->find(partner) == conversations->end())
	{
#ifdef DEBUG
		Logger::log(L"Error: Tried close a conversation for a name that no ongoing conversation is associated with.");
#endif
		return false;
	}

	//TODO: write fingerprints?

	//get corresponding context
	ConnContext* cc = otrl_context_find(user->getState(), partner.c_str(), user->getName().c_str(), OTR_PROTOCOL_NAME, 0, NULL, NULL, NULL);
	if (cc == NULL)
	{
		Logger::log(L"Error: Could not get user-context.");
		return S_FALSE;
	}

	otrl_context_force_plaintext(cc);
	otrl_context_forget(cc);

	conversations->erase(partner);

	return true;
}

bool OTRCallbacks::startAKE(const std::string &partner)
{
	char* queryMsg = otrl_proto_default_query_msg(user->getName().c_str(), OTR_POLICY);
	
	if (queryMsg == NULL)
	{
		Logger::log(L"Error: Failed to initiate OTR AKE.");
		return false;
	}
	inject_message(NULL, user->getName().c_str(), OTR_PROTOCOL_NAME, partner.c_str(), queryMsg);
	
	otrl_message_free(queryMsg);
	return true;
}

bool OTRCallbacks::encryptMessage(const std::string &partner, std::string &message)
{
	char* tempMessage = (char*) malloc(message.length()+1);
	if (tempMessage == NULL)
	{
#ifdef DEBUG
		Logger::log(L"Error: Failed to alloc memory for message buffer.");
#endif
		return false;
	}
	if (strcpy_s(tempMessage, message.length()+1, message.c_str()) != 0)
	{
#ifdef DEBUG
		Logger::log(L"Error: Failed to copy outgoing message.");
#endif
		free(tempMessage);
		return false;
	}

	char* newMessage = NULL;

	if (otrl_message_sending(user->getState(), &callbacks, NULL, user->getName().c_str(), OTR_PROTOCOL_NAME, 
		partner.c_str(), tempMessage, NULL, &newMessage, NULL, NULL) != GPG_ERR_NO_ERROR)
	{
		Logger::log(L"Error: Failed to encrypt outgoing message!");
		return false;
	}

	if (newMessage == NULL)
	{
		//message was encrypted in place
		message = tempMessage;
	}
	else
	{
		//encrypted message is longer than plain message -> OTR created a new buffer
		message = newMessage;
		otrl_message_free(newMessage);
	}
	free(tempMessage);
	return true;
}

bool OTRCallbacks::decryptMessage(const std::string &partner, std::string &message, bool &deploy)
{
#ifdef DEBUG
	Logger::log(L"Entered decryptMessage().");
#endif
	deploy = false;

	char* tempMessage = (char*) malloc(message.length()+1);
	if (tempMessage == NULL)
	{
#ifdef DEBUG
		Logger::log(L"Error: Failed to alloc memory for message buffer.");
#endif
		return false;
	}
	if (strcpy_s(tempMessage, message.length()+1, message.c_str()) != 0)
	{
#ifdef DEBUG
		Logger::log(L"Error: Failed to copy outgoing message.");
#endif
		free(tempMessage);
		return false;
	}

	char* newMessage = NULL;

#ifdef DEBUG
	Logger::log(L"\tCalling otrl_message_receiving().");
#endif

	int messageStatus = otrl_message_receiving(user->getState(), &callbacks, NULL, user->getName().c_str(), OTR_PROTOCOL_NAME, 
		partner.c_str(),tempMessage, &newMessage, NULL, NULL, NULL);

#ifdef DEBUG
	Logger::log(L"\tReturned from otrl_message_receiving().");
#endif

	if (messageStatus == 1)
	{
		//OTR recognized the message as an internal protocol message - everything is fine
	}
	else if (messageStatus == 0) 
	{
		//OTR did not recongize the message as one of its own -> deploy it
		deploy = true;
		//The message does not belong to any 
		if (newMessage == NULL)
		{
			//message was encrypted in place
			message = tempMessage;
		}
		else
		{
			//encrypted message is longer than plain message -> OTR created a new buffer
			message = newMessage;
			otrl_message_free(newMessage);
		}
	}
	else
	{
		//something went wrong
		Logger::log(L"Error: otrl_message_receiving() returned an unexpected value.");
		free(tempMessage);
		return false;
	}

	free(tempMessage);

#ifdef DEBUG
	Logger::log(L"Leaving decryptMessage().");
#endif
	return true;
}

bool OTRCallbacks::getConversation(const std::string &partner, Conversation** conversation)
{
	if (conversations->find(partner) == conversations->end())
	{
#ifdef DEBUG
		Logger::log(L"Error: Tried to get a non existant conversation.");
#endif
		return false;
	}

	*conversation = conversations->at(partner);
	return true;
}


bool OTRCallbacks::confirmFingerprint(const std::string &partner, const std::string &fingerprint)
{
	if (conversations->find(partner) == conversations->end())
	{
		Logger::log(L"Error: Tried to confirm a fingerprint for a non existant conversation.");
		return false;
	}

	(*conversations)[partner]->confirmFingerprint(fingerprint);
	updateFingerprintStore();
	return true;
}

bool OTRCallbacks::updateFingerprintStore()
{
	//NOTE: Thread-safety is achieved due to this component being a "Single-Threaded Apartment" COM module

	//only write fingerprints if we have no unconfirmed fingerprints in any conversation!
	std::map<std::string, Conversation*>::iterator iter;
	int unconfirmedFP = 0;
	for (iter = conversations->begin(); iter != conversations->end(); iter++)
	{
		unconfirmedFP += iter->second->getNumberUnconfirmedFingerprints();
	}

	//got no unconfirmed fingerprints?
	if (unconfirmedFP != 0)
	{
#ifdef DEBUG
		Logger::log(L"Info: Aborted write to fingerprints store because of missing confirmations.");
#endif
		return false;
	}
	FILE* fingerprintsFile;
	if (OTRCompaFileIO::_wfopen_s(&fingerprintsFile, user->getPathFingerprintsFile().c_str(), L"w+") == EINVAL)
	{
		Logger::log(L"Error: Failed to update fingerprints-file on OTR's request");
		return false;
	}

	gcry_error_t error = otrl_privkey_write_fingerprints_FILEp(user->getState(), fingerprintsFile);
	OTRCompaFileIO::fclose(fingerprintsFile);

	if (error != GPG_ERR_NO_ERROR)
	{
		//TODO: Give feedback to user
		Logger::log(L"Error: OTR failed to update fingerprints file.");
		return false;
	}

	return true;
}

/**
* These implementations borrow heaviliy from otr-plugin.c of the libOTR
* for pidgin plugin.
**/

OtrlPolicy OTRCallbacks::policy(void *opdata, ConnContext *context)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	//TODO: Revisit?
	return OTR_POLICY;
}

void OTRCallbacks::create_privkey(void *opdata, const char *accountname,
		const char *protocol) 
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	//TODO: Dunno if this is needed, because private keys are already in Crypto::init()
}

int OTRCallbacks::is_logged_in(void *opdata, const char *accountname,
	    const char *protocol, const char *recipient)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	//The user we're talking to should always be logged in -> return 1
	return 1;
}

void OTRCallbacks::inject_message(void *opdata, const char *accountname,
		const char *protocol, const char *recipient, const char *message)

{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	if (conversations->find(recipient) == conversations->end())
	{
		//recipient is unknown
		Logger::log(L"Error: OTR requested to send a message to an unknown recipient. This should never happen.");
		return;
	}

	//recipient is known -> add message
	std::string msg = message;
	(*conversations)[recipient]->addOutgoingMessage(msg);
}

void OTRCallbacks::notify(void *opdata, OtrlNotifyLevel level,
	    const char *accountname, const char *protocol,
	    const char *username, const char *title,
		const char *primary, const char *secondary)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	if (conversations->find(username) == conversations->end())
	{
		//recipient is unknown
		Logger::log(L"Error: OTR requested to display a notification concerning an unknown username. This should never happen.");
		return;
	}

	std::string notification = "";
	switch (level) {
	case OTRL_NOTIFY_ERROR:
	    notification += "Error: ";
	    break;
	case OTRL_NOTIFY_WARNING:
	    notification += "Warning: ";
	    break;
	case OTRL_NOTIFY_INFO:
	    notification += "Info: ";
	    break;
	default:
		notification += "Unknown :";
    }

	if (title != NULL)
	{
		if (strlen(title) +1 + notification.size() >= notification.max_size())
		{
			Logger::log(L"Error: Received a notification that exceeds std::string::max_size().");
			return;
		}
		notification += title;
		notification += ":";
	}

	if (primary != NULL)
	{
		if (strlen(primary) +1 + notification.size() >= notification.max_size())
		{
			Logger::log(L"Error: Received a notification that exceeds std::string::max_size().");
			return;
		}
		notification += primary;
		notification += ":";
	}

	if (secondary != NULL)
	{
		if (strlen(secondary) + notification.size() >= notification.max_size())
		{
			Logger::log(L"Error: Received a notification that exceeds std::string::max_size().");
			return;
		}
		notification += secondary;
	}
	
	(*conversations)[username]->addNotification(notification);
#ifdef DEBUG
	Logger::log(L"Leaving notify().");
#endif DEBUG
}

int OTRCallbacks::display_otr_message(void *opdata, const char *accountname,
		const char *protocol, const char *username, const char *msg)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	notify(opdata, OTRL_NOTIFY_INFO, accountname, protocol, username, "", msg, "");
	return 1;
#ifdef DEBUG
	Logger::log(L"Leaving display_otr_message().");
#endif DEBUG
}

void OTRCallbacks::update_context_list(void *opdata)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	//TODO: Do something
	Logger::log(L"Info: OTR context list was updated. This should have an effect.");
}

const char* OTRCallbacks::protocol_name(void *opdata, const char *protocol) 
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	return OTR_PROTOCOL_NAME;
}

void OTRCallbacks::protocol_name_free(void *opdata, const char *protocol_name) 
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	//do nothing
}

void OTRCallbacks::new_fingerprint(void *opdata, OtrlUserState us,
	    const char *accountname, const char *protocol,
		const char *username, unsigned char fingerprint[20])
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	if (conversations->find(username) == conversations->end())
	{
		//recipient is unknown
		Logger::log(L"Error: OTR wants confirmation for a new fingerprint that does not belong to any known user. This should never happen.");
		return;
	}	
	char fp[45];
	otrl_privkey_hash_to_human(fp, fingerprint);
	std::string strFP = fp;

	(*conversations)[username]->addUnconfirmedFingerprint(strFP);
}

void OTRCallbacks::write_fingerprints(void *opdata)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	updateFingerprintStore();
}

void OTRCallbacks::gone_secure(void *opdata, ConnContext *context)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	if (conversations->find(context->username) == conversations->end())
	{
		//recipient is unknown
		Logger::log(L"Error: OTR got a secure communication with an unknown user. This should never happen.");
		return;
	}
	Event<EventTypeSpecific> newEvent(INFO_CONNECTION_SECURE, "");
	(*conversations)[context->username]->addEvent(newEvent);
}

void OTRCallbacks::gone_insecure(void *opdata, ConnContext *context)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	if (conversations->find(context->username) == conversations->end())
	{
		//recipient is unknown
		Logger::log(L"Error: OTR got an insecure communication with an unknown user. This should never happen.");
		return;
	}
	//TODO: should the panic event be flagged here?
	Event<EventTypeSpecific> newEvent(INFO_CONNECTION_INSECURE, "");
	(*conversations)[context->username]->addEvent(newEvent);
}

void OTRCallbacks::still_secure(void *opdata, ConnContext *context, int is_reply)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	if (conversations->find(context->username) == conversations->end())
	{
		//recipient is unknown
		Logger::log(L"Error: OTR pretends to have an ongoing insecure communication with an unknown user. This should never happen.");
		return;
	}
	Event<EventTypeSpecific> newEvent(INFO_CONNECTION_STILL_SECURE, "");
	(*conversations)[context->username]->addEvent(newEvent);
}

void OTRCallbacks::log_message(void *opdata, const char *message)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif

	std::string msg = message;
	Logger::log(HelperFunctions::stringToWstring(msg).c_str());
}

int OTRCallbacks::max_message_size(void *opdata, ConnContext *context)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	return MAX_MSG_SIZE;
}

const char* OTRCallbacks::account_name(void *opdata, const char *account, const char *protocol)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	char* accountName = (char*)malloc(user->getName().length()+1);
	strcpy_s(accountName, user->getName().length()+1, user->getName().c_str());

	return accountName;
}

void OTRCallbacks::account_name_free(void *opdata, const char *account_name)
{
#ifdef DEBUG
	Logger::log( __FUNCTION__ );
#endif
	free((void*)account_name);
}