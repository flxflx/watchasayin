

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sun Jul 01 22:07:47 2012
 */
/* Compiler settings for Bottom.idl:
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

#ifndef __Bottom_i_h__
#define __Bottom_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGeneric_FWD_DEFINED__
#define __IGeneric_FWD_DEFINED__
typedef interface IGeneric IGeneric;
#endif 	/* __IGeneric_FWD_DEFINED__ */


#ifndef __IHookingEngine_FWD_DEFINED__
#define __IHookingEngine_FWD_DEFINED__
typedef interface IHookingEngine IHookingEngine;
#endif 	/* __IHookingEngine_FWD_DEFINED__ */


#ifndef __Generic_FWD_DEFINED__
#define __Generic_FWD_DEFINED__

#ifdef __cplusplus
typedef class Generic Generic;
#else
typedef struct Generic Generic;
#endif /* __cplusplus */

#endif 	/* __Generic_FWD_DEFINED__ */


#ifndef __HookingEngine_FWD_DEFINED__
#define __HookingEngine_FWD_DEFINED__

#ifdef __cplusplus
typedef class HookingEngine HookingEngine;
#else
typedef struct HookingEngine HookingEngine;
#endif /* __cplusplus */

#endif 	/* __HookingEngine_FWD_DEFINED__ */


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
    
    MIDL_INTERFACE("9D98616B-7348-4A98-B785-1B4D522FFFF1")
    IGeneric : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getPid( 
            /* [retval][out] */ int *pid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getLog( 
            /* [retval][out] */ SAFEARRAY * *lines) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE unload( 
            /* [in] */ BSTR ownName) = 0;
        
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
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getPid )( 
            IGeneric * This,
            /* [retval][out] */ int *pid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *getLog )( 
            IGeneric * This,
            /* [retval][out] */ SAFEARRAY * *lines);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *unload )( 
            IGeneric * This,
            /* [in] */ BSTR ownName);
        
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


#define IGeneric_getPid(This,pid)	\
    ( (This)->lpVtbl -> getPid(This,pid) ) 

#define IGeneric_getLog(This,lines)	\
    ( (This)->lpVtbl -> getLog(This,lines) ) 

#define IGeneric_unload(This,ownName)	\
    ( (This)->lpVtbl -> unload(This,ownName) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGeneric_INTERFACE_DEFINED__ */


#ifndef __IHookingEngine_INTERFACE_DEFINED__
#define __IHookingEngine_INTERFACE_DEFINED__

/* interface IHookingEngine */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IHookingEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CBE2E8B4-2575-4D16-A800-64E6AD91295D")
    IHookingEngine : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE hookNetworkIO( 
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE unhookNetworkIO( 
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getOpenConnectionsOutgoing( 
            /* [retval][out] */ SAFEARRAY * *connections) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getOpenConnectionsIncoming( 
            /* [retval][out] */ SAFEARRAY * *connections) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE resetConnections( 
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE addConnectionOutgoing( 
            /* [in] */ unsigned int sessionId,
            /* [in] */ unsigned int ip,
            /* [in] */ unsigned short sourcePort,
            /* [in] */ unsigned short destinationPort,
            /* [in] */ unsigned int protocol,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE addConnectionIncoming( 
            /* [in] */ unsigned int sessionId,
            /* [in] */ unsigned int ip,
            /* [in] */ unsigned short sourcePort,
            /* [in] */ unsigned short destinationPort,
            /* [in] */ unsigned int protocol,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE createSession( 
            /* [in] */ BOOL expandable,
            /* [retval][out] */ unsigned int *sessionId) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE startSession( 
            /* [in] */ unsigned int sessionId,
            /* [in] */ SAFEARRAY * key,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE destroySession( 
            /* [in] */ unsigned int sessionId,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE init( 
            /* [in] */ BSTR baseDir,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE setSecurityPolicy( 
            int securityPolicy,
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE startCountingConnections( 
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE stopCountingConnections( 
            /* [retval][out] */ BOOL *success) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getMostUsedPeerIP( 
            /* [retval][out] */ unsigned int *ip) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHookingEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHookingEngine * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHookingEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHookingEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IHookingEngine * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IHookingEngine * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IHookingEngine * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IHookingEngine * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *hookNetworkIO )( 
            IHookingEngine * This,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *unhookNetworkIO )( 
            IHookingEngine * This,
            /* [retval][out] */ BOOL *success);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *getOpenConnectionsOutgoing )( 
            IHookingEngine * This,
            /* [retval][out] */ SAFEARRAY * *connections);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *getOpenConnectionsIncoming )( 
            IHookingEngine * This,
            /* [retval][out] */ SAFEARRAY * *connections);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *resetConnections )( 
            IHookingEngine * This,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *addConnectionOutgoing )( 
            IHookingEngine * This,
            /* [in] */ unsigned int sessionId,
            /* [in] */ unsigned int ip,
            /* [in] */ unsigned short sourcePort,
            /* [in] */ unsigned short destinationPort,
            /* [in] */ unsigned int protocol,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *addConnectionIncoming )( 
            IHookingEngine * This,
            /* [in] */ unsigned int sessionId,
            /* [in] */ unsigned int ip,
            /* [in] */ unsigned short sourcePort,
            /* [in] */ unsigned short destinationPort,
            /* [in] */ unsigned int protocol,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *createSession )( 
            IHookingEngine * This,
            /* [in] */ BOOL expandable,
            /* [retval][out] */ unsigned int *sessionId);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *startSession )( 
            IHookingEngine * This,
            /* [in] */ unsigned int sessionId,
            /* [in] */ SAFEARRAY * key,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *destroySession )( 
            IHookingEngine * This,
            /* [in] */ unsigned int sessionId,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *init )( 
            IHookingEngine * This,
            /* [in] */ BSTR baseDir,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *setSecurityPolicy )( 
            IHookingEngine * This,
            int securityPolicy,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *startCountingConnections )( 
            IHookingEngine * This,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *stopCountingConnections )( 
            IHookingEngine * This,
            /* [retval][out] */ BOOL *success);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *getMostUsedPeerIP )( 
            IHookingEngine * This,
            /* [retval][out] */ unsigned int *ip);
        
        END_INTERFACE
    } IHookingEngineVtbl;

    interface IHookingEngine
    {
        CONST_VTBL struct IHookingEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHookingEngine_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IHookingEngine_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IHookingEngine_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IHookingEngine_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IHookingEngine_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IHookingEngine_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IHookingEngine_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IHookingEngine_hookNetworkIO(This,success)	\
    ( (This)->lpVtbl -> hookNetworkIO(This,success) ) 

#define IHookingEngine_unhookNetworkIO(This,success)	\
    ( (This)->lpVtbl -> unhookNetworkIO(This,success) ) 

#define IHookingEngine_getOpenConnectionsOutgoing(This,connections)	\
    ( (This)->lpVtbl -> getOpenConnectionsOutgoing(This,connections) ) 

#define IHookingEngine_getOpenConnectionsIncoming(This,connections)	\
    ( (This)->lpVtbl -> getOpenConnectionsIncoming(This,connections) ) 

#define IHookingEngine_resetConnections(This,success)	\
    ( (This)->lpVtbl -> resetConnections(This,success) ) 

#define IHookingEngine_addConnectionOutgoing(This,sessionId,ip,sourcePort,destinationPort,protocol,success)	\
    ( (This)->lpVtbl -> addConnectionOutgoing(This,sessionId,ip,sourcePort,destinationPort,protocol,success) ) 

#define IHookingEngine_addConnectionIncoming(This,sessionId,ip,sourcePort,destinationPort,protocol,success)	\
    ( (This)->lpVtbl -> addConnectionIncoming(This,sessionId,ip,sourcePort,destinationPort,protocol,success) ) 

#define IHookingEngine_createSession(This,expandable,sessionId)	\
    ( (This)->lpVtbl -> createSession(This,expandable,sessionId) ) 

#define IHookingEngine_startSession(This,sessionId,key,success)	\
    ( (This)->lpVtbl -> startSession(This,sessionId,key,success) ) 

#define IHookingEngine_destroySession(This,sessionId,success)	\
    ( (This)->lpVtbl -> destroySession(This,sessionId,success) ) 

#define IHookingEngine_init(This,baseDir,success)	\
    ( (This)->lpVtbl -> init(This,baseDir,success) ) 

#define IHookingEngine_setSecurityPolicy(This,securityPolicy,success)	\
    ( (This)->lpVtbl -> setSecurityPolicy(This,securityPolicy,success) ) 

#define IHookingEngine_startCountingConnections(This,success)	\
    ( (This)->lpVtbl -> startCountingConnections(This,success) ) 

#define IHookingEngine_stopCountingConnections(This,success)	\
    ( (This)->lpVtbl -> stopCountingConnections(This,success) ) 

#define IHookingEngine_getMostUsedPeerIP(This,ip)	\
    ( (This)->lpVtbl -> getMostUsedPeerIP(This,ip) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IHookingEngine_INTERFACE_DEFINED__ */



#ifndef __BottomLib_LIBRARY_DEFINED__
#define __BottomLib_LIBRARY_DEFINED__

/* library BottomLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_BottomLib;

EXTERN_C const CLSID CLSID_Generic;

#ifdef __cplusplus

class DECLSPEC_UUID("DEE1561E-5FDA-49CF-9BD0-48A8D6314369")
Generic;
#endif

EXTERN_C const CLSID CLSID_HookingEngine;

#ifdef __cplusplus

class DECLSPEC_UUID("19F24CE7-3551-4CDC-A2A0-B4DA4B0F396E")
HookingEngine;
#endif
#endif /* __BottomLib_LIBRARY_DEFINED__ */

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


