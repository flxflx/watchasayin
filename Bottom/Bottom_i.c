

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IGeneric,0x9D98616B,0x7348,0x4A98,0xB7,0x85,0x1B,0x4D,0x52,0x2F,0xFF,0xF1);


MIDL_DEFINE_GUID(IID, IID_IHookingEngine,0xCBE2E8B4,0x2575,0x4D16,0xA8,0x00,0x64,0xE6,0xAD,0x91,0x29,0x5D);


MIDL_DEFINE_GUID(IID, LIBID_BottomLib,0xF3B2DB50,0x48FD,0x45FC,0x93,0xC0,0x69,0xC0,0xF7,0x85,0xD0,0xD5);


MIDL_DEFINE_GUID(CLSID, CLSID_Generic,0xDEE1561E,0x5FDA,0x49CF,0x9B,0xD0,0x48,0xA8,0xD6,0x31,0x43,0x69);


MIDL_DEFINE_GUID(CLSID, CLSID_HookingEngine,0x19F24CE7,0x3551,0x4CDC,0xA2,0xA0,0xB4,0xDA,0x4B,0x0F,0x39,0x6E);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



