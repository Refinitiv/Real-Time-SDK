/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

//  description:
//  This header file is meant to define platform specific
//  definitions needed for compiles.
//
//	NOTE : This header file MUST use old style comments since
//	it is used by K&R, ANSI, and C++ compilers.
//
//	GetDTableSize - Used in old ssl ipc mechanism (max fd number).


#ifndef __RIPC_PLATFORM
#define __RIPC_PLATFORM

#include "rtr/platform.h"


/* default thread safe macros */
#define ENTER_C_SESSION_STATE_SYNC()
#define EXIT_C_SESSION_STATE_SYNC()

#define ENTER_S_SESSION_STATE_SYNC()
#define EXIT_S_SESSION_STATE_SYNC()

#define THREAD_SAFE_ASSIGN_32(__ptarget, __value) 	(*__ptarget = __value)
#define THREAD_SAFE_INCR_32(__ptarget) 				((*__ptarget)++)
#define THREAD_SAFE_DECR_32(__ptarget) 				((*__ptarget)--)

#define CREATE_EVENT_OBJ(__obj)
#define DELETE_EVENT_OBJ(__obj)			
#define SET_EVENT(__obj)
#define RESET_EVENT(__obj)
#define WAIT_FOR_MULT_EVENTS(__noObj, __pobj, __msecs)
#define WAIT_FOR_ONE_EVENT(__obj, __msecs)

/* Definitions for MS-Windows Platforms (Win3.1) (WinNT) */
#if defined( _WIN16 ) || defined( _WIN32 )

#define GetDTableSize()	FD_SETSIZE
//#define MAP_SOCK_2_SESSION
#define SOCK_IO_W_4_ARGS
//#define NO_SO_DONTLINGER

#if defined(_WIN32)
	
	#include <windows.h>

	typedef char FAR * caddr_t;

	#undef  THREAD_SAFE_ASSIGN_32
	#define THREAD_SAFE_ASSIGN_32(__ptarget, __value)	InterlockedExchange(__ptarget, __value)
			
	#undef  THREAD_SAFE_INCR_32							
	#define THREAD_SAFE_INCR_32(__ptarget)				InterlockedIncrement(__ptarget)
	
	#undef  THREAD_SAFE_DECR_32
	#define THREAD_SAFE_DECR_32(__ptarget)				InterlockedDecrement(__ptarget)

/* client state sync */
	extern 	CRITICAL_SECTION 				ripcCStateCritSec;

	#undef  ENTER_C_SESSION_STATE_SYNC
	#define ENTER_C_SESSION_STATE_SYNC() 	EnterCriticalSection(&ripcCStateCritSec)
	
	#undef  EXIT_C_SESSION_STATE_SYNC
	#define EXIT_C_SESSION_STATE_SYNC()  	LeaveCriticalSection(&ripcCStateCritSec)

/* client state sync */
	extern 	CRITICAL_SECTION 				ripcSStateCritSec;

	#undef  ENTER_S_SESSION_STATE_SYNC
	#define ENTER_S_SESSION_STATE_SYNC() 	EnterCriticalSection(&ripcSStateCritSec)
	
	#undef  EXIT_S_SESSION_STATE_SYNC
	#define EXIT_S_SESSION_STATE_SYNC()  	LeaveCriticalSection(&ripcSStateCritSec)

/* channel read / write completion events */
	#undef  CREATE_EVENT_OBJ
	#define CREATE_EVENT_OBJ(__obj)			(__obj = CreateEvent(NULL, FALSE, FALSE, NULL))
	
	#undef  DELETE_EVENT_OBJ
	#define DELETE_EVENT_OBJ(__obj)			CloseHandle(__obj)
	
	#undef  SET_EVENT
	#define SET_EVENT(__obj)				SetEvent(__obj)

	#undef  RESET_EVENT
	#define RESET_EVENT(__obj)				ResetEvent(__obj)

	#undef  WAIT_FOR_MULT_EVENTS
	#define WAIT_FOR_MULT_EVENTS(__noObj, __pobj, __msecs)	WaitForMultipleObjects(__noObj, __pobj, TRUE, __msecs)
	
	#undef  WAIT_FOR_ONE_EVENT
	#define WAIT_FOR_ONE_EVENT(__obj, __msecs)				WaitForSingleObject(__obj, __msecs)

	#define CLOSE_WAIT_MSECS				3000
	#define READ_WAIT_MSECS					0
	#define WRITE_WAIT_MSECS				0

#endif

#endif


#endif
