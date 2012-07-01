#pragma once
#include "libOTR.h"
#include <map>
#include <string>
#include "User.h"
#include "Conversation.h"

//! Static class for interaction with libOTR.
class OTRCallbacks
{
private:
	//TODO: does access have to be thread-save?
	static std::map<std::string, Conversation*>* conversations;
	static LocalUser* user;
	static OtrlMessageAppOps callbacks;

public:

	OTRCallbacks(LocalUser* user);
	~OTRCallbacks();

	//! Opens a new conversation with the given partner.
	bool openConversation(const std::string &partner);

	//! Closes the conversation with the given partner
	bool closeConversation(const std::string &partner);

	/*! Starts an AKE with the given partner.
		\param partner The name of the partner to start an AKE with.
		\returns A flag indicating success. */
	bool startAKE(const std::string &partner);

	/*! Encrypts a message for the given conversation partner.
		\param partner The name of the conversation partner.
		\param message The message to encrypt.
		\returns A flag indicating success. */
	bool encryptMessage(const std::string &partner, std::string &message);

	/*! Decrypts a message from the given conversation partner.
		\param partner The name of the conversation partner.
		\param message The message to decrypt (decrypted string is return in same parameter).
		\param deploy [returns] A flag indicating whether this message should be deployed to the user.
		\returns A flag indicating success. */
	bool decryptMessage(const std::string &partner, std::string &message, bool &deploy);

	/*! Gets the conversation associated with the given partner.
		\param partner The name of the conversation partner.
		\param conversation [returns] The requested conversation.
		\returns A flag indicating success. */
	bool getConversation(const std::string &partner, Conversation** conversation);

	/*! Confirms the given fingerprint for the given conversatio partner.
		\param partner The name of the conversation partner.
		\param fingerprint The fingerprint to be confirmed.
		\returns A flag indicating success. */
	bool confirmFingerprint(const std::string &partner, const std::string &fingerprint);

	//! Updates the fingerprint store on the harddisk.
	static bool updateFingerprintStore();

private:
	/************
	* callbacks *
	************/
	static OtrlPolicy policy(void *opdata, ConnContext *context);

    /* Create a private key for the given accountname/protocol if
     * desired. */
    static void create_privkey(void *opdata, const char *accountname,
		const char *protocol);

    /* Report whether you think the given user is online.  Return 1 if
     * you think he is, 0 if you think he isn't, -1 if you're not sure.
     *
     * If you return 1, messages such as heartbeats or other
     * notifications may be sent to the user, which could result in "not
     * logged in" errors if you're wrong. */
    static int is_logged_in(void *opdata, const char *accountname,
		const char *protocol, const char *recipient);

    /* Send the given IM to the given recipient from the given
     * accountname/protocol. */
    static void inject_message(void *opdata, const char *accountname,
		const char *protocol, const char *recipient, const char *message);

    /* Display a notification message for a particular accountname /
     * protocol / username conversation. */
    static void notify(void *opdata, OtrlNotifyLevel level,
	    const char *accountname, const char *protocol,
	    const char *username, const char *title,
		const char *primary, const char *secondary);

    /* Display an OTR control message for a particular accountname /
     * protocol / username conversation.  Return 0 if you are able to
     * successfully display it.  If you return non-0 (or if this
     * function is NULL), the control message will be displayed inline,
     * as a received message, or else by using the above notify()
     * callback. */
    static int display_otr_message(void *opdata, const char *accountname,
		const char *protocol, const char *username, const char *msg);

    /* When the list of ConnContexts changes (including a change in
     * state), this is called so the UI can be updated. */
	static void update_context_list(void *opdata);

    /* Return a newly allocated string containing a human-friendly name
     * for the given protocol id */
	static const char* protocol_name(void *opdata, const char *protocol);

    /* Deallocate a string allocated by protocol_name */
	static void protocol_name_free(void *opdata, const char *protocol_name);

    /* A new fingerprint for the given user has been received. */
    static void new_fingerprint(void *opdata, OtrlUserState us,
	    const char *accountname, const char *protocol,
		const char *username, unsigned char fingerprint[20]);

    /* The list of known fingerprints has changed.  Write them to disk. */
	static void write_fingerprints(void *opdata);

    /* A ConnContext has entered a secure state. */
	static void gone_secure(void *opdata, ConnContext *context);

    /* A ConnContext has left a secure state. */
	static void gone_insecure(void *opdata, ConnContext *context);

    /* We have completed an authentication, using the D-H keys we
     * already knew.  is_reply indicates whether we initiated the AKE. */
	static void still_secure(void *opdata, ConnContext *context, int is_reply);

    /* Log a message.  The passed message will end in "\n". */
	static void log_message(void *opdata, const char *message);

    /* Find the maximum message size supported by this protocol. */
	static int max_message_size(void *opdata, ConnContext *context);

    /* Return a newly allocated string containing a human-friendly
     * representation for the given account */
    static const char* account_name(void *opdata, const char *account,
		const char *protocol);

    /* Deallocate a string returned by account_name */
	static void account_name_free(void *opdata, const char *account_name);
	
};

