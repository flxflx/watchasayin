// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

//custom
#define OTR_PROTOCOL_NAME "watchasayin"
#define OTR_POLICY OTRL_POLICY_ALLOW_V2 | OTRL_POLICY_REQUIRE_ENCRYPTION
#define MAX_MSG_SIZE 8192 //afaik Skype can handle even more, but this should be enough

#define BREAKPOINT _asm{int 3};