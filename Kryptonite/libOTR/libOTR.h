#pragma once
#include "gpg-error.h"

/****************
* From version.h *
*****************/

#define OTRL_VERSION "3.2.0"

#define OTRL_VERSION_MAJOR 3
#define OTRL_VERSION_MINOR 2
#define OTRL_VERSION_SUB 0

/****************
* From gcrypt.h *
*****************/

struct gcry_mpi;
typedef struct gcry_mpi *gcry_mpi_t;

/* The data object used to hold a handle to an encryption object.  */
struct gcry_cipher_handle;
typedef struct gcry_cipher_handle *gcry_cipher_hd_t;

/* All symmetric encryption algorithms are identified by their IDs.
   More IDs may be registered at runtime. */
enum gcry_cipher_algos
  {
    GCRY_CIPHER_NONE        = 0,
    GCRY_CIPHER_IDEA        = 1,
    GCRY_CIPHER_3DES        = 2,
    GCRY_CIPHER_CAST5       = 3,
    GCRY_CIPHER_BLOWFISH    = 4,
    GCRY_CIPHER_SAFER_SK128 = 5,
    GCRY_CIPHER_DES_SK      = 6,
    GCRY_CIPHER_AES         = 7,
    GCRY_CIPHER_AES192      = 8,
    GCRY_CIPHER_AES256      = 9,
    GCRY_CIPHER_TWOFISH     = 10,

    /* Other cipher numbers are above 300 for OpenPGP reasons. */
    GCRY_CIPHER_ARCFOUR     = 301,  /* Fully compatible with RSA's RC4 (tm). */
    GCRY_CIPHER_DES         = 302,  /* Yes, this is single key 56 bit DES. */
    GCRY_CIPHER_TWOFISH128  = 303,
    GCRY_CIPHER_SERPENT128  = 304,
    GCRY_CIPHER_SERPENT192  = 305,
    GCRY_CIPHER_SERPENT256  = 306,
    GCRY_CIPHER_RFC2268_40  = 307,  /* Ron's Cipher 2 (40 bit). */
    GCRY_CIPHER_RFC2268_128 = 308,  /* Ron's Cipher 2 (128 bit). */
    GCRY_CIPHER_SEED        = 309,  /* 128 bit cipher described in RFC4269. */
    GCRY_CIPHER_CAMELLIA128 = 310,
    GCRY_CIPHER_CAMELLIA192 = 311,
    GCRY_CIPHER_CAMELLIA256 = 312
  };

/* The Rijndael algorithm is basically AES, so provide some macros. */
#define GCRY_CIPHER_AES128      GCRY_CIPHER_AES    
#define GCRY_CIPHER_RIJNDAEL    GCRY_CIPHER_AES    
#define GCRY_CIPHER_RIJNDAEL128 GCRY_CIPHER_AES128 
#define GCRY_CIPHER_RIJNDAEL192 GCRY_CIPHER_AES192 
#define GCRY_CIPHER_RIJNDAEL256 GCRY_CIPHER_AES256 

/* The supported encryption modes.  Note that not all of them are
   supported for each algorithm. */
enum gcry_cipher_modes 
  {
    GCRY_CIPHER_MODE_NONE   = 0,  /* Not yet specified. */
    GCRY_CIPHER_MODE_ECB    = 1,  /* Electronic codebook. */
    GCRY_CIPHER_MODE_CFB    = 2,  /* Cipher feedback. */
    GCRY_CIPHER_MODE_CBC    = 3,  /* Cipher block chaining. */
    GCRY_CIPHER_MODE_STREAM = 4,  /* Used with stream ciphers. */
    GCRY_CIPHER_MODE_OFB    = 5,  /* Outer feedback. */
    GCRY_CIPHER_MODE_CTR    = 6   /* Counter. */
  };

/* Flags used with the open function. */ 
enum gcry_cipher_flags
  {
    GCRY_CIPHER_SECURE      = 1,  /* Allocate in secure memory. */
    GCRY_CIPHER_ENABLE_SYNC = 2,  /* Enable CFB sync mode. */
    GCRY_CIPHER_CBC_CTS     = 4,  /* Enable CBC cipher text stealing (CTS). */
    GCRY_CIPHER_CBC_MAC     = 8   /* Enable CBC message auth. code (MAC). */
  };

/* Algorithm IDs for the hash functions we know about. Not all of them
   are implemnted. */
enum gcry_md_algos
  {
    GCRY_MD_NONE    = 0,  
    GCRY_MD_MD5     = 1,
    GCRY_MD_SHA1    = 2,
    GCRY_MD_RMD160  = 3,
    GCRY_MD_MD2     = 5,
    GCRY_MD_TIGER   = 6,   /* TIGER/192. */
    GCRY_MD_HAVAL   = 7,   /* HAVAL, 5 pass, 160 bit. */
    GCRY_MD_SHA256  = 8,
    GCRY_MD_SHA384  = 9,
    GCRY_MD_SHA512  = 10,
    GCRY_MD_SHA224  = 11,
    GCRY_MD_MD4     = 301,
    GCRY_MD_CRC32         = 302,
    GCRY_MD_CRC32_RFC1510 = 303,
    GCRY_MD_CRC24_RFC2440 = 304,
    GCRY_MD_WHIRLPOOL = 305
  };

/* Flags used with the open function.  */
enum gcry_md_flags
  {
    GCRY_MD_FLAG_SECURE = 1,  /* Allocate all buffers in "secure" memory.  */
    GCRY_MD_FLAG_HMAC   = 2   /* Make an HMAC out of this algorithm.  */
  };

/* (Forward declaration.)  */
struct gcry_md_context;

/* This object is used to hold a handle to a message digest object.
   This structure is private - only to be used by the public gcry_md_*
   macros.  */
typedef struct gcry_md_handle 
{
  /* Actual context.  */
  struct gcry_md_context *ctx;
  
  /* Buffer management.  */
  int  bufpos;
  int  bufsize;
  unsigned char buf[1];
} *gcry_md_hd_t;

struct gcry_sexp;
typedef struct gcry_sexp *gcry_sexp_t;

/* The possible values for the S-expression format. */
enum gcry_sexp_format
  {
    GCRYSEXP_FMT_DEFAULT   = 0,
    GCRYSEXP_FMT_CANON     = 1,
    GCRYSEXP_FMT_BASE64    = 2,
    GCRYSEXP_FMT_ADVANCED  = 3
  };

/* Wrappers for the libgpg-error library.  */

typedef gpg_error_t gcry_error_t;
typedef gpg_err_code_t gcry_err_code_t;
typedef gpg_err_source_t gcry_err_source_t;

/****************
* From dh.h *
*****************/

#define DH1536_GROUP_ID 5

typedef struct {
    unsigned int groupid;
    gcry_mpi_t priv, pub;
} DH_keypair;

/* Which half of the secure session id should be shown in bold? */
typedef enum {
    OTRL_SESSIONID_FIRST_HALF_BOLD,
    OTRL_SESSIONID_SECOND_HALF_BOLD
} OtrlSessionIdHalf;

typedef struct {
    unsigned char sendctr[16];
    unsigned char rcvctr[16];
    gcry_cipher_hd_t sendenc;
    gcry_cipher_hd_t rcvenc;
    gcry_md_hd_t sendmac;
    unsigned char sendmackey[20];
    int sendmacused;
    gcry_md_hd_t rcvmac;
    unsigned char rcvmackey[20];
    int rcvmacused;
} DH_sesskeys;

/****************
* From proto.h *
*****************/

typedef unsigned int OtrlPolicy;

#define OTRL_POLICY_ALLOW_V1			0x01
#define OTRL_POLICY_ALLOW_V2			0x02
#define OTRL_POLICY_REQUIRE_ENCRYPTION		0x04
#define OTRL_POLICY_SEND_WHITESPACE_TAG		0x08
#define OTRL_POLICY_WHITESPACE_START_AKE	0x10
#define OTRL_POLICY_ERROR_START_AKE		0x20

#define OTRL_POLICY_VERSION_MASK (OTRL_POLICY_ALLOW_V1 | OTRL_POLICY_ALLOW_V2)

typedef enum {
    OTRL_MSGTYPE_NOTOTR,
    OTRL_MSGTYPE_TAGGEDPLAINTEXT,
    OTRL_MSGTYPE_QUERY,
    OTRL_MSGTYPE_DH_COMMIT,
    OTRL_MSGTYPE_DH_KEY,
    OTRL_MSGTYPE_REVEALSIG,
    OTRL_MSGTYPE_SIGNATURE,
    OTRL_MSGTYPE_V1_KEYEXCH,
    OTRL_MSGTYPE_DATA,
    OTRL_MSGTYPE_ERROR,
    OTRL_MSGTYPE_UNKNOWN
} OtrlMessageType;

typedef enum {
    OTRL_FRAGMENT_UNFRAGMENTED,
    OTRL_FRAGMENT_INCOMPLETE,
    OTRL_FRAGMENT_COMPLETE
} OtrlFragmentResult;

typedef enum {
    OTRL_FRAGMENT_SEND_ALL,
    OTRL_FRAGMENT_SEND_ALL_BUT_FIRST,
    OTRL_FRAGMENT_SEND_ALL_BUT_LAST
} OtrlFragmentPolicy;

/* Initialize the OTR library.  Pass the version of the API you are
 * using. */

extern "C" void __cdecl otrl_init(unsigned int ver_major, unsigned int ver_minor,
	unsigned int ver_sub);

extern "C" char* otrl_proto_default_query_msg(const char *ourname, OtrlPolicy policy);

/* Shortcut */
#define OTRL_INIT do { \
	otrl_init(OTRL_VERSION_MAJOR, OTRL_VERSION_MINOR, OTRL_VERSION_SUB); \
    } while(0)

/****************
* From auth.h *
*****************/

typedef enum {
    OTRL_AUTHSTATE_NONE,
    OTRL_AUTHSTATE_AWAITING_DHKEY,
    OTRL_AUTHSTATE_AWAITING_REVEALSIG,
    OTRL_AUTHSTATE_AWAITING_SIG,
    OTRL_AUTHSTATE_V1_SETUP
} OtrlAuthState;

typedef struct {
    OtrlAuthState authstate;              /* Our state */

    DH_keypair our_dh;                    /* Our D-H key */
    unsigned int our_keyid;               /* ...and its keyid */

    unsigned char *encgx;                 /* The encrypted value of g^x */
    size_t encgx_len;                     /*  ...and its length */
    unsigned char r[16];                  /* The encryption key */

    unsigned char hashgx[32];             /* SHA256(g^x) */

    gcry_mpi_t their_pub;                 /* Their D-H public key */
    unsigned int their_keyid;             /*  ...and its keyid */

    gcry_cipher_hd_t enc_c, enc_cp;       /* c and c' encryption keys */
    gcry_md_hd_t mac_m1, mac_m1p;         /* m1 and m1' MAC keys */
    gcry_md_hd_t mac_m2, mac_m2p;         /* m2 and m2' MAC keys */

    unsigned char their_fingerprint[20];  /* The fingerprint of their
					     long-term signing key */

    int initiated;                        /* Did we initiate this
					     authentication? */

    unsigned int protocol_version;        /* The protocol version number
					     used to authenticate. */

    unsigned char secure_session_id[20];  /* The secure session id */
    size_t secure_session_id_len;         /* And its actual length,
					     which may be either 20 (for
					     v1) or 8 (for v2) */
    OtrlSessionIdHalf session_id_half;    /* Which half of the session
					     id gets shown in bold */

    char *lastauthmsg;                    /* The last auth message
					     (base-64 encoded) we sent,
					     in case we need to
					     retransmit it. */
} OtrlAuthInfo;

/****************
* From sm.h *
*****************/

#define SM_HASH_ALGORITHM GCRY_MD_SHA256
#define SM_DIGEST_SIZE 32

typedef enum {
    OTRL_SMP_EXPECT1,
    OTRL_SMP_EXPECT2,
    OTRL_SMP_EXPECT3,
    OTRL_SMP_EXPECT4,
    OTRL_SMP_EXPECT5
} NextExpectedSMP;

typedef enum {
    OTRL_SMP_PROG_OK = 0,            /* All is going fine so far */
    OTRL_SMP_PROG_CHEATED = -2,      /* Some verification failed */
    OTRL_SMP_PROG_FAILED = -1,       /* The secrets didn't match */
    OTRL_SMP_PROG_SUCCEEDED = 1      /* The SMP completed successfully */
} OtrlSMProgState;

typedef struct {
    gcry_mpi_t secret, x2, x3, g1, g2, g3, g3o, p, q, pab, qab;
    NextExpectedSMP nextExpected;
    int received_question;  /* 1 if we received a question in an SMP1Q TLV */
    OtrlSMProgState sm_prog_state;
} OtrlSMState;

typedef OtrlSMState OtrlSMAliceState;
typedef OtrlSMState OtrlSMBobState;

/****************
* From context.h *
*****************/

typedef enum {
    OTRL_MSGSTATE_PLAINTEXT,           /* Not yet started an encrypted
					  conversation */
    OTRL_MSGSTATE_ENCRYPTED,           /* Currently in an encrypted
					  conversation */
    OTRL_MSGSTATE_FINISHED             /* The remote side has sent us a
					  notification that he has ended
					  his end of the encrypted
					  conversation; prevent any
					  further messages from being
					  sent to him. */
} OtrlMessageState;

typedef struct s_fingerprint {
    struct s_fingerprint *next;        /* The next fingerprint in the list */
    struct s_fingerprint **tous;       /* A pointer to the pointer to us */
    unsigned char *fingerprint;        /* The fingerprint, or NULL */
    struct context *context;           /* The context to which we belong */
    char *trust;                       /* The trust level of the fingerprint */
} Fingerprint;

typedef struct context {
    struct context * next;             /* Linked list pointer */
    struct context ** tous;            /* A pointer to the pointer to us */

    char * username;                   /* The user this context is for */
    char * accountname;                /* The username is relative to
					  this account... */
    char * protocol;                   /* ... and this protocol */

    char *fragment;                    /* The part of the fragmented message
					  we've seen so far */
    size_t fragment_len;               /* The length of fragment */
    unsigned short fragment_n;         /* The total number of fragments
					  in this message */
    unsigned short fragment_k;         /* The highest fragment number
					  we've seen so far for this
					  message */

    OtrlMessageState msgstate;         /* The state of message disposition
					  with this user */
    OtrlAuthInfo auth;                 /* The state of ongoing
					  authentication with this user */

    Fingerprint fingerprint_root;      /* The root of a linked list of
					  Fingerprints entries */
    Fingerprint *active_fingerprint;   /* Which fingerprint is in use now?
                                          A pointer into the above list */
    unsigned int their_keyid;          /* current keyid used by other side;
                                          this is set to 0 if we get a
					  OTRL_TLV_DISCONNECTED message from
					  them. */
    gcry_mpi_t their_y;                /* Y[their_keyid] (their DH pubkey) */
    gcry_mpi_t their_old_y;            /* Y[their_keyid-1] (their prev DH
					  pubkey) */
    unsigned int our_keyid;            /* current keyid used by us */
    DH_keypair our_dh_key;             /* DH key[our_keyid] */
    DH_keypair our_old_dh_key;         /* DH key[our_keyid-1] */

    DH_sesskeys sesskeys[2][2];        /* sesskeys[i][j] are the session keys
					  derived from DH key[our_keyid-i]
					  and mpi Y[their_keyid-j] */

    unsigned char sessionid[20];       /* The sessionid and bold half */
    size_t sessionid_len;              /* determined when this private */
    OtrlSessionIdHalf sessionid_half;  /* connection was established. */

    unsigned int protocol_version;     /* The version of OTR in use */

    unsigned char *preshared_secret;   /* A secret you share with this
					  user, in order to do
					  authentication. */
    size_t preshared_secret_len;       /* The length of the above secret. */

    /* saved mac keys to be revealed later */
    unsigned int numsavedkeys;
    unsigned char *saved_mac_keys;

    /* generation number: increment every time we go private, and never
     * reset to 0 (unless we remove the context entirely) */
    unsigned int generation;

    time_t lastsent;      /* The last time a Data Message was sent */
    char *lastmessage;    /* The plaintext of the last Data Message sent */
    int may_retransmit;   /* Is the last message eligible for
			     retransmission? */

    enum {
	OFFER_NOT,
	OFFER_SENT,
	OFFER_REJECTED,
	OFFER_ACCEPTED
    } otr_offer;          /* Has this correspondent repsponded to our
			     OTR offers? */

    /* Application data to be associated with this context */
    void *app_data;
    /* A function to free the above data when we forget this context */
    void (*app_data_free)(void *);

    OtrlSMState *smstate;              /* The state of the current
                                          socialist millionaires exchange */
} ConnContext;

/****************
* From privkey-t.h *
*****************/

typedef struct s_OtrlPrivKey {
    struct s_OtrlPrivKey *next;
    struct s_OtrlPrivKey **tous;

    char *accountname;
    char *protocol;
    unsigned short pubkey_type;
    gcry_sexp_t privkey;
    unsigned char *pubkey_data;
    size_t pubkey_datalen;
} OtrlPrivKey;

#define OTRL_PUBKEY_TYPE_DSA 0x0000

/****************
* From userstate.h *
*****************/

typedef struct s_OtrlUserState* OtrlUserState;

struct s_OtrlUserState {
    ConnContext *context_root;
    OtrlPrivKey *privkey_root;
};

/* Create a new OtrlUserState.  Most clients will only need one of
 * these.  A OtrlUserState encapsulates the list of known fingerprints
 * and the list of private keys; if you have separate files for these
 * things for (say) different users, use different OtrlUserStates.  If
 * you've got only one user, with multiple accounts all stored together
 * in the same fingerprint store and privkey store files, use just one
 * OtrlUserState. */
extern "C" OtrlUserState __cdecl otrl_userstate_create(void);

/* Free a OtrlUserState */
extern "C" void __cdecl otrl_userstate_free(OtrlUserState us);

/****************
* From context.h *
*****************/

/* Look up a connection context by name/account/protocol from the given
 * OtrlUserState.  If add_if_missing is true, allocate and return a new
 * context if one does not currently exist.  In that event, call
 * add_app_data(data, context) so that app_data and app_data_free can be
 * filled in by the application, and set *addedp to 1. */
extern "C" ConnContext * otrl_context_find(OtrlUserState us, const char *user,
	const char *accountname, const char *protocol, int add_if_missing,
	int *addedp,
	void (*add_app_data)(void *data, ConnContext *context), void *data);

/* Forget a whole context, so long as it's PLAINTEXT. */
extern "C" void otrl_context_forget(ConnContext *context);

/* Force a context into the OTRL_MSGSTATE_PLAINTEXT state. */
extern "C" void otrl_context_force_plaintext(ConnContext *context);

/****************
* From privkey.h *
*****************/

/* Convert a 20-byte hash value to a 45-byte human-readable value */
extern "C" void otrl_privkey_hash_to_human(char human[45], const unsigned char hash[20]);

/* Calculate a human-readable hash of our DSA public key.  Return it in
 * the passed fingerprint buffer.  Return NULL on error, or a pointer to
 * the given buffer on success. */
extern "C" char *otrl_privkey_fingerprint(OtrlUserState us, char fingerprint[45],
	const char *accountname, const char *protocol);

/* Calculate a raw hash of our DSA public key.  Return it in the passed
 * fingerprint buffer.  Return NULL on error, or a pointer to the given
 * buffer on success. */
extern "C" unsigned char *otrl_privkey_fingerprint_raw(OtrlUserState us,
	unsigned char hash[20], const char *accountname, const char *protocol);

/* Read a sets of private DSA keys from a file on disk into the given
 * OtrlUserState. */
extern "C" gcry_error_t otrl_privkey_read(OtrlUserState us, const char *filename);

/* Read a sets of private DSA keys from a FILE* into the given
 * OtrlUserState.  The FILE* must be open for reading. */
extern "C" gcry_error_t otrl_privkey_read_FILEp(OtrlUserState us, FILE *privf);

/* Generate a private DSA key for a given account, storing it into a
 * file on disk, and loading it into the given OtrlUserState.  Overwrite any
 * previously generated keys for that account in that OtrlUserState. */
extern "C" gcry_error_t otrl_privkey_generate(OtrlUserState us, const char *filename,
	const char *accountname, const char *protocol);

/* Generate a private DSA key for a given account, storing it into a
 * FILE*, and loading it into the given OtrlUserState.  Overwrite any
 * previously generated keys for that account in that OtrlUserState.
 * The FILE* must be open for reading and writing. */
extern "C" gcry_error_t otrl_privkey_generate_FILEp(OtrlUserState us, FILE *privf,
	const char *accountname, const char *protocol);

/* Read the fingerprint store from a file on disk into the given
 * OtrlUserState.  Use add_app_data to add application data to each
 * ConnContext so created. */
extern "C" gcry_error_t otrl_privkey_read_fingerprints(OtrlUserState us,
	const char *filename,
	void (*add_app_data)(void *data, ConnContext *context),
	void  *data);

/* Read the fingerprint store from a FILE* into the given
 * OtrlUserState.  Use add_app_data to add application data to each
 * ConnContext so created.  The FILE* must be open for reading. */
extern "C" gcry_error_t otrl_privkey_read_fingerprints_FILEp(OtrlUserState us,
	FILE *storef,
	void (*add_app_data)(void *data, ConnContext *context),
	void  *data);

/* Write the fingerprint store from a given OtrlUserState to a file on disk. */
extern "C" gcry_error_t otrl_privkey_write_fingerprints(OtrlUserState us,
	const char *filename);

/* Write the fingerprint store from a given OtrlUserState to a FILE*.
 * The FILE* must be open for writing. */
extern "C" gcry_error_t otrl_privkey_write_fingerprints_FILEp(OtrlUserState us,
	FILE *storef);

/* Fetch the private key from the given OtrlUserState associated with
 * the given account */
extern "C" OtrlPrivKey *otrl_privkey_find(OtrlUserState us, const char *accountname,
	const char *protocol);

/* Forget a private key */
extern "C" void otrl_privkey_forget(OtrlPrivKey *privkey);

/* Forget all private keys in a given OtrlUserState. */
extern "C" void otrl_privkey_forget_all(OtrlUserState us);

/* Sign data using a private key.  The data must be small enough to be
 * signed (i.e. already hashed, if necessary).  The signature will be
 * returned in *sigp, which the caller must free().  Its length will be
 * returned in *siglenp. */
extern "C" gcry_error_t otrl_privkey_sign(unsigned char **sigp, size_t *siglenp,
	OtrlPrivKey *privkey, const unsigned char *data, size_t len);

/* Verify a signature on data using a public key.  The data must be
 * small enough to be signed (i.e. already hashed, if necessary). */
extern "C" gcry_error_t otrl_privkey_verify(const unsigned char *sigbuf, size_t siglen,
	unsigned short pubkey_type, gcry_sexp_t pubs,
	const unsigned char *data, size_t len);

/****************
* From tlv.h    *
*****************/

typedef struct s_OtrlTLV {
    unsigned short type;
    unsigned short len;
    unsigned char *data;
    struct s_OtrlTLV *next;
} OtrlTLV;

/****************
* From message.h *
*****************/

typedef enum {
    OTRL_NOTIFY_ERROR,
    OTRL_NOTIFY_WARNING,
    OTRL_NOTIFY_INFO
} OtrlNotifyLevel;

typedef struct s_OtrlMessageAppOps {
    /* Return the OTR policy for the given context. */
    OtrlPolicy (*policy)(void *opdata, ConnContext *context);

    /* Create a private key for the given accountname/protocol if
     * desired. */
    void (*create_privkey)(void *opdata, const char *accountname,
	    const char *protocol);

    /* Report whether you think the given user is online.  Return 1 if
     * you think he is, 0 if you think he isn't, -1 if you're not sure.
     *
     * If you return 1, messages such as heartbeats or other
     * notifications may be sent to the user, which could result in "not
     * logged in" errors if you're wrong. */
    int (*is_logged_in)(void *opdata, const char *accountname,
	    const char *protocol, const char *recipient);

    /* Send the given IM to the given recipient from the given
     * accountname/protocol. */
    void (*inject_message)(void *opdata, const char *accountname,
	    const char *protocol, const char *recipient, const char *message);

    /* Display a notification message for a particular accountname /
     * protocol / username conversation. */
    void (*notify)(void *opdata, OtrlNotifyLevel level,
	    const char *accountname, const char *protocol,
	    const char *username, const char *title,
	    const char *primary, const char *secondary);

    /* Display an OTR control message for a particular accountname /
     * protocol / username conversation.  Return 0 if you are able to
     * successfully display it.  If you return non-0 (or if this
     * function is NULL), the control message will be displayed inline,
     * as a received message, or else by using the above notify()
     * callback. */
    int (*display_otr_message)(void *opdata, const char *accountname,
	    const char *protocol, const char *username, const char *msg);

    /* When the list of ConnContexts changes (including a change in
     * state), this is called so the UI can be updated. */
    void (*update_context_list)(void *opdata);

    /* Return a newly allocated string containing a human-friendly name
     * for the given protocol id */
    const char *(*protocol_name)(void *opdata, const char *protocol);

    /* Deallocate a string allocated by protocol_name */
    void (*protocol_name_free)(void *opdata, const char *protocol_name);

    /* A new fingerprint for the given user has been received. */
    void (*new_fingerprint)(void *opdata, OtrlUserState us,
	    const char *accountname, const char *protocol,
	    const char *username, unsigned char fingerprint[20]);

    /* The list of known fingerprints has changed.  Write them to disk. */
    void (*write_fingerprints)(void *opdata);

    /* A ConnContext has entered a secure state. */
    void (*gone_secure)(void *opdata, ConnContext *context);

    /* A ConnContext has left a secure state. */
    void (*gone_insecure)(void *opdata, ConnContext *context);

    /* We have completed an authentication, using the D-H keys we
     * already knew.  is_reply indicates whether we initiated the AKE. */
    void (*still_secure)(void *opdata, ConnContext *context, int is_reply);

    /* Log a message.  The passed message will end in "\n". */
    void (*log_message)(void *opdata, const char *message);

    /* Find the maximum message size supported by this protocol. */
    int (*max_message_size)(void *opdata, ConnContext *context);

    /* Return a newly allocated string containing a human-friendly
     * representation for the given account */
    const char *(*account_name)(void *opdata, const char *account,
	    const char *protocol);

    /* Deallocate a string returned by account_name */
    void (*account_name_free)(void *opdata, const char *account_name);

} OtrlMessageAppOps;

/* Deallocate a message allocated by other otrl_message_* routines. */
extern "C" void otrl_message_free(char *message);

/* Handle a message about to be sent to the network.  It is safe to pass
 * all messages about to be sent to this routine.  add_appdata is a
 * function that will be called in the event that a new ConnContext is
 * created.  It will be passed the data that you supplied, as well as a
 * pointer to the new ConnContext.  You can use this to add
 * application-specific information to the ConnContext using the
 * "context->app" field, for example.  If you don't need to do this, you
 * can pass NULL for the last two arguments of otrl_message_sending.  
 *
 * tlvs is a chain of OtrlTLVs to append to the private message.  It is
 * usually correct to just pass NULL here.
 *
 * If this routine returns non-zero, then the library tried to encrypt
 * the message, but for some reason failed.  DO NOT send the message in
 * the clear in that case.
 * 
 * If *messagep gets set by the call to something non-NULL, then you
 * should replace your message with the contents of *messagep, and
 * send that instead.  Call otrl_message_free(*messagep) when you're
 * done with it. */
extern "C" gcry_error_t otrl_message_sending(OtrlUserState us,
	const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *recipient, const char *message, OtrlTLV *tlvs,
	char **messagep,
	void (*add_appdata)(void *data, ConnContext *context),
	void *data);

/* Handle a message just received from the network.  It is safe to pass
 * all received messages to this routine.  add_appdata is a function
 * that will be called in the event that a new ConnContext is created.
 * It will be passed the data that you supplied, as well as
 * a pointer to the new ConnContext.  You can use this to add
 * application-specific information to the ConnContext using the
 * "context->app" field, for example.  If you don't need to do this, you
 * can pass NULL for the last two arguments of otrl_message_receiving.  
 *
 * If otrl_message_receiving returns 1, then the message you received
 * was an internal protocol message, and no message should be delivered
 * to the user.
 *
 * If it returns 0, then check if *messagep was set to non-NULL.  If
 * so, replace the received message with the contents of *messagep, and
 * deliver that to the user instead.  You must call
 * otrl_message_free(*messagep) when you're done with it.  If tlvsp is
 * non-NULL, *tlvsp will be set to a chain of any TLVs that were
 * transmitted along with this message.  You must call
 * otrl_tlv_free(*tlvsp) when you're done with those.
 *
 * If otrl_message_receiving returns 0 and *messagep is NULL, then this
 * was an ordinary, non-OTR message, which should just be delivered to
 * the user without modification. */
extern "C" int otrl_message_receiving(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *sender, const char *message, char **newmessagep,
	OtrlTLV **tlvsp,
	void (*add_appdata)(void *data, ConnContext *context),
	void *data);

/* Send a message to the network, fragmenting first if necessary.
 * All messages to be sent to the network should go through this
 * method immediately before they are sent, ie after encryption. */
extern "C" gcry_error_t otrl_message_fragment_and_send(const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const char *message,
	OtrlFragmentPolicy fragPolicy, char **returnFragment);

/* Put a connection into the PLAINTEXT state, first sending the
 * other side a notice that we're doing so if we're currently ENCRYPTED,
 * and we think he's logged in. */
extern "C" void otrl_message_disconnect(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, const char *accountname, const char *protocol,
	const char *username);

/* Initiate the Socialist Millionaires' Protocol */
extern "C" void otrl_message_initiate_smp(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const unsigned char *secret,
	size_t secretlen);

/* Initiate the Socialist Millionaires' Protocol and send a prompt
 * question to the buddy */
extern "C" void otrl_message_initiate_smp_q(OtrlUserState us,
	const OtrlMessageAppOps *ops, void *opdata, ConnContext *context,
	const char *question, const unsigned char *secret, size_t secretlen);

/* Respond to a buddy initiating the Socialist Millionaires' Protocol */
extern "C" void otrl_message_respond_smp(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context, const unsigned char *secret,
	size_t secretlen);

/* Abort the SMP.  Called when an unexpected SMP message breaks the
 * normal flow. */
extern "C" void otrl_message_abort_smp(OtrlUserState us, const OtrlMessageAppOps *ops,
	void *opdata, ConnContext *context);

