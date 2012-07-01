

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sun Jul 01 22:07:47 2012
 */
/* Compiler settings for Kryptonite.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __Kryptonite_i_h__
#define __Kryptonite_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGeneric_FWD_DEFINED__
#define __IGeneric_FWD_DEFINED__
typedef interface IGeneric IGeneric;
#endif 	/* __IGeneric_FWD_DEFINED__ */


#ifndef __ICrypto_FWD_DEFINED__
#define __ICrypto_FWD_DEFINED__
typedef interface ICrypto ICrypto;
#endif 	/* __ICrypto_FWD_DEFINED__ */


#ifndef __Generic_FWD_DEFINED__
#define __Generic_FWD_DEFINED__

#ifdef __cplusplus
typedef class Generic Generic;
#else
typedef struct Generic Generic;
#endif /* __cplusplus */

#endif 	/* __Generic_FWD_DEFINED__ */


#ifndef __Crypto_FWD_DEFINED__
#define __Crypto_FWD_DEFINED__

#ifdef __cplusplus
typedef class Crypto Crypto;
#else
typedef struct Crypto Crypto;
#endif /* __cplusplus */

#endif 	/* __Crypto_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IGeneric_INTERFACE_DEFINED__
#define __IGeneric_INTERFACE_DEFINED__

/* interface IGeneric */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IGeneric;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("92FE0A85-9012-4699-B58F-D34BD2FA5115")
    IGeneric : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getLog( 
            /* [retval][out] */ SAFEARRAY * *lines) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGenericVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGeneric * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGeneric * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGeneric * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGeneric * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGeneric * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGeneric * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGeneric * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getLog )( 
            IGeneric * This,
            /* [retval][out] */ SAFEARRAY * *lines);
        
        END_INTERFACE
    } IGenericVtbl;

    interface IGeneric
    {
        CONST_VTBL struct IGenericVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGeneric_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGeneric_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGeneric_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGeneric_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGeneric_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGeneric_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGeneric_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IGeneric_getLog(This,lines)	\
    ( (This)->lpVtbl -> getLog(This,lines) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGeneric_INTERFACE_DEFINED__ */


#ifndef __ICrypto_INTERFACE_DEFINED__
#define __ICrypto_INTERFACE_DEFINED__

/* interface ICrypto */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICrypto;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5F1202DB-2F2C-42B3-86B9-3D092E0B5E4A")
    ICrypto : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE init( 
            /* [in] */ BSTR accountName,
            /* [in] */ BSTR baseDir,
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE generatePrivateKey( 
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE messageReceived( 
            /* [in] */ BSTR partner,
            /* [in] */ BSTR message,
            /* [out] */ BSTR *plain,
            /* [out] */ BOOL *deploy,
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE messageSending( 
            /* [in] */ BSTR partner,
            /* [in] */ BSTR message,
            /* [out] */ BSTR *cipher,
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getNextMessage( 
            /* [in] */ BSTR partner,
            /* [out] */ BSTR *message,
            /* [retval][out] */ int *messagesLeft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getNextNotification( 
            /* [in] */ BSTR partner,
            /* [out] */ BSTR *notification,
            /* [retval][out] */ int *notificationsLeft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getNextEvent( 
            /* [in] */ BSTR partner,
            /* [out] */ int *eventType,
            /* [out] */ BSTR *eventData,
            /* [retval][out] */ int *eventsLeft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getNextEventGeneric( 
            /* [out] */ int *eventType,
            /* [out] */ BSTR *eventData,
            /* [retval][out] */ int *eventsLeft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE startSession( 
            /* [in] */ BSTR partner,
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE endSession( 
            /* [in] */ BSTR partner,
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE createFingerprintStore( 
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getOwnFingerprint( 
            /* [out] */ BSTR *fingerprint,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE confirmFingerprint( 
            /* [in] */ BSTR partner,
            /* [in] */ BSTR fingerprint,
            /* [retval][out] */ int *nextAction) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getSessionKey( 
            /* [in] */ BSTR partner,
            /* [out] */ SAFEARRAY * *key,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getFingerprint( 
            /* [in] */ BSTR partner,
            /* [out] */ BSTR *fingerprint,
            /* [retval][out] */ BOOL *success) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICryptoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrypto * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrypto * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrypto * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICrypto * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICrypto * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICrypto * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICrypto * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *init )( 
            ICrypto * This,
            /* [in] */ BSTR accountName,
            /* [in] */ BSTR baseDir,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *generatePrivateKey )( 
            ICrypto * This,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *messageReceived )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [in] */ BSTR message,
            /* [out] */ BSTR *plain,
            /* [out] */ BOOL *deploy,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *messageSending )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [in] */ BSTR message,
            /* [out] */ BSTR *cipher,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getNextMessage )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [out] */ BSTR *message,
            /* [retval][out] */ int *messagesLeft);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getNextNotification )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [out] */ BSTR *notification,
            /* [retval][out] */ int *notificationsLeft);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getNextEvent )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [out] */ int *eventType,
            /* [out] */ BSTR *eventData,
            /* [retval][out] */ int *eventsLeft);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getNextEventGeneric )( 
            ICrypto * This,
            /* [out] */ int *eventType,
            /* [out] */ BSTR *eventData,
            /* [retval][out] */ int *eventsLeft);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *startSession )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *endSession )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *createFingerprintStore )( 
            ICrypto * This,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getOwnFingerprint )( 
            ICrypto * This,
            /* [out] */ BSTR *fingerprint,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *confirmFingerprint )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [in] */ BSTR fingerprint,
            /* [retval][out] */ int *nextAction);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getSessionKey )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [out] */ SAFEARRAY * *key,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getFingerprint )( 
            ICrypto * This,
            /* [in] */ BSTR partner,
            /* [out] */ BSTR *fingerprint,
            /* [retval][out] */ BOOL *success);
        
        END_INTERFACE
    } ICryptoVtbl;

    interface ICrypto
    {
        CONST_VTBL struct ICryptoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrypto_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICrypto_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICrypto_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICrypto_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICrypto_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICrypto_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICrypto_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICrypto_init(This,accountName,baseDir,nextAction)	\
    ( (This)->lpVtbl -> init(This,accountName,baseDir,nextAction) ) 

#define ICrypto_generatePrivateKey(This,nextAction)	\
    ( (This)->lpVtbl -> generatePrivateKey(This,nextAction) ) 

#define ICrypto_messageReceived(This,partner,message,plain,deploy,nextAction)	\
    ( (This)->lpVtbl -> messageReceived(This,partner,message,plain,deploy,nextAction) ) 

#define ICrypto_messageSending(This,partner,message,cipher,nextAction)	\
    ( (This)->lpVtbl -> messageSending(This,partner,message,cipher,nextAction) ) 

#define ICrypto_getNextMessage(This,partner,message,messagesLeft)	\
    ( (This)->lpVtbl -> getNextMessage(This,partner,message,messagesLeft) ) 

#define ICrypto_getNextNotification(This,partner,notification,notificationsLeft)	\
    ( (This)->lpVtbl -> getNextNotification(This,partner,notification,notificationsLeft) ) 

#define ICrypto_getNextEvent(This,partner,eventType,eventData,eventsLeft)	\
    ( (This)->lpVtbl -> getNextEvent(This,partner,eventType,eventData,eventsLeft) ) 

#define ICrypto_getNextEventGeneric(This,eventType,eventData,eventsLeft)	\
    ( (This)->lpVtbl -> getNextEventGeneric(This,eventType,eventData,eventsLeft) ) 

#define ICrypto_startSession(This,partner,nextAction)	\
    ( (This)->lpVtbl -> startSession(This,partner,nextAction) ) 

#define ICrypto_endSession(This,partner,nextAction)	\
    ( (This)->lpVtbl -> endSession(This,partner,nextAction) ) 

#define ICrypto_createFingerprintStore(This,nextAction)	\
    ( (This)->lpVtbl -> createFingerprintStore(This,nextAction) ) 

#define ICrypto_getOwnFingerprint(This,fingerprint,success)	\
    ( (This)->lpVtbl -> getOwnFingerprint(This,fingerprint,success) ) 

#define ICrypto_confirmFingerprint(This,partner,fingerprint,nextAction)	\
    ( (This)->lpVtbl -> confirmFingerprint(This,partner,fingerprint,nextAction) ) 

#define ICrypto_getSessionKey(This,partner,key,success)	\
    ( (This)->lpVtbl -> getSessionKey(This,partner,key,success) ) 

#define ICrypto_getFingerprint(This,partner,fingerprint,success)	\
    ( (This)->lpVtbl -> getFingerprint(This,partner,fingerprint,success) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICrypto_INTERFACE_DEFINED__ */



#ifndef __KryptoniteLib_LIBRARY_DEFINED__
#define __KryptoniteLib_LIBRARY_DEFINED__

/* library KryptoniteLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_KryptoniteLib;

EXTERN_C const CLSID CLSID_Generic;

#ifdef __cplusplus

class DECLSPEC_UUID("82612A97-7D8A-42CF-B9C4-C7758BAFE048")
Generic;
#endif

EXTERN_C const CLSID CLSID_Crypto;

#ifdef __cplusplus

class DECLSPEC_UUID("CD109C50-2C13-4121-833F-8307BB46F665")
Crypto;
#endif
#endif /* __KryptoniteLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


