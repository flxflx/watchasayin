

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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

MIDL_DEFINE_GUID(IID, IID_IGeneric,0x92FE0A85,0x9012,0x4699,0xB5,0x8F,0xD3,0x4B,0xD2,0xFA,0x51,0x15);


MIDL_DEFINE_GUID(IID, IID_ICrypto,0x5F1202DB,0x2F2C,0x42B3,0x86,0xB9,0x3D,0x09,0x2E,0x0B,0x5E,0x4A);


MIDL_DEFINE_GUID(IID, LIBID_KryptoniteLib,0x89D3E345,0x93D5,0x4797,0x8B,0xB2,0xB9,0xC3,0x2B,0x5B,0x42,0x55);


MIDL_DEFINE_GUID(CLSID, CLSID_Generic,0x82612A97,0x7D8A,0x42CF,0xB9,0xC4,0xC7,0x75,0x8B,0xAF,0xE0,0x48);


MIDL_DEFINE_GUID(CLSID, CLSID_Crypto,0xCD109C50,0x2C13,0x4121,0x83,0x3F,0x83,0x07,0xBB,0x46,0xF6,0x65);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



